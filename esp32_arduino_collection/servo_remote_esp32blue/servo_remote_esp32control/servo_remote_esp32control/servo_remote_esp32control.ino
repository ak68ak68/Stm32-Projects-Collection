#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ESP32Servo.h>  // 使用ESP32专用舵机库[6](@ref)

/*-------------------------------------------
              硬件配置宏定义
-------------------------------------------*/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
const int ledPin = 2;        // LED引脚
const int servoPin = 22;     // 舵机信号引脚(GPIO22)
Servo myServo;               // 创建舵机对象[10](@ref)

/*-------------------------------------------
            BLE回调类（连接状态处理）
-------------------------------------------*/
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        digitalWrite(ledPin, HIGH);
    }

    void onDisconnect(BLEServer* pServer) {
        digitalWrite(ledPin, LOW);
        pServer->startAdvertising();
    }
};

/*-------------------------------------------
          BLE特征回调类（数据接收处理）
-------------------------------------------*/
class MyCharCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String cmd = pCharacteristic->getValue().c_str();
        int sepIndex = cmd.indexOf(':');
        
        if(sepIndex != -1) {
            String type = cmd.substring(0, sepIndex);
            int angle = cmd.substring(sepIndex+1).toInt();

            if(type == "SERVO") {
                angle = constrain(angle, 0, 180);  // 角度限幅[2](@ref)
                myServo.write(angle);              // 写入角度[10](@ref)
                Serial.print("舵机角度已设置：");
                Serial.println(angle);
            }
            else if(type == "LED") {
                digitalWrite(ledPin, angle ? HIGH : LOW);
            }
        }
    }
};

/*-------------------------------------------
              全局BLE对象声明
-------------------------------------------*/
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

/*-------------------------------------------
                 初始化配置
-------------------------------------------*/
void setup() {
    Serial.begin(115200);
    
    // 硬件初始化
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    
    // 舵机初始化[6](@ref)
    myServo.setPeriodHertz(50);          // 设置50Hz标准频率
    myServo.attach(servoPin, 500, 2500); // 脉宽范围500-2500μs
    myServo.write(90);                   // 初始位置设为90度

    // BLE初始化
    BLEDevice::init("ESP32-Servo");
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
    Serial.println("BLE已启动");
}

void loop() {
    delay(1000);
}
