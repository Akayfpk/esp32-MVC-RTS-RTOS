#include "Synchronization.h"
#include <stdarg.h>

// Initialize static instance pointer to nullptr
Synchronization* Synchronization::m_instance = nullptr;

/**
 * @brief Constructor - Initializes all synchronization primitives to nullptr
 */
Synchronization::Synchronization()
  : m_displayMutex(nullptr), m_stateMutex(nullptr), m_serialMutex(nullptr),
    m_messageQueue(nullptr), m_eventGroup(nullptr) {
}

/**
 * @brief Singleton instance getter
 * @return Pointer to the single instance of Synchronization class
 */
Synchronization* Synchronization::getInstance() {
  if (m_instance == nullptr) {
    m_instance = new Synchronization();
  }
  return m_instance;
}

/**
 * @brief Initializes all synchronization primitives
 * @return true if initialization succeeded, false otherwise
 */
bool Synchronization::initialize() {
  // Create mutexes for different subsystems
  m_displayMutex = xSemaphoreCreateMutex();
  m_stateMutex = xSemaphoreCreateMutex();
  m_serialMutex = xSemaphoreCreateMutex();
  
  // Create message queue for inter-task communication
  m_messageQueue = xQueueCreate(MESSAGE_QUEUE_SIZE, sizeof(SystemMessage));
  
  // Create event group for system-wide event notification
  m_eventGroup = xEventGroupCreate();
  
  // Verify all resources were created successfully
  if (m_displayMutex == nullptr || m_stateMutex == nullptr || 
      m_serialMutex == nullptr || m_messageQueue == nullptr || 
      m_eventGroup == nullptr) {
    Serial.println("Failed to create synchronization primitives");
    cleanup();
    return false;
  }
  
  Serial.println("Synchronization system initialized");
  return true;
}

/**
 * @brief Cleans up all synchronization resources
 */
void Synchronization::cleanup() {
  // Delete display mutex if it exists
  if (m_displayMutex != nullptr) {
    vSemaphoreDelete(m_displayMutex);
    m_displayMutex = nullptr;
  }
  
  // Delete state mutex if it exists
  if (m_stateMutex != nullptr) {
    vSemaphoreDelete(m_stateMutex);
    m_stateMutex = nullptr;
  }
  
  // Delete serial mutex if it exists
  if (m_serialMutex != nullptr) {
    vSemaphoreDelete(m_serialMutex);
    m_serialMutex = nullptr;
  }
  
  // Delete message queue if it exists
  if (m_messageQueue != nullptr) {
    vQueueDelete(m_messageQueue);
    m_messageQueue = nullptr;
  }
  
  // Delete event group if it exists
  if (m_eventGroup != nullptr) {
    vEventGroupDelete(m_eventGroup);
    m_eventGroup = nullptr;
  }
}

// Mutex operations for display access
bool Synchronization::acquireDisplayMutex(TickType_t timeout) {
  return m_displayMutex != nullptr && xSemaphoreTake(m_displayMutex, timeout) == pdTRUE;
}

void Synchronization::releaseDisplayMutex() {
  if (m_displayMutex != nullptr) {
    xSemaphoreGive(m_displayMutex);
  }
}

// Mutex operations for state access
bool Synchronization::acquireStateMutex(TickType_t timeout) {
  return m_stateMutex != nullptr && xSemaphoreTake(m_stateMutex, timeout) == pdTRUE;
}

void Synchronization::releaseStateMutex() {
  if (m_stateMutex != nullptr) {
    xSemaphoreGive(m_stateMutex);
  }
}

// Mutex operations for serial port access
bool Synchronization::acquireSerialMutex(TickType_t timeout) {
  return m_serialMutex != nullptr && xSemaphoreTake(m_serialMutex, timeout) == pdTRUE;
}

void Synchronization::releaseSerialMutex() {
  if (m_serialMutex != nullptr) {
    xSemaphoreGive(m_serialMutex);
  }
}

