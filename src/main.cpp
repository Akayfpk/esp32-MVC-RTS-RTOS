#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
// Include all our MVC components
#include "Model.h"
#include "Controller.h"
#include "OLEDView.h"
#include "LCDView.h"
#include "Synchronization.h"

// Global system components
Model* g_model = nullptr;
Controller* g_controller = nullptr;
OLEDView* g_oledView = nullptr;
LCDView* g_lcdView = nullptr;
Synchronization* g_sync = nullptr;

 // Real-Time Clock instance
RTC_DS1307 rtc;

// System status
bool g_systemInitialized = false;

// Function prototypes
bool initializeHardware();
bool initializeComponents();
bool startTasks();
void cleanup();
void systemStatusTask(void* pvParameters);

void setup() {
  Serial.begin(115200);
  Serial.println("=== ESP32 Menu System Starting ===");
  
  // Initialize hardware
  if (!initializeHardware()) {
    Serial.println("Hardware initialization failed!");
    return;
  }
  
  // MVC component initialization
  if (!initializeComponents()) {
    Serial.println("Component initialization failed!");
    cleanup();
    return;
  }
  
  // FreeRTOS task creation
  if (!startTasks()) {
    Serial.println("Task startup failed!");
    cleanup();
    return;
  }
  
  // Wait for system to be ready
  if (g_sync->waitForSystemReady(pdMS_TO_TICKS(5000))) {
    g_systemInitialized = true;
    Serial.println("=== System Ready ===");
  } else {
    Serial.println("System startup timeout!");
    cleanup();
  }

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
}

}

void loop() {
  // Main loop is empty - FreeRTOS handles everything
  // Optional: Add watchdog or health monitoring here
  if (g_systemInitialized) {
    // System running normally
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Optional: Print system status periodically
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 30000) { // Every 30 seconds
      g_sync->safePrintf("System uptime: %lu ms, Free heap: %d bytes\n", 
                         millis(), ESP.getFreeHeap());
      lastStatus = millis();
    }
  } else {
    // System failed to initialize
    delay(1000);
  }
}

bool initializeHardware() {
  Serial.println("Initializing hardware...");
  
  // Initialize I2C
  Wire.begin();
  Serial.println("I2C initialized");
  
  // Add any other hardware initialization here
  // (SPI, additional peripherals, etc.)
  
  Serial.println("Hardware initialization complete");
  return true;
}

bool initializeComponents() {
  Serial.println("Initializing software components...");
  
  // Initialize synchronization first
  g_sync = Synchronization::getInstance();
  if (!g_sync->initialize()) {
    Serial.println("Synchronization initialization failed");
    return false;
  }
  
  // Initialize model
  g_model = Model::getInstance();
  
  if (!g_model->initialize()) {
    Serial.println("Model initialization failed");
    return false;
  }
  
  // Initialize controller
  g_controller = new Controller();
  if (!g_controller->initialize()) {
    Serial.println("Controller initialization failed");
    return false;
  }
  
  // Initialize views
  g_oledView = new OLEDView();
  if (!g_oledView->initialize()) {
    Serial.println("OLED View initialization failed");
    return false;
  }
  
  g_lcdView = new LCDView();
  if (!g_lcdView->initialize()) {
    Serial.println("LCD View initialization failed");
    return false;
  }
  
  Serial.println("All components initialized successfully");
  return true;
}

bool startTasks() {
  Serial.println("Starting tasks...");
  
  // Start controller task
  if (!g_controller->start()) {
    Serial.println("Failed to start controller task");
    return false;
  }
  g_sync->notifyControllerReady();
  
  // Start view tasks
  if (!g_oledView->start()) {
    Serial.println("Failed to start OLED task");
    return false;
  }
  
  if (!g_lcdView->start()) {
    Serial.println("Failed to start LCD task");
    return false;
  }
  g_sync->notifyDisplayReady();
  
  // Create system status monitoring task
  xTaskCreate(
    systemStatusTask,
    "SystemStatus",
    1024,
    nullptr,
    1,
    nullptr
  );

  // Create RTC update task
  xTaskCreate(
    [](void*) {
      Model* model = Model::getInstance();
      TickType_t lastWake = xTaskGetTickCount();
      for (;;) {
        model->updateTime();
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000)); // update every second
      }
    },
    "RTC_Update",
    2048, // stack size
    nullptr,
    1,    // priority
    nullptr
  );

  Serial.println("All tasks started successfully");
  return true;
}


void cleanup() {
  Serial.println("Cleaning up system...");
  
  // Stop and cleanup components
  if (g_controller != nullptr) {
    g_controller->stop();
    delete g_controller;
    g_controller = nullptr;
  }
  
  if (g_oledView != nullptr) {
    g_oledView->stop();
    delete g_oledView;
    g_oledView = nullptr;
  }
  
  if (g_lcdView != nullptr) {
    g_lcdView->stop();
    delete g_lcdView;
    g_lcdView = nullptr;
  }
  
  if (g_model != nullptr) {
    g_model->cleanup();
    // Don't delete model as it's a singleton
  }
  
  if (g_sync != nullptr) {
    g_sync->cleanup();
    // Don't delete sync as it's a singleton
  }
  
  Serial.println("Cleanup complete");
}

void systemStatusTask(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t frequency = pdMS_TO_TICKS(10000); // 10 seconds
  
  while (true) {
    // Monitor system health
    if (g_systemInitialized) {
      // Check for any error conditions
      UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(nullptr);
      if (highWaterMark < 100) { // Less than 100 bytes free
        g_sync->safePrintln("WARNING: Low stack space in system status task");
      }
      
      // Monitor message queue
      int messageCount = g_sync->getMessageCount();
      if (messageCount > 8) { // Queue getting full
        g_sync->safePrintf("WARNING: Message queue filling up (%d messages)\n", messageCount);
      }
      
      // Check for shutdown signal
      EventBits_t bits = g_sync->getCurrentBits();
      if (bits & SYSTEM_SHUTDOWN_BIT) {
        g_sync->safePrintln("Shutdown signal received, cleaning up...");
        cleanup();
        g_systemInitialized = false;
      }
    }
    
    vTaskDelayUntil(&lastWakeTime, frequency);
  }
}