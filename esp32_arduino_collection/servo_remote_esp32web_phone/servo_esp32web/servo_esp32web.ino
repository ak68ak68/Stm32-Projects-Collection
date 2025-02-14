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
// 创建一个舵机对象，用于控制舵机的转动

WiFiServer server(80);
// 创建一个 WiFi 服务器对象，监听 80 端口，用于接收客户端（如手机浏览器）的请求

void setup() {
  // 初始化函数，在 ESP32 启动时执行一次
  Serial.begin(115200);
  // 初始化串口通信，波特率设置为 115200，用于调试信息的输出

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  // 分配 PWM（脉冲宽度调制）定时器，用于舵机控制，确保舵机能够正常工作
  myServo.setPeriodHertz(50);
  // 设置舵机的 PWM 频率为 50Hz，这是 SG90 舵机常用的频率
  myServo.attach(servoPin, 500, 2400);
  // 将舵机对象连接到指定的引脚（GPIO 18），并设置脉冲宽度范围为 500 到 2400 微秒，对应舵机的最小和最大角度

  WiFi.begin(ssid, password);
  // 启动 WiFi 连接，尝试连接到指定的 WiFi 网络
  while (WiFi.status() != WL_CONNECTED) {
    // 循环检查 WiFi 连接状态，直到连接成功
    delay(1000);
    // 每次检查间隔 1 秒
    Serial.println("Connecting to WiFi...");
    // 通过串口输出正在连接 WiFi 的提示信息
  }
  Serial.println("Connected to WiFi");
  // 打印 WiFi 连接成功的信息
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
  // 打印 ESP32 获取到的本地 IP 地址，方便后续在浏览器中访问

  server.begin();
  // 启动 WiFi 服务器，开始监听 80 端口的客户端请求
  Serial.println("Server started");
  // 打印服务器启动成功的信息
}

