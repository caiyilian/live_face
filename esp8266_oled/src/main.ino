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

// ========== HTML 页面（分两段，中间插入 PC_IP） ==========

const char HTML_A[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>人脸表情识别</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,sans-serif;background:#1a1a2e;color:#eee;display:flex;flex-direction:column;align-items:center;min-height:100vh;padding:20px}
h1{margin:20px 0;font-size:24px}
.container{position:relative;width:640px;max-width:100%;min-height:480px;background:#111;border-radius:12px;display:flex;align-items:center;justify-content:center}
#video,#stream{width:100%;border-radius:12px;display:none}
#video.show,#stream.show{display:block}
.placeholder{color:#555;font-size:16px}
.controls{margin-top:16px;display:flex;gap:12px;flex-wrap:wrap;justify-content:center}
.btn{padding:10px 24px;border:none;border-radius:8px;font-size:15px;cursor:pointer;transition:all .2s}
.btn:disabled{opacity:.4;cursor:not-allowed}
.btn-primary{background:#0f3460;color:#fff}
.btn-success{background:#16a34a;color:#fff}
.btn-danger{background:#dc2626;color:#fff}
.status{margin-top:12px;font-size:14px;color:#94a3b8}
</style>
</head>
<body>
<h1>人脸表情识别</h1>
<div class="container">
<video id="video" autoplay playsinline></video>
<img id="stream" style="display:none">
<span class="placeholder" id="placeholder">摄像头未开启</span>
</div>
<div class="controls">
<button id="btnOpen" class="btn btn-primary">打开摄像头</button>
<button id="btnStart" class="btn btn-success" disabled>开启表情识别</button>
<button id="btnStop" class="btn btn-danger" disabled>停止</button>
</div>
<div class="status" id="status">等待操作...</div>
<script>
const PC_IP = ')rawliteral";

const char HTML_B[] PROGMEM = R"rawliteral(';
const PC_PORT = 5000;
const v=document.getElementById('video'),si=document.getElementById('stream'),ph=document.getElementById('placeholder');
const bo=document.getElementById('btnOpen'),bs=document.getElementById('btnStart'),bt=document.getElementById('btnStop');
const st=document.getElementById('status');
let ms=null,ir=false;
bo.addEventListener('click',async()=>{
try{ms=await navigator.mediaDevices.getUserMedia({video:true});v.srcObject=ms;ph.style.display='none';v.classList.add('show');si.style.display='none';bo.disabled=true;bs.disabled=false;st.textContent='\u6444\u50cf\u5934\u5df2\u6253\u5f00';\u70b9\u51fb\u201c\u5f00\u542f\u8868\u60c5\u8bc6\u522b\u201d\u5f00\u59cb\u8bc6\u522b'}catch(e){st.textContent='\u65e0\u6cd5\u6253\u5f00\u6444\u50cf\u5934: '+e.message}});
bs.addEventListener('click',()=>{
if(!ms)return;ms.getTracks().forEach(t=>t.stop());ms=null;v.classList.remove('show');ir=true;si.style.display='block';si.src='http://'+PC_IP+':'+PC_PORT+'/video_feed?_t='+Date.now();bs.disabled=true;bt.disabled=false;bo.disabled=true;st.textContent='\u8868\u60c5\u8bc6\u522b\u5df2\u5f00\u542f'});
bt.addEventListener('click',async()=>{
ir=false;si.style.display='none';si.src='';await fetch('http://'+PC_IP+':'+PC_PORT+'/stop');try{ms=await navigator.mediaDevices.getUserMedia({video:true});v.srcObject=ms;v.classList.add('show');bo.disabled=true;bs.disabled=false;bt.disabled=true;st.textContent='\u6444\u50cf\u5934\u5df2\u6062\u590d'}catch(e){bo.disabled=false;bs.disabled=true;bt.disabled=true;st.textContent='\u8bf7\u91cd\u65b0\u6253\u5f00\u6444\u50cf\u5934'}});
</script>
</body>
</html>
)rawliteral";

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
  Serial.print("PC IP: ");
  Serial.println(PC_IP);

  if (MDNS.begin("esp8266")) {
    Serial.println("mDNS: http://esp8266.local");
  }

  showEmotion("HAPPY");

  server.on("/", []() {
    String html = FPSTR(HTML_A) + String(PC_IP) + FPSTR(HTML_B);
    server.send(200, "text/html", html);
  });

  server.on("/emotion", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      int s = body.indexOf("\"emotion\":\"") + 10;
      int e = body.indexOf("\"", s);
      if (s > 9 && e > s) {
        currentEmotion = body.substring(s, e);
        showEmotion(currentEmotion);
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