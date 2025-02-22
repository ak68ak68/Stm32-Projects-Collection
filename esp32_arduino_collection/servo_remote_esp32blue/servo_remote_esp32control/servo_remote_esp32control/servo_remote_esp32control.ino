#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

/*-------------------------------------------
              硬件配置宏定义
-------------------------------------------*/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
const int ledPin = 2;        // LED引脚
const int servoPin = 22;     // 修改后的舵机信号引脚
const int PWM_FREQ = 50;     // SG90标准PWM频率(50Hz)
const int PWM_RESOLUTION = 16;// PWM分辨率

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
        String arduinoString = pCharacteristic->getValue();
        std::string value(arduinoString.c_str(), arduinoString.length());
        
        /* 协议规范：
         * [类型][分隔符][数值]
         * 示例：
         * LED:1       -> 开灯
         * SERVO:90    -> 舵机转90度
         */
        if(!value.empty()) {
            String cmd = String(value.c_str());
            int sepIndex = cmd.indexOf(':');
            
            if(sepIndex != -1) {
                String type = cmd.substring(0, sepIndex);
                int val = cmd.substring(sepIndex+1).toInt();

                if(type == "LED") {
                    digitalWrite(ledPin, val ? HIGH : LOW);
                    Serial.print("LED状态: ");
                    Serial.println(val ? "ON" : "OFF");
                }
                else if(type == "SERVO") {
                    val = constrain(val, 0, 180);  // 限制角度范围
                    int duty = map(val, 0, 180, 500, 2500); // 角度转脉宽(us)
                    ledcWrite(0, duty);  // 通道0输出PWM
                    Serial.print("舵机角度: ");
                    Serial.println(val);
                }
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
    
    // 舵机PWM配置（适配新版API）
    ledcAttach(servoPin, PWM_FREQ, PWM_RESOLUTION); // 自动分配通道
    
    // BLE初始化
    BLEDevice::init("ESP32-Servo-BLE");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // 创建服务
    pService = pServer->createService(SERVICE_UUID);
    
    // 配置特征
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new MyCharCallbacks());
    
    // 启动服务
    pService->start();

    // 广播配置
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    BLEDevice::startAdvertising();
    
    Serial.println("[BLE] 设备已就绪");
}

void loop() {
    delay(1000);
}
