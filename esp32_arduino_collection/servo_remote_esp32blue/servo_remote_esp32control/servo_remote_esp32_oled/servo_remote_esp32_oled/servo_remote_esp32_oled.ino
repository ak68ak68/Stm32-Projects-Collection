#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ESP32Servo.h>

/*-------------------------------------------
              硬件配置宏定义
-------------------------------------------*/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // 创建OLED对象[4](@ref)

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
const int ledPin = 2;
const int servoPin = 18;
Servo myServo;

// 状态变量
bool bleConnected = false;
int currentAngle = 90;
bool ledState = LOW;

/*-------------------------------------------
            OLED显示更新函数
-------------------------------------------*/
void updateDisplay() {
  display.clearDisplay();
  
  // 显示标题
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("ESP32 Status Monitor");
  
  // 绘制分隔线
  display.drawLine(0,12,128,12,SSD1306_WHITE);
  
  // 显示蓝牙状态
  display.setCursor(0,20);
  display.print("BLE: ");
  display.print(bleConnected ? "Connected" : "Disconnected");
  
  // 显示舵机角度
  display.setCursor(0,35);
  display.print("Servo: ");
  display.print(currentAngle);
  display.print((char)247);  // 显示°符号
  
  // 显示LED状态
  display.setCursor(0,50);
  display.print("LED: ");
  display.print(ledState ? "ON " : "OFF");
  
  display.display();
}

/*-------------------------------------------
            BLE回调类
-------------------------------------------*/
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleConnected = true;
      digitalWrite(ledPin, HIGH);
      ledState = HIGH;
      updateDisplay();
    }

    void onDisconnect(BLEServer* pServer) {
      bleConnected = false;
      digitalWrite(ledPin, LOW);
      ledState = LOW;
      pServer->startAdvertising();
      updateDisplay();
    }
};

class MyCharCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String cmd = pCharacteristic->getValue().c_str();
      int sepIndex = cmd.indexOf(':');
      
      if(sepIndex != -1) {
        String type = cmd.substring(0, sepIndex);
        int val = cmd.substring(sepIndex+1).toInt();

        if(type == "SERVO") {
          val = constrain(val, 0, 180);
          myServo.write(val);
          currentAngle = val;
          updateDisplay();
        }
        else if(type == "LED") {
          ledState = val ? HIGH : LOW;
          digitalWrite(ledPin, ledState);
          updateDisplay();
        }
      }
    }
};

/*-------------------------------------------
              全局BLE对象
-------------------------------------------*/
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

void setup() {
  Serial.begin(115200);
  
  // OLED初始化[4,7](@ref)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  
  // 硬件初始化
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // 舵机初始化[6](@ref)
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 500, 2500);
  myServo.write(currentAngle);

  // BLE初始化[4](@ref)
  BLEDevice::init("SmartServo-OLED");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new MyCharCallbacks());
  
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  // 初始显示
  updateDisplay();
}

void loop() {
  delay(1000);
  // 定期刷新显示（防止数据残留）
  updateDisplay(); 
}
