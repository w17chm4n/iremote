#include <Arduino.h>
#include <SPI.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "BluefruitConfig.h"
#include "keycode.h"

#define MODIFIER 0xC //0x9
#define FACTORYRESET_ENABLE 0
#define VBAT_PIN A9

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

const int LEFT_PIN = 13;
const int MIDDLE_PIN = 12;
const int RIGHT_PIN = 11;

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

  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(MIDDLE_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);

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

  if(!ble.sendCommandCheckOK("AT+HWModeLED=0")) {
    Serial.println("Could not disable HW MODE LED!");
  }

  if(!ble.reset()) {
    Serial.println("Could not reset!");
  }

  ble.setMode(BLUEFRUIT_MODE_DATA);
}

void sendBatteryLevel() {
  float batteryLevel = analogRead(VBAT_PIN);
  batteryLevel *= 2;
  batteryLevel *= 3.3;
  batteryLevel /= 1024;

  char buffer[5]; buffer[5] = 0;

  dtostrf(batteryLevel,2,2,buffer);
  
  ble.println(buffer);
}

void sendKey(uint8_t modifier, uint8_t key) {
  keyReport.modifier = modifier;
  keyReport.keycode = key;
  ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport, 3);
  delay(40);
  keyReport.modifier = HID_KEY_NONE;
  keyReport.keycode = HID_KEY_NONE;
  ble.atcommand("AT+BLEKEYBOARDCODE", (uint8_t*) &keyReport, 3);
}

void loop() {
  int leftButtonState = digitalRead(LEFT_PIN);
  int middleButtonState = digitalRead(MIDDLE_PIN);
  int rightButtonState = digitalRead(RIGHT_PIN);

  if (leftButtonState == LOW && previousButtonState == HIGH) {
    previousButtonState = LOW;
    sendKey(MODIFIER, HID_KEY_ARROW_LEFT);
  } else if (middleButtonState == LOW && previousButtonState == HIGH) {
    previousButtonState = LOW;
    sendKey(HID_KEY_NONE, HID_KEY_SPACE);
    sendBatteryLevel();
  } else if (rightButtonState == LOW && previousButtonState == HIGH) {
    previousButtonState = LOW;
    sendKey(MODIFIER, HID_KEY_ARROW_RIGHT);
  } else if ((leftButtonState & middleButtonState & rightButtonState) == HIGH && previousButtonState == LOW) {
    previousButtonState = HIGH;
  }

  delay(60);
}
