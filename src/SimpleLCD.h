#ifndef SIMPLE_LCD_H
#define SIMPLE_LCD_H

#include <LiquidCrystal_I2C.h>

class SimpleLCD {
public:
  // Constructor with default I2C address and size (can be extended later)
  SimpleLCD() : lcd(0x27, 16, 2) {}

  // Initialize LCD
  void begin() {
    lcd.begin(16, 2);
    lcd.backlight();
  }

  // Clear screen
  void clear() {
    lcd.clear();
  }

  // Print at default (0,0)
  void print(const String &text) {
    lcd.setCursor(0, 0);
    lcd.print(text);
  }

  // Print at a given position
  void print(const String &text, uint8_t col, uint8_t row) {
    lcd.setCursor(col, row);
    lcd.print(text);
  }

  // Print padded text at position to fully overwrite a line
  void printPadded(const String &text, uint8_t col, uint8_t row) {
    lcd.setCursor(col, row);
    String padded = text + "                ";  // Add extra spaces
    lcd.print(padded.substring(0, 16));         // Trim to 16 characters
  }

  // Set cursor position
  void setCursor(uint8_t col, uint8_t row) {
    lcd.setCursor(col, row);
  }

  // Optional: control backlight
  void backlight() {
    lcd.backlight();
  }

  void noBacklight() {
    lcd.noBacklight();
  }

private:
  LiquidCrystal_I2C lcd;
};

#endif
