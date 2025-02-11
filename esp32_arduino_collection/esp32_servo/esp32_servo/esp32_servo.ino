#include <ESP32Servo.h>
#define PIN_SG90 22  // 连接SG90舵机信号引脚的ESP32引脚
Servo sg90;
void setup() {
    sg90.setPeriodHertz(50);  // SG90舵机的PWM频率
    sg90.attach(PIN_SG90, 500, 2400);  // 连接舵机，设置最小和最大脉冲宽度
}
void loop() {
    // 从0度旋转到180度
    for (int pos = 0; pos <= 180; pos += 1) {
        sg90.write(pos);
        delay(10);
    }
    // 从180度旋转到0度
    for (int pos = 180; pos >= 0; pos -= 1) {
        sg90.write(pos);
        delay(10);
    }
}
