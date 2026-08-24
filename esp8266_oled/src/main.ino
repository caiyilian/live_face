#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#define OLED_SDA 4
#define OLED_SCL 5
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(128, 64, &Wire, -1);
ESP8266WebServer server(80);
String currentEmotion = "HAPPY";

// ========== OLED 表情绘制 ==========

void drawFace(int cx, int cy, int r) {
  display.drawCircle(cx, cy, r, SSD1306_WHITE);
}

void drawEyes(int cx, int cy, int r) {
  display.fillCircle(cx - 10, cy - 8, r, SSD1306_WHITE);
  display.fillCircle(cx + 10, cy - 8, r, SSD1306_WHITE);
}

void drawEyebrows(int cx, int cy, int x1, int y1, int x2, int y2) {
  display.drawLine(cx + x1, cy + y1, cx + x2, cy + y2, SSD1306_WHITE);
}

void drawHappy(int cx, int cy) {
  drawFace(cx, cy, 28);
  drawEyes(cx, cy, 5);
  display.drawCircle(cx, cy + 6, 12, SSD1306_WHITE);
  display.fillRect(cx - 12, cy + 6 - 12, 24, 13, SSD1306_BLACK);
}

void drawSad(int cx, int cy) {
  drawFace(cx, cy, 28);
  drawEyes(cx, cy, 5);
  display.drawLine(cx - 14, cy - 14, cx - 7, cy - 9, SSD1306_WHITE);
  display.drawLine(cx + 14, cy - 14, cx + 7, cy - 9, SSD1306_WHITE);
  display.drawCircle(cx, cy + 8, 12, SSD1306_WHITE);
  display.fillRect(cx - 12, cy - 4, 24, 14, SSD1306_BLACK);
  display.fillCircle(cx - 16, cy, 2, SSD1306_WHITE);
}

void drawAngry(int cx, int cy) {
  drawFace(cx, cy, 28);
  drawEyes(cx, cy, 5);
  display.drawLine(cx - 16, cy - 18, cx - 5, cy - 11, SSD1306_WHITE);
  display.drawLine(cx + 16, cy - 18, cx + 5, cy - 11, SSD1306_WHITE);
  display.drawLine(cx - 8, cy + 6, cx + 8, cy + 6, SSD1306_WHITE);
}

void drawSurprise(int cx, int cy) {
  drawFace(cx, cy, 28);
  display.drawCircle(cx - 10, cy - 8, 6, SSD1306_WHITE);
  display.drawCircle(cx + 10, cy - 8, 6, SSD1306_WHITE);
  display.fillCircle(cx, cy + 6, 8, SSD1306_WHITE);
  display.fillCircle(cx, cy + 6, 4, SSD1306_BLACK);
  display.drawLine(cx - 10, cy - 18, cx - 7, cy - 14, SSD1306_WHITE);
  display.drawLine(cx + 10, cy - 18, cx + 7, cy - 14, SSD1306_WHITE);
}

void drawFear(int cx, int cy) {
  drawFace(cx, cy, 28);
  display.drawCircle(cx - 10, cy - 8, 6, SSD1306_WHITE);
  display.drawCircle(cx + 10, cy - 8, 6, SSD1306_WHITE);
  display.fillCircle(cx - 10, cy - 8, 2, SSD1306_WHITE);
  display.fillCircle(cx + 10, cy - 8, 2, SSD1306_WHITE);
  display.drawCircle(cx, cy + 6, 7, SSD1306_WHITE);
  display.fillRect(cx - 7, cy + 6, 14, 7, SSD1306_BLACK);
  display.drawLine(cx - 12, cy - 18, cx - 9, cy - 14, SSD1306_WHITE);
  display.drawLine(cx + 12, cy - 18, cx + 9, cy - 14, SSD1306_WHITE);
}

