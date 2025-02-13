#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32Servo.h>

// 替换为你的网络信息
const char* ssid = "360WiFi-F3C43C";
const char* password = "18962426951";

// 服务器的IP地址和端口号
const char* serverIP = "192.168.0.14";
const int serverPort = 8888;

// 定义舵机连接的引脚
const int servoPin = 22;
Servo myServo;

WiFiClient client;

void setup() {
  // 初始化串口通信，波特率为115200，方便查看调试信息
  Serial.begin(115200);
  
  // 初始化舵机
  myServo.attach(servoPin);
  
  // 连接到WiFi网络
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // 尝试连接到服务器
  Serial.print("Trying to connect to server at ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.println(serverPort);
  
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server successfully!");
  } else {
    Serial.println("Failed to connect to server.");
  }
}

void loop() {
  if (client.connected()) {
    if (client.available()) {
      // 读取来自服务器的滑动变阻器值
      String potentiometerValueStr = client.readStringUntil('\n');
      int potentiometerValue = potentiometerValueStr.toInt();
      
      // 将滑动变阻器的值等比例转换为舵机角度（0 - 180度）
      int servoAngle = map(potentiometerValue, 0, 1023, 0, 180);
      
      // 驱动舵机转动到指定角度
      myServo.write(servoAngle);
      
      // 打印调试信息
      Serial.print("Potentiometer value: ");
      Serial.print(potentiometerValue);
      Serial.print(", Servo angle: ");
      Serial.println(servoAngle);
    }
  } else {
    // 尝试重新连接到服务器
    Serial.println("Connection lost. Trying to reconnect...");
    if (client.connect(serverIP, serverPort)) {
      Serial.println("Reconnected to server");
    } else {
      Serial.println("Reconnection failed");
      delay(5000);
    }
  }
  
  delay(10);
}