// Message queue operations
bool Synchronization::sendMessage(const SystemMessage& message, TickType_t timeout) {
  if (m_messageQueue == nullptr) return false;
  return xQueueSend(m_messageQueue, &message, timeout) == pdTRUE;
}

bool Synchronization::receiveMessage(SystemMessage& message, TickType_t timeout) {
  if (m_messageQueue == nullptr) return false;
  return xQueueReceive(m_messageQueue, &message, timeout) == pdTRUE;
}

bool Synchronization::peekMessage(SystemMessage& message) {
  if (m_messageQueue == nullptr) return false;
  return xQueuePeek(m_messageQueue, &message, 0) == pdTRUE;
}

int Synchronization::getMessageCount() {
  if (m_messageQueue == nullptr) return 0;
  return uxQueueMessagesWaiting(m_messageQueue);
}

// Event group operations
void Synchronization::setEventBits(EventBits_t bits) {
  if (m_eventGroup != nullptr) {
    xEventGroupSetBits(m_eventGroup, bits);
  }
}

void Synchronization::clearEventBits(EventBits_t bits) {
  if (m_eventGroup != nullptr) {
    xEventGroupClearBits(m_eventGroup, bits);
  }
}

EventBits_t Synchronization::waitForAnyBit(EventBits_t bits, TickType_t timeout) {
  if (m_eventGroup == nullptr) return 0;
  return xEventGroupWaitBits(m_eventGroup, bits, pdFALSE, pdFALSE, timeout);
}

EventBits_t Synchronization::waitForAllBits(EventBits_t bits, TickType_t timeout) {
  if (m_eventGroup == nullptr) return 0;
  return xEventGroupWaitBits(m_eventGroup, bits, pdFALSE, pdTRUE, timeout);
}

EventBits_t Synchronization::getCurrentBits() {
  if (m_eventGroup == nullptr) return 0;
  return xEventGroupGetBits(m_eventGroup);
}

// Thread-safe printing functions
void Synchronization::safePrint(const char* message) {
  if (acquireSerialMutex(pdMS_TO_TICKS(100))) {
    Serial.print(message);
    releaseSerialMutex();
  }
}

void Synchronization::safePrintln(const char* message) {
  if (acquireSerialMutex(pdMS_TO_TICKS(100))) {
    Serial.println(message);
    releaseSerialMutex();
  }
}

void Synchronization::safePrintf(const char* format, ...) {
  if (acquireSerialMutex(pdMS_TO_TICKS(100))) {
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.print(buffer);
    
    va_end(args);
    releaseSerialMutex();
  }
}

// System notification functions
void Synchronization::notifyStateChange() {
  setEventBits(STATE_CHANGED_BIT);
  
  SystemMessage msg;
  msg.type = SystemMessage::MSG_STATE_CHANGE;
  msg.data.stateValue = 0; // Could include specific state info
  sendMessage(msg, 0); // Don't block
}

void Synchronization::notifyDisplayReady() {
  setEventBits(DISPLAY_READY_BIT);
  safePrintln("Display system ready");
}

void Synchronization::notifyControllerReady() {
  setEventBits(CONTROLLER_READY_BIT);
  safePrintln("Controller system ready");
}

void Synchronization::signalShutdown() {
  setEventBits(SYSTEM_SHUTDOWN_BIT);
  safePrintln("System shutdown signaled");
}

/**
 * @brief Waits for all system components to be ready
 * @param timeout Maximum time to wait in ticks
 * @return true if all components are ready, false if timeout occurred
 */
bool Synchronization::waitForSystemReady(TickType_t timeout) {
  EventBits_t requiredBits = DISPLAY_READY_BIT | CONTROLLER_READY_BIT;
  EventBits_t result = waitForAllBits(requiredBits, timeout);
  return (result & requiredBits) == requiredBits;
}