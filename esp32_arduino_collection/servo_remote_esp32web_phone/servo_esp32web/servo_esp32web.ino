
#include <WiFi.h>
#include <ESP32Servo.h>

// 替换为你的网络信息
const char* ssid = "360WiFi-F3C43C";
// 定义WiFi网络的名称
const char* password = "18962426951";
// 定义WiFi网络的密码

// 舵机控制引脚
const int servoPin = 22;

// 定义舵机信号引脚连接到ESP32的GPIO 22



Servo myServo;

WiFiServer server(80);

// 滤波参数
#define FILTER_SIZE 5
int angleFilter[FILTER_SIZE];
int filterIndex = 0;
int filterSum = 0;

void setup() {
  Serial.begin(115200);

  // 分配 PWM 定时器，用于舵机控制
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  // 尝试调整 PWM 频率，这里设置为 60Hz
  myServo.setPeriodHertz(50);
  // 尝试调整脉冲宽度范围
  myServo.attach(servoPin, 500, 2500);

  // 启动 WiFi 连接
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  // 启动 WiFi 服务器，监听 80 端口
  server.begin();
  Serial.println("Server started");

  // 初始化滤波数组
  for (int i = 0; i < FILTER_SIZE; i++) {
    angleFilter[i] = 0;
  }
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // 发送 HTTP 响应头
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // 发送 HTML 页面内容
            client.println("<html>");
            client.println("<head>");
            client.println("<style>");
            client.println(".steering-wheel {");
            client.println("  width: 200px;");
            client.println("  height: 200px;");
            // 设置方向盘的背景图片为网络 URL 图片，需替换为实际有效的图片链接
            client.println("  background: url('https://preview.qiantucdn.com/58pic/38/36/68/44g58PICFpGMuDuI6heER_PIC2018.png!qt_w320') no-repeat center center;");
            client.println("  background-size: cover;");
            client.println("  border-radius: 50%;");
            client.println("  position: relative;");
            client.println("  margin: 50px auto;");
            client.println("}");
            client.println("</style>");
            client.println("<script>");
            client.println("document.addEventListener('DOMContentLoaded', function() {");
            client.println("  const steeringWheel = document.querySelector('.steering-wheel');");
            client.println("  let startAngle = 0;");
            client.println("  let currentAngle = 0;");
            client.println("  let lastSendTime = 0;");
            client.println("  const sendInterval = 100; // 发送间隔，单位为毫秒");

            // 处理鼠标按下事件
            client.println("  steeringWheel.addEventListener('mousedown', function(e) {");
            client.println("    const rect = steeringWheel.getBoundingClientRect();");
            client.println("    const centerX = rect.left + rect.width / 2;");
            client.println("    const centerY = rect.top + rect.height / 2;");
            client.println("    startAngle = Math.atan2(e.clientY - centerY, e.clientX - centerX) * (180 / Math.PI);");
            client.println("    document.addEventListener('mousemove', onMouseMove);");
            client.println("    document.addEventListener('mouseup', onMouseUp);");
            client.println("  });");

            // 处理触摸开始事件
            client.println("  steeringWheel.addEventListener('touchstart', function(e) {");
            client.println("    const rect = steeringWheel.getBoundingClientRect();");
            client.println("    const centerX = rect.left + rect.width / 2;");
            client.println("    const centerY = rect.top + rect.height / 2;");
            client.println("    const touch = e.touches[0];");
            client.println("    startAngle = Math.atan2(touch.clientY - centerY, touch.clientX - centerX) * (180 / Math.PI);");
            client.println("    document.addEventListener('touchmove', onTouchMove);");
            client.println("    document.addEventListener('touchend', onTouchEnd);");
            client.println("  });");

            // 鼠标移动事件处理函数
            client.println("  function onMouseMove(e) {");
            client.println("    const now = Date.now();");
            client.println("    if (now - lastSendTime < sendInterval) {");
            client.println("      return;");
            client.println("    }");
            client.println("    lastSendTime = now;");
            client.println("    const rect = steeringWheel.getBoundingClientRect();");
            client.println("    const centerX = rect.left + rect.width / 2;");
            client.println("    const centerY = rect.top + rect.height / 2;");
            client.println("    const newAngle = Math.atan2(e.clientY - centerY, e.clientX - centerX) * (180 / Math.PI);");
            client.println("    const deltaAngle = newAngle - startAngle;");
            client.println("    currentAngle += deltaAngle;");
            // 将方向盘旋转角度限制在 -90 到 90 度之间
            client.println("    currentAngle = Math.max(-90, Math.min(90, currentAngle));");
            client.println("    steeringWheel.style.transform = `rotate(${currentAngle}deg)`;");
            client.println("    startAngle = newAngle;");
            client.println("    const xhr = new XMLHttpRequest();");
            // 发送请求，将角度数据传递给 ESP32
            client.println("    xhr.open('GET', `/setAngle?angle=${currentAngle + 90}`, true);");
            client.println("    xhr.send();");
            client.println("  }");

            // 触摸移动事件处理函数
            client.println("  function onTouchMove(e) {");
            client.println("    const now = Date.now();");
            client.println("    if (now - lastSendTime < sendInterval) {");
            client.println("      return;");
            client.println("    }");
            client.println("    lastSendTime = now;");
            client.println("    const rect = steeringWheel.getBoundingClientRect();");
            client.println("    const centerX = rect.left + rect.width / 2;");
            client.println("    const centerY = rect.top + rect.height / 2;");
            client.println("    const touch = e.touches[0];");
            client.println("    const newAngle = Math.atan2(touch.clientY - centerY, touch.clientX - centerX) * (180 / Math.PI);");
            client.println("    const deltaAngle = newAngle - startAngle;");
            client.println("    currentAngle += deltaAngle;");
            // 将方向盘旋转角度限制在 -90 到 90 度之间
            client.println("    currentAngle = Math.max(-90, Math.min(90, currentAngle));");
            client.println("    steeringWheel.style.transform = `rotate(${currentAngle}deg)`;");
            client.println("    startAngle = newAngle;");
            client.println("    const xhr = new XMLHttpRequest();");
            // 发送请求，将角度数据传递给 ESP32
            client.println("    xhr.open('GET', `/setAngle?angle=${currentAngle + 90}`, true);");
            client.println("    xhr.send();");
            client.println("  }");

            // 鼠标抬起事件处理函数
            client.println("  function onMouseUp() {");
            client.println("    document.removeEventListener('mousemove', onMouseMove);");
            client.println("    document.removeEventListener('mouseup', onMouseUp);");
            client.println("  }");

            // 触摸结束事件处理函数
            client.println("  function onTouchEnd() {");
            client.println("    document.removeEventListener('touchmove', onTouchMove);");
            client.println("    document.removeEventListener('touchend', onTouchEnd);");
            client.println("  }");
            client.println("});");
            client.println("</script>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h1>SG90 Servo Control</h1>");
            client.println("<div class='steering-wheel'></div>");
            client.println("</body>");
            client.println("</html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /setAngle?angle=")) {
          String angleStr = "";
          char c;
          while (client.available()) {
            c = client.read();
            if (c == ' ') {
              break;
            }
            angleStr += c;
          }
          int newAngle = angleStr.toInt();
          if (newAngle >= 0 && newAngle <= 180) {
            // 滤波处理
            filterSum -= angleFilter[filterIndex];
            angleFilter[filterIndex] = newAngle;
            filterSum += newAngle;
            filterIndex = (filterIndex + 1) % FILTER_SIZE;
            int filteredAngle = filterSum / FILTER_SIZE;

            // 控制舵机转动
            myServo.write(filteredAngle);
            // 添加 20 毫秒的延时，让舵机有足够时间完成动作
            delay(20); 
          }
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}
