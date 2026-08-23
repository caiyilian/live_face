#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#define OLED_SDA 4
#define OLED_SCL 5
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(128, 64, &Wire, -1);

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
  display.setCursor(28, 24);
  display.println("Connecting...");
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    tries++;
  }

  drawHappyFace();
  display.display();
}

void loop() {
  delay(10000);
}

void drawHappyFace() {
  display.clearDisplay();
  int cx = 64, cy = 32, r = 30;

  // 脸
  display.drawCircle(cx, cy, r, SSD1306_WHITE);

  // 左眼
  display.fillCircle(cx - 11, cy - 8, 5, SSD1306_WHITE);
  // 右眼
  display.fillCircle(cx + 11, cy - 8, 5, SSD1306_WHITE);

  // 嘴巴（微笑：画圆，擦掉下半部分，保留上半弧线）
  int mouthY = cy + 6;
  int mouthR = 14;
  display.drawCircle(cx, mouthY, mouthR, SSD1306_WHITE);
  display.fillRect(cx - mouthR, mouthY, mouthR * 2, mouthR + 2, SSD1306_BLACK);
}