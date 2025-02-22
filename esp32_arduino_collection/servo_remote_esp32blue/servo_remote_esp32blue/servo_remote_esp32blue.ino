#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

/*-------------------------------------------
              硬件配置宏定义
-------------------------------------------*/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // BLE服务UUID
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" // 特征UUID
const int ledPin = 2;  // LED连接的GPIO引脚

/*-------------------------------------------
            BLE服务器回调类（连接状态处理）
-------------------------------------------*/
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("[BLE] 设备已连接");
        digitalWrite(ledPin, HIGH); // 连接时点亮LED
    }

    void onDisconnect(BLEServer* pServer) {
        Serial.println("[BLE] 设备已断开，正在重启广播...");
        digitalWrite(ledPin, LOW);  // 断开时熄灭LED
        pServer->startAdvertising(); // 核心：重启广播
    }
};

/*-------------------------------------------
          BLE特征回调类（数据接收处理）
-------------------------------------------*/
class MyCharCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        // 获取原始数据（兼容Arduino String类型）
        String arduinoString = pCharacteristic->getValue();
        std::string value(arduinoString.c_str(), arduinoString.length());

        // 调试输出：显示原始字节数据（HEX格式）
        Serial.print("[DATA] 接收原始字节: ");
        for (size_t i = 0; i < value.length(); ++i) {
            Serial.print(static_cast<unsigned char>(value[i]), HEX);
            Serial.print(" ");
        }
        Serial.println();

        /*-------------------------------------------
                      LED控制逻辑
        协议规范：
        - 单字节数据
        - 0x00: 关闭LED
        - 0x01: 打开LED
        -------------------------------------------*/
        if (value.length() == 1) {
            uint8_t byteValue = static_cast<uint8_t>(value[0]);
            
            if (byteValue == 0x00 || byteValue == 0x01) {
                digitalWrite(ledPin, byteValue ? HIGH : LOW);
                Serial.print("[LED] 状态: ");
                Serial.println(byteValue ? "ON" : "OFF");
            } else {
                Serial.println("[ERROR] 无效数值！请发送0x00或0x01");
            }
        } else {
            Serial.println("[ERROR] 数据长度错误！请发送单字节数据");
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
    // 硬件初始化
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); // 初始关闭LED

    // BLE协议栈初始化
    BLEDevice::init("ESP32-LED-CTRL"); // 设备名称
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); // 设置连接状态回调

    // 创建BLE服务
    pService = pServer->createService(SERVICE_UUID);
    
    // 配置可写特征
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new MyCharCallbacks()); // 设置数据回调
    
    // 启动服务
    pService->start();

    // 广播配置
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);    // 携带服务UUID
    pAdvertising->setScanResponse(true);           // 允许扫描响应
    pAdvertising->setMinPreferred(0x06);           // 优化连接参数
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("[BLE] 就绪，等待连接...");
}

/*-------------------------------------------
                 主循环
-------------------------------------------*/
void loop() {
    // BLE事件通过回调处理，无需在此操作
    delay(1000); // 降低CPU占用
}
