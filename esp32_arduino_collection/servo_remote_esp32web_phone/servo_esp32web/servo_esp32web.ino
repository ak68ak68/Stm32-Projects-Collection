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
  // 检查是否有新的客户端连接到服务器
  if (client) {
    // 如果有新的客户端连接
    Serial.println("New client connected");
    // 打印新客户端连接的提示信息
    String currentLine = "";
    // 创建一个空字符串用于存储当前读取的行
    while (client.connected()) {
      // 循环检查客户端连接状态，直到客户端断开连接
      if (client.available()) {
        // 检查客户端是否有数据可读
        char c = client.read();
        // 读取一个字符
        Serial.write(c);
        // 将读取的字符通过串口输出
        if (c == '\n') {
          // 如果读取到换行符
          if (currentLine.length() == 0) {
            // 如果当前行是空行，表示HTTP请求头结束
            client.println("HTTP/1.1 200 OK");
            // 发送HTTP响应状态码200，表示请求成功
            client.println("Content-type:text/html");
            // 发送HTTP响应头，指定响应内容类型为HTML
            client.println();
            // 发送空行，表示HTTP响应头结束
            client.println("<html><body>");
            // 发送HTML页面开始标签
            client.println("<h1>SG90 Servo Control</h1>");
            // 发送HTML标题
            client.println("<form method='get'>");
            // 发送HTML表单开始标签，使用GET方法提交数据
            client.println("<input type='number' name='angle' min='0' max='180'>");
            // 发送一个数字输入框，用于输入舵机角度，范围为0到180
            client.println("<input type='submit' value='Set Angle'>");
            // 发送一个提交按钮
            client.println("</form>");
            // 发送HTML表单结束标签
            client.println("</body></html>");
            // 发送HTML页面结束标签
            break;
            // 跳出循环
          } else {
            currentLine = "";
            // 清空当前行字符串
          }
        } else if (c != '\r') {
          currentLine += c;
          // 如果不是回车符，将字符添加到当前行字符串中
        }
        if (currentLine.endsWith("GET /?angle=")) {
          // 如果当前行以"GET /?angle="结尾，表示接收到了设置舵机角度的请求
          String angleStr = "";
          // 创建一个空字符串用于存储角度值
          char c;
          // 定义一个字符变量
          while (client.available()) {
            // 循环读取客户端数据，直到没有数据可读
            c = client.read();
            // 读取一个字符
            if (c == ' ') {
              // 如果读取到空格，表示角度值结束
              break;
              // 跳出循环
            }
            angleStr += c;
            // 将字符添加到角度值字符串中
          }
          int angle = angleStr.toInt();
          // 将角度值字符串转换为整数
          if (angle >= 0 && angle <= 180) {
            // 检查角度是否在0到180度的有效范围内
            myServo.write(angle);
            // 如果有效，将舵机转动到指定角度
          }
        }
      }
    }
    client.stop();
    // 关闭客户端连接
    Serial.println("Client disconnected");
    // 打印客户端断开连接的提示信息
  }
}
