# ESP8266 OLED 表情显示

仅保留 WiFi 连接 + OLED 显示表情功能，去除原项目的 RSSI/Ping/Loss 扫描和 UDP 发送。

## 接线

| OLED | NodeMCU | GPIO |
|------|---------|------|
| GND  | GND     | -    |
| VDD  | 3.3V    | -    |
| SCK  | D1      | GPIO5 |
| SDA  | D2      | GPIO4 |

## 使用

```bash
# 1. 配置 WiFi
cp src/config.example.h src/config.h
# 编辑 src/config.h，填入 WiFi 名称和密码

# 2. 编译烧录
pio run -t upload --upload-port COMx
```