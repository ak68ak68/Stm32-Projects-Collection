// 定义滑动变阻器连接的模拟引脚
const int potentiometerPin = A0;

void setup() {
  // 初始化串口通信，波特率为9600
  Serial.begin(9600);
}

void loop() {
  // 读取滑动变阻器的值
  int potentiometerValue = analogRead(potentiometerPin);
  
  // 将读取的值发送到串口
  Serial.println(potentiometerValue);
  
  // 延迟一段时间，避免频繁发送数据
  delay(100);
}
