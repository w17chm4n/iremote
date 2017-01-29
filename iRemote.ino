#include <Arduino.h>
#include <SPI.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "BluefruitConfig.h"
#include "keycode.h"

#define FACTORYRESET_ENABLE 0

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

const int BUTTON_PIN = 9;
const int LED_PIN = 13;

int previousButtonState = LOW;

typedef struct {
  uint8_t modifier;
  uint8_t reserved;
  uint8_t keycode;
} hid_keyboard_report_t;

hid_keyboard_report_t keyReport = {0, 0, 0};

void setup() {
  //while (!Serial);
  delay(500);
  
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT); // button pin
  pinMode(LED_PIN, OUTPUT); //led pin

  Serial.println("iRemote!");
  Serial.println("------");

  if (!ble.begin(VERBOSE_MODE)) {
    Serial.println("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring!");
  } else {
    Serial.println("BLE OK!");
  }

  ble.echo(false);

  if(!ble.sendCommandCheckOK("AT+GAPDEVNAME=iRemote")) {
    Serial.println("Could not set device name!");
  }

  if(!ble.sendCommandCheckOK("AT+BleHIDEn")) {
    Serial.println("Could not enable HID!");
  }

  if(!ble.reset()) {
    Serial.println("Could not reset!");
  }
  
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  
  if (buttonState == LOW && previousButtonState == HIGH) {
    previousButtonState = LOW;
    keyReport.keycode = HID_KEY_SPACE;
    ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport, 3);
    delay(40);
    keyReport.keycode = HID_KEY_NONE;
    ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport, 3);
  } else if (buttonState == HIGH && previousButtonState == LOW) {
    previousButtonState = HIGH;
  }
  delay(60);
}
