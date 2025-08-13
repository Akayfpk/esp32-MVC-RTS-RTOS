#include "Controller.h"
#include "RTClib.h"  // Make sure this is included in the Controller's cpp

extern RTC_DS1307 rtc;
/**
 * @brief Constructor - Initializes controller with model reference
 */
Controller::Controller() : m_model(nullptr), m_taskHandle(nullptr) {
  m_model = Model::getInstance();
}

/**
 * @brief Destructor - Ensures proper task cleanup
 */
Controller::~Controller() {
  stop(); // Safely stop FreeRTOS task
}

/**
 * @brief Initializes the controller
 * @return true if initialization succeeded, false otherwise
 */
bool Controller::initialize() {
  if (m_model == nullptr) { 
    // Validate model instance
    Serial.println("Model not available");
    return false;
  }
  
  initializeButtons();
  // Hardware button setup
  Serial.println("Controller initialized");
  return true;
}

/**
 * @brief Configures button pins and initial state
 * Uses INPUT_PULLUP mode with active-low logic
 */
void Controller::initializeButtons() {
 // Button configuration with pin numbers and initial states
   m_buttons[0] = {BTN_UP_PIN, HIGH, 0, false};      // Up button
  m_buttons[1] = {BTN_DOWN_PIN, HIGH, 0, false};    // Down button
  m_buttons[2] = {BTN_LEFT_PIN, HIGH, 0, false};    // Left button
  m_buttons[3] = {BTN_RIGHT_PIN, HIGH, 0, false};   // Right button
  m_buttons[4] = {BTN_SELECT1_PIN, HIGH, 0, false}; // Primary select
  m_buttons[5] = {BTN_SELECT2_PIN, HIGH, 0, false}; // Secondary select
  
  // Configure all button pins as INPUT_PULLUP
  for (int i = 0; i < 6; i++) {
    pinMode(m_buttons[i].pin, INPUT_PULLUP);
  }
}

/**
 * @brief Starts the controller task
 * @return true if task started successfully, false otherwise
 */
bool Controller::start() {
  if (m_taskHandle != nullptr) {
    Serial.println("Controller task already running");
    return false;
  }
  
  // Create FreeRTOS task for button handling
  BaseType_t result = xTaskCreate(
    taskWrapper,
    "ButtonTask",
    2048, // Stack size
    this, // Task parameter
    2,    // Priority (higher than display tasks)
    &m_taskHandle
  );
  
  return result == pdPASS;
  // pdPASS indicates success
}

/**
 * @brief Stops the controller task
 */
void Controller::stop() {
  if (m_taskHandle != nullptr) {
    vTaskDelete(m_taskHandle);
    m_taskHandle = nullptr;
  }
}

/**
 * @brief FreeRTOS task wrapper function
 * @param pvParameters Pointer to the Controller instance
 */
void Controller::taskWrapper(void* pvParameters) {
  Controller* controller = static_cast<Controller*>(pvParameters);
  controller->buttonTask();
}

/**
 * @brief Main button task function
 * Continuously polls buttons and handles events
 */
void Controller::buttonTask() {
  Serial.println("Button task started");
  
  while (true) {
    SystemEvent event = readButtons();
    
    if (event != EVENT_NONE) {
      handleEvent(event);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // 10ms polling rate
  }
}

/**
 * @brief Reads and debounces all buttons
 * @return SystemEvent representing the button press (or EVENT_NONE)
 */
SystemEvent Controller::readButtons() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < 6; i++) {
    bool currentState = digitalRead(m_buttons[i].pin);
    
    // Check for state change
    if (currentState != m_buttons[i].lastState) {
      m_buttons[i].lastDebounceTime = currentTime;
    }
    
    // Check if debounce period has passed
    if ((currentTime - m_buttons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      // Detect button press (LOW for pull-up)
      if (!m_buttons[i].pressed && currentState == LOW) {
        m_buttons[i].pressed = true;
        m_buttons[i].lastState = currentState;
        
        // Return corresponding event
        switch (i) {
          case 0: return EVENT_UP;
          case 1: return EVENT_DOWN;
          case 2: return EVENT_LEFT;
          case 3: return EVENT_RIGHT;
          case 4: return EVENT_SELECT1;
          case 5: return EVENT_SELECT2;
        }
      }
      // Detect button release
      else if (m_buttons[i].pressed && currentState == HIGH) {
        m_buttons[i].pressed = false;
      }
    }
    
    m_buttons[i].lastState = currentState;
  }
  
  return EVENT_NONE;
}

/**
 * @brief Handles system events based on current state
 * @param event The system event to handle
 */
void Controller::handleEvent(SystemEvent event) {
  SystemState currentState = m_model->getCurrentState();
  
  // Route to appropriate state handler
  switch (currentState) {
    case STATE_MENU:
      handleMenuState(event);
      break;
    case STATE_SETTINGS:
      handleSettingsState(event);
      break;
    case STATE_ABOUT:
      handleAboutState(event);
      break;
    case STATE_CONFIRM_EXIT:
      handleConfirmExitState(event);
      break;
  }
}

// State-specific event handlers
void Controller::handleMenuState(SystemEvent event) {
  switch (event) {
    case EVENT_UP:
      m_model->decrementMenuIndex();
      break;
    case EVENT_DOWN:
      m_model->incrementMenuIndex();
      break;
    case EVENT_SELECT1:
      {
        int menuIndex = m_model->getMenuIndex();
        switch (menuIndex) {
          case 0: { // Home
            DateTime now = rtc.now();
            char timeStr[25];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d %02d/%02d/%04d",
              now.hour(), now.minute(), now.second(),
              now.day(), now.month(), now.year());

            Serial.print("Home selected - Current Time: ");
            Serial.println(timeStr);

            m_model->setCurrentTime(now);
            break;
          }
          case 1: // Settings
            m_model->setState(STATE_SETTINGS);
            break;
          case 2: // About
            m_model->setState(STATE_ABOUT);
            break;
          case 3: // Exit
            m_model->setState(STATE_CONFIRM_EXIT);
            break;
        }
      }
      break;
    case EVENT_SELECT2:
      Serial.println("Secondary select in menu");
      break;
    default:
      break;
  }
}

void Controller::handleSettingsState(SystemEvent event) {
  switch (event) {
    case EVENT_LEFT:
    case EVENT_SELECT2:
      m_model->setState(STATE_MENU);
      Serial.println("Returning to menu from settings");
      break;
    case EVENT_SELECT1:
      Serial.println("Settings action");
      break;
    default:
      break;
  }
}

void Controller::handleAboutState(SystemEvent event) {
  switch (event) {
    case EVENT_LEFT:
    case EVENT_SELECT2:
      m_model->setState(STATE_MENU);
      Serial.println("Returning to menu from about");
      break;
    default:
      break;
  }
}

void Controller::handleConfirmExitState(SystemEvent event) {
  switch (event) {
    case EVENT_SELECT1: // Confirm exit
      Serial.println("Exit confirmed - implement shutdown logic");
      m_model->setState(STATE_MENU); // Temporary - would normally shutdown
      break;
    case EVENT_LEFT:
    case EVENT_SELECT2: // Cancel exit
      m_model->setState(STATE_MENU);
      Serial.println("Exit cancelled");
      break;
    default:
      break;
  }
}