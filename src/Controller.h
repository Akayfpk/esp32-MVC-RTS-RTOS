#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Model.h"

// Button configuration
struct ButtonConfig {
  int pin;
  bool lastState;
  unsigned long lastDebounceTime;
  bool pressed;
};

class Controller {
private:
  // Button pins
  static const int BTN_UP_PIN = 32;
  static const int BTN_DOWN_PIN = 33;
  static const int BTN_LEFT_PIN = 25;
  static const int BTN_RIGHT_PIN = 26;
  static const int BTN_SELECT1_PIN = 27;
  static const int BTN_SELECT2_PIN = 14;
  
  // Debounce timing
  static const unsigned long DEBOUNCE_DELAY = 50;
  static const unsigned long REPEAT_DELAY = 200;
  
  // Button states
  ButtonConfig m_buttons[6];
  
  // Model reference
  Model* m_model;
  
  // Task handle
  TaskHandle_t m_taskHandle;
  
  // Private methods
  void initializeButtons();
  SystemEvent readButtons();
  bool isButtonPressed(int buttonIndex);
  void handleEvent(SystemEvent event);
  void handleMenuState(SystemEvent event);
  void handleSettingsState(SystemEvent event);
  void handleAboutState(SystemEvent event);
  void handleConfirmExitState(SystemEvent event);
  
  // Static task wrapper
  static void taskWrapper(void* pvParameters);
  
  // Task implementation
  void buttonTask();

public:
  Controller();
  ~Controller();
  
  // Initialize controller
  bool initialize();
  
  // Start the controller task
  bool start();
  
  // Stop the controller task
  void stop();
};

#endif // CONTROLLER_H