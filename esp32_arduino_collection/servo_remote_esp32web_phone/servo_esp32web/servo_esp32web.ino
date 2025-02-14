#include <WiFi.h>
// 引入WiFi库，用于ESP32连接到无线网络
#include <ESP32Servo.h>
// 引入ESP32Servo库，用于控制舵机

// 替换为你的网络信息
const char* ssid = "360WiFi-F3C43C";
// 定义WiFi网络的名称
const char* password = "18962426951";
// 定义WiFi网络的密码

// 舵机控制引脚
const int servoPin = 22;

// 定义舵机信号引脚连接到ESP32的GPIO 22
Servo myServo;
// 创建一个舵机对象

WiFiServer server(80);
// 创建一个WiFi服务器对象，监听80端口

void setup() {
  // 初始化函数，在ESP32启动时执行一次
  Serial.begin(115200);
  // 初始化串口通信，波特率为115200

  // 允许舵机库使用更多引脚
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  // 分配PWM定时器，用于舵机控制
  myServo.setPeriodHertz(50);
  // 设置舵机的PWM频率为50Hz
  myServo.attach(servoPin, 500, 2400);
  // 将舵机对象连接到指定引脚，并设置脉冲宽度范围为500到2400微秒

  WiFi.begin(ssid, password);
  // 启动WiFi连接，传入WiFi名称和密码
  while (WiFi.status() != WL_CONNECTED) {
    // 循环检查WiFi连接状态，直到连接成功
    delay(1000);
    // 每次检查间隔1秒
    Serial.println("Connecting to WiFi...");
    // 打印正在连接WiFi的提示信息
  }
  Serial.println("Connected to WiFi");
  // 打印WiFi连接成功信息
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP()); // 打印ESP32的IP地址

  server.begin();
  // 启动WiFi服务器
  Serial.println("Server started");
  // 打印服务器启动成功信息
}

void loop() {
  // 主循环函数，不断循环执行
  WiFiClient client = server.available();
  if (client) {
    // 如果有新的客户端连接
    Serial.println("New client connected");
    String currentLine = "";
    int currentAngle = 90; // 默认初始角度为90度

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<html><body>");
            client.println("<h1>SG90 Servo Control</h1>");
            // 构建带有当前角度值的滑块
            client.print("<form method='get'>");
            client.print("<input type='range' name='angle' min='0' max='180' value='");
            client.print(currentAngle);
            client.print("' oninput='this.form.submit()'>");
            client.println("</form>");
            client.println("</body></html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        if (currentLine.endsWith("GET /?angle=")) {
          String angleStr = "";
          char c;
          while (client.available()) {
            c = client.read();
            if (c == ' ') {
              break;
            }
            angleStr += c;
          }
          int angle = angleStr.toInt();
          if (angle >= 0 && angle <= 180) {
            myServo.write(angle);
            currentAngle = angle; // 更新当前角度值
          }
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}
