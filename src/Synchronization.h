#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>

// Event group bits for system events
#define STATE_CHANGED_BIT    BIT0
#define DISPLAY_READY_BIT    BIT1
#define CONTROLLER_READY_BIT BIT2
#define SYSTEM_SHUTDOWN_BIT  BIT3

// Queue message structure for inter-task communication
struct SystemMessage {
  enum MessageType {
    MSG_STATE_CHANGE,
    MSG_BUTTON_EVENT,
    MSG_DISPLAY_UPDATE,
    MSG_SYSTEM_EVENT
  } type;
  
  union {
    int stateValue;
    int buttonValue;
    struct {
      int param1;
      int param2;
    } params;
  } data;
};

class Synchronization {
private:
  // Singleton instance
  static Synchronization* m_instance;
  
  // FreeRTOS primitives
  SemaphoreHandle_t m_displayMutex;
  SemaphoreHandle_t m_stateMutex;
  SemaphoreHandle_t m_serialMutex;
  QueueHandle_t m_messageQueue;
  EventGroupHandle_t m_eventGroup;
  
  // Configuration
  static const int MESSAGE_QUEUE_SIZE = 10;
  static const TickType_t DEFAULT_TIMEOUT = pdMS_TO_TICKS(1000);
  
  // Private constructor
  Synchronization();

public:
  // Singleton access
  static Synchronization* getInstance();
  
  // Initialization
  bool initialize();
  void cleanup();
  
  // Mutex operations
  bool acquireDisplayMutex(TickType_t timeout = DEFAULT_TIMEOUT);
  void releaseDisplayMutex();
  bool acquireStateMutex(TickType_t timeout = DEFAULT_TIMEOUT);
  void releaseStateMutex();
  bool acquireSerialMutex(TickType_t timeout = DEFAULT_TIMEOUT);
  void releaseSerialMutex();
  
  // Message queue operations
  bool sendMessage(const SystemMessage& message, TickType_t timeout = DEFAULT_TIMEOUT);
  bool receiveMessage(SystemMessage& message, TickType_t timeout = DEFAULT_TIMEOUT);
  bool peekMessage(SystemMessage& message);
  int getMessageCount();
  
  // Event group operations
  void setEventBits(EventBits_t bits);
  void clearEventBits(EventBits_t bits);
  EventBits_t waitForAnyBit(EventBits_t bits, TickType_t timeout = DEFAULT_TIMEOUT);
  EventBits_t waitForAllBits(EventBits_t bits, TickType_t timeout = DEFAULT_TIMEOUT);
  EventBits_t getCurrentBits();
  
  // Utility functions
  void safePrint(const char* message);
  void safePrintln(const char* message);
  void safePrintf(const char* format, ...);
  
  // System synchronization helpers
  void notifyStateChange();
  void notifyDisplayReady();
  void notifyControllerReady();
  void signalShutdown();
  bool waitForSystemReady(TickType_t timeout = DEFAULT_TIMEOUT);
};

// Convenience macros for common operations
#define SYNC_DISPLAY_LOCK() Synchronization::getInstance()->acquireDisplayMutex()
#define SYNC_DISPLAY_UNLOCK() Synchronization::getInstance()->releaseDisplayMutex()
#define SYNC_STATE_LOCK() Synchronization::getInstance()->acquireStateMutex()
#define SYNC_STATE_UNLOCK() Synchronization::getInstance()->releaseStateMutex()
#define SYNC_PRINT(msg) Synchronization::getInstance()->safePrintln(msg)

#endif // SYNCHRONIZATION_H