void loop() {
  // 主循环函数，会不断循环执行
  WiFiClient client = server.available();
  // 检查是否有新的客户端连接到服务器，如果有则获取该客户端对象
  if (client) {
    // 如果有新的客户端连接
    Serial.println("New client connected");
    // 打印新客户端连接的信息
    String currentLine = "";
    // 创建一个空字符串，用于存储当前读取的客户端请求行
    while (client.connected()) {
      // 循环检查客户端是否保持连接
      if (client.available()) {
        // 检查客户端是否有数据可读
        char c = client.read();
        // 从客户端读取一个字符
        Serial.write(c);
        // 将读取的字符通过串口输出，方便调试
        if (c == '\n') {
          // 如果读取到换行符
          if (currentLine.length() == 0) {
            // 如果当前行是空行，说明 HTTP 请求头结束
            client.println("HTTP/1.1 200 OK");
            // 向客户端发送 HTTP 响应状态码 200，表示请求成功
            client.println("Content-type:text/html");
            // 发送 HTTP 响应头，指定响应内容类型为 HTML
            client.println();
            // 发送空行，表示 HTTP 响应头结束

            // 以下是发送 HTML 页面内容
            client.println("<html>");
            client.println("<head>");
            client.println("<style>");
            client.println(".steering-wheel {");
            client.println("  width: 200px;");
            client.println("  height: 200px;");
            client.println("  background: url('https://p0.ssl.qhimgs1.com/t04df5437c576b57820.png') no-repeat center center;");
            // 设置方向盘的背景图片为占位符图片，可替换为实际的方向盘图片
            client.println("  background-size: cover;");
            client.println("  border-radius: 50%;");
            // 将元素设置为圆形，模拟方向盘的形状
            client.println("  position: relative;");
            client.println("  margin: 50px auto;");
            // 设置元素的外边距，使其在页面中居中显示
            client.println("}");
            client.println("</style>");

            client.println("<script>");
            client.println("document.addEventListener('DOMContentLoaded', function() {");
            // 当 HTML 文档加载完成后执行以下代码
            client.println("  const steeringWheel = document.querySelector('.steering-wheel');");
            // 获取页面中类名为 steering-wheel 的元素，即方向盘元素
            client.println("  let startAngle = 0;");
            // 定义一个变量，用于记录鼠标按下时的初始角度
            client.println("  let currentAngle = 0;");
            // 定义一个变量，用于记录当前方向盘的旋转角度

            client.println("  steeringWheel.addEventListener('mousedown', function(e) {");
            // 监听方向盘元素的鼠标按下事件
            client.println("    const rect = steeringWheel.getBoundingClientRect();");
            // 获取方向盘元素在页面中的位置和大小信息
            client.println("    const centerX = rect.left + rect.width / 2;");
            client.println("    const centerY = rect.top + rect.height / 2;");
            // 计算方向盘元素的中心点坐标
            client.println("    startAngle = Math.atan2(e.clientY - centerY, e.clientX - centerX) * (180 / Math.PI);");
            // 根据鼠标按下的位置计算初始角度
            client.println("    document.addEventListener('mousemove', onMouseMove);");
            client.println("    document.addEventListener('mouseup', onMouseUp);");
            // 监听鼠标移动和鼠标抬起事件
            client.println("  });");

            client.println("  function onMouseMove(e) {");
            // 鼠标移动事件处理函数
            client.println("    const rect = steeringWheel.getBoundingClientRect();");
            client.println("    const centerX = rect.left + rect.width / 2;");
            client.println("    const centerY = rect.top + rect.height / 2;");
            // 再次计算方向盘元素的中心点坐标
            client.println("    const newAngle = Math.atan2(e.clientY - centerY, e.clientX - centerX) * (180 / Math.PI);");
            // 根据鼠标当前位置计算新的角度
            client.println("    const deltaAngle = newAngle - startAngle;");
            // 计算角度变化量
            client.println("    currentAngle += deltaAngle;");
            // 更新当前旋转角度
            client.println("    currentAngle = Math.max(-180, Math.min(180, currentAngle));");
            // 将当前旋转角度限制在 -180 到 180 度之间
            client.println("    steeringWheel.style.transform = `rotate(${currentAngle}deg)`;");
            // 根据当前旋转角度旋转方向盘元素
            client.println("    startAngle = newAngle;");
            // 更新初始角度为新的角度

            client.println("    const xhr = new XMLHttpRequest();");
            // 创建一个 XMLHttpRequest 对象，用于向服务器发送请求
            client.println("    xhr.open('GET', `/setAngle?angle=${currentAngle + 180}`, true);");
            // 打开一个 GET 请求，将当前旋转角度加上 180 度后作为参数发送给服务器
            client.println("    xhr.send();");
            // 发送请求
            client.println("  }");

            client.println("  function onMouseUp() {");
            // 鼠标抬起事件处理函数
            client.println("    document.removeEventListener('mousemove', onMouseMove);");
            client.println("    document.removeEventListener('mouseup', onMouseUp);");
            // 移除鼠标移动和鼠标抬起事件的监听器
            client.println("  }");
            client.println("});");
            client.println("</script>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h1>SG90 Servo Control</h1>");
            // 显示页面标题
            client.println("<div class='steering-wheel'></div>");
            // 创建一个类名为 steering-wheel 的元素，即方向盘元素
            client.println("</body>");
            client.println("</html>");
            break;
            // 跳出循环，结束处理当前客户端请求
          } else {
            currentLine = "";
            // 清空当前行字符串，准备读取下一行
          }
        } else if (c != '\r') {
          currentLine += c;
          // 如果不是回车符，将字符添加到当前行字符串中
        }

        if (currentLine.endsWith("GET /setAngle?angle=")) {
          // 如果当前行以 "GET /setAngle?angle=" 结尾，说明接收到了设置舵机角度的请求
          String angleStr = "";
          // 创建一个空字符串，用于存储接收到的角度值
          char c;
          // 定义一个字符变量
          while (client.available()) {
            // 循环读取客户端数据，直到没有数据可读
            c = client.read();
            // 读取一个字符
            if (c == ' ') {
              // 如果读取到空格，说明角度值结束
              break;
              // 跳出循环
            }
            angleStr += c;
            // 将字符添加到角度值字符串中
          }
          int angle = angleStr.toInt();
          // 将角度值字符串转换为整数
          if (angle >= 0 && angle <= 360) {
            // 检查角度值是否在 0 到 360 度的有效范围内
            // 将 0 - 360 度映射到 0 - 180 度
            angle = angle / 2;
            myServo.write(angle);
            // 如果有效，将舵机转动到指定角度
          }
        }
      }
    }
    client.stop();
    // 关闭客户端连接
    Serial.println("Client disconnected");
    // 打印客户端断开连接的信息
  }
}