void drawDisgust(int cx, int cy) {
  drawFace(cx, cy, 28);
  display.fillCircle(cx - 10, cy - 8, 5, SSD1306_WHITE);
  display.drawCircle(cx + 10, cy - 8, 5, SSD1306_WHITE);
  display.drawLine(cx - 15, cy - 16, cx - 8, cy - 11, SSD1306_WHITE);
  display.drawLine(cx + 15, cy - 16, cx + 8, cy - 11, SSD1306_WHITE);
  display.drawLine(cx - 7, cy + 3, cx, cy + 9, SSD1306_WHITE);
  display.drawLine(cx, cy + 9, cx + 7, cy + 3, SSD1306_WHITE);
}

void drawNeutral(int cx, int cy) {
  drawFace(cx, cy, 28);
  drawEyes(cx, cy, 5);
  display.drawLine(cx - 8, cy + 6, cx + 8, cy + 6, SSD1306_WHITE);
}

void showEmotion(const String& em) {
  display.clearDisplay();
  int cx = 64, cy = 32;
  if (em == "HAPPY") drawHappy(cx, cy);
  else if (em == "SAD") drawSad(cx, cy);
  else if (em == "ANGRY") drawAngry(cx, cy);
  else if (em == "SURPRISE") drawSurprise(cx, cy);
  else if (em == "FEAR") drawFear(cx, cy);
  else if (em == "DISGUST") drawDisgust(cx, cy);
  else if (em == "NEUTRAL") drawNeutral(cx, cy);
  else drawHappy(cx, cy);
  display.display();
}

// ========== 解析 PC 后端 via mDNS ==========

IPAddress pcIp(0, 0, 0, 0);
bool pcResolved = false;

bool resolvePc() {
  // 用 DNS-SD 服务查询找 PC 的 _http._tcp 服务（实例名为 liveface）
  uint32_t count = MDNS.queryService("http", "tcp");
  for (uint32_t u = 0; u < count; u++) {
    if (String(MDNS.answerHostname(u)) == "liveface") {
      pcIp = MDNS.answerIP(u);
      pcResolved = true;
      Serial.print("PC resolved via mDNS: ");
      Serial.println(pcIp);
      return true;
    }
  }
  pcResolved = false;
  Serial.println("PC not resolved via mDNS");
  return false;
}

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

  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("PC hostname: ");
  Serial.println(PC_HOSTNAME);

  if (MDNS.begin("esp8266")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS: esp8266.local");
  }

  // 解析 PC 后端地址
  resolvePc();

  // 在 OLED 上显示 IP 地址 3 秒
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 16);
  display.println("IP:");
  display.setCursor(10, 30);
  display.println(WiFi.localIP());
  display.setCursor(10, 44);
  display.println(pcResolved ? "PC: liveface" : "PC: not found");
  display.display();
  delay(3000);

  showEmotion("HAPPY");

  server.on("/", []() {
    // 每次访问都重新解析 PC 地址，保证 IP 变了也能跟上
    resolvePc();
    if (pcResolved) {
      String url = "https://" + pcIp.toString();
      server.sendHeader("Location", url);
      server.send(302, "text/html", "Redirecting to " + url);
    } else {
      server.send(200, "text/html",
        "<h3>PC 后端未检测到</h3><p>请先启动电脑上的后端服务 (webapp/app.py)</p>"
        "<p>刷新本页重试</p>");
    }
  });

  server.on("/emotion", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      // 健壮解析：找到 "emotion" 键，跳过冒号和空格，取引号内的值
      int k = body.indexOf("emotion");
      if (k >= 0) {
        int colon = body.indexOf(':', k);
        int vq = body.indexOf('"', colon);
        if (vq >= 0) {
          int start = vq + 1;
          int end = body.indexOf('"', start);
          if (end > start) {
            currentEmotion = body.substring(start, end);
            showEmotion(currentEmotion);
          }
        }
      }
    }
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/esp_ip", []() {
    server.send(200, "text/plain", WiFi.localIP().toString());
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}