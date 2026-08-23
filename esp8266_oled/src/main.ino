#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#define OLED_SDA 4
#define OLED_SCL 5
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const char* EMOTIONS[] = {"HAPPY", "SAD", "ANGRY", "SURPRISE", "FEAR", "DISGUST", "NEUTRAL"};
int current = 0;
unsigned long lastSwitch = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (1) delay(100);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 24);
  display.println("Connecting...");
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    tries++;
  }

  lastSwitch = millis();
}

void loop() {
  if (millis() - lastSwitch >= 1000) {
    lastSwitch = millis();
    current = (current + 1) % 7;
    drawEmotion(current);
    display.display();
  }
}

void drawEmotion(int idx) {
  display.clearDisplay();
  int cx = 46, cy = 32, r = 22;

  display.drawCircle(cx, cy, r, SSD1306_WHITE);

  switch (idx) {
    case 0: drawHappy(cx, cy); break;
    case 1: drawSad(cx, cy); break;
    case 2: drawAngry(cx, cy); break;
    case 3: drawSurprise(cx, cy); break;
    case 4: drawFear(cx, cy); break;
    case 5: drawDisgust(cx, cy); break;
    case 6: drawNeutral(cx, cy); break;
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(78, 28);
  display.println(EMOTIONS[idx]);
}

void drawHappy(int cx, int cy) {
  display.fillCircle(cx - 7, cy - 5, 4, SSD1306_WHITE);
  display.fillCircle(cx + 7, cy - 5, 4, SSD1306_WHITE);
  int my = cy + 6;
  display.drawCircle(cx, my, 10, SSD1306_WHITE);
  display.fillRect(cx - 10, my, 20, 10, SSD1306_BLACK);
}

void drawSad(int cx, int cy) {
  display.fillCircle(cx - 7, cy - 5, 4, SSD1306_WHITE);
  display.fillCircle(cx + 7, cy - 5, 4, SSD1306_WHITE);
  display.drawLine(cx - 12, cy - 12, cx - 6, cy - 8, SSD1306_WHITE);
  display.drawLine(cx + 12, cy - 12, cx + 6, cy - 8, SSD1306_WHITE);
  int my = cy + 8;
  display.drawCircle(cx, my, 10, SSD1306_WHITE);
  display.fillRect(cx - 10, my - 10, 20, 12, SSD1306_BLACK);
  display.fillCircle(cx - 14, cy + 1, 2, SSD1306_WHITE);
}

void drawAngry(int cx, int cy) {
  display.fillCircle(cx - 7, cy - 5, 4, SSD1306_WHITE);
  display.fillCircle(cx + 7, cy - 5, 4, SSD1306_WHITE);
  display.drawLine(cx - 14, cy - 16, cx - 4, cy - 10, SSD1306_WHITE);
  display.drawLine(cx + 14, cy - 16, cx + 4, cy - 10, SSD1306_WHITE);
  display.drawLine(cx - 6, cy + 6, cx + 6, cy + 6, SSD1306_WHITE);
}

void drawSurprise(int cx, int cy) {
  display.drawCircle(cx - 7, cy - 5, 5, SSD1306_WHITE);
  display.drawCircle(cx + 7, cy - 5, 5, SSD1306_WHITE);
  display.fillCircle(cx, cy + 6, 7, SSD1306_WHITE);
  display.fillCircle(cx, cy + 6, 4, SSD1306_BLACK);
  display.drawLine(cx - 8, cy - 16, cx - 6, cy - 12, SSD1306_WHITE);
  display.drawLine(cx + 8, cy - 16, cx + 6, cy - 12, SSD1306_WHITE);
}

void drawFear(int cx, int cy) {
  display.drawCircle(cx - 7, cy - 5, 5, SSD1306_WHITE);
  display.drawCircle(cx + 7, cy - 5, 5, SSD1306_WHITE);
  display.fillCircle(cx - 7, cy - 5, 2, SSD1306_WHITE);
  display.fillCircle(cx + 7, cy - 5, 2, SSD1306_WHITE);
  display.drawCircle(cx, cy + 6, 6, SSD1306_WHITE);
  display.fillRect(cx - 6, cy + 6, 12, 6, SSD1306_BLACK);
  display.drawLine(cx - 10, cy - 16, cx - 7, cy - 12, SSD1306_WHITE);
  display.drawLine(cx + 10, cy - 16, cx + 7, cy - 12, SSD1306_WHITE);
}

void drawDisgust(int cx, int cy) {
  display.fillCircle(cx - 7, cy - 5, 4, SSD1306_WHITE);
  display.drawCircle(cx + 7, cy - 5, 4, SSD1306_WHITE);
  display.drawLine(cx - 13, cy - 14, cx - 7, cy - 10, SSD1306_WHITE);
  display.drawLine(cx + 13, cy - 14, cx + 7, cy - 10, SSD1306_WHITE);
  display.drawLine(cx - 6, cy + 3, cx, cy + 8, SSD1306_WHITE);
  display.drawLine(cx, cy + 8, cx + 6, cy + 3, SSD1306_WHITE);
}

void drawNeutral(int cx, int cy) {
  display.fillCircle(cx - 7, cy - 5, 4, SSD1306_WHITE);
  display.fillCircle(cx + 7, cy - 5, 4, SSD1306_WHITE);
  display.drawLine(cx - 6, cy + 6, cx + 6, cy + 6, SSD1306_WHITE);
}