#include <Arduino.h>

void setup() {
  // 初始化UART0串口，波特率115200
  Serial.begin(115200);
  
  // 等待串口初始化完成
  delay(1000);
  
  // 输出初始化信息
  Serial.println("ESP32C3 UART0 Communication Started");
  Serial.println("Sending AA BB CC DD every 1 second...");
}

void loop() {
  // 要发送的十六进制数据
  uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
  
  // 发送十六进制数据
  Serial.write(data, sizeof(data));
  
  // 可选：同时以可读格式输出到串口监视器
  Serial.print("Sent: ");
  for (int i = 0; i < sizeof(data); i++) {
    if (data[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // 等待1秒
  delay(1000);
}