#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// 定义服务和特征的 UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// 定义 LED 连接的引脚
const int ledPin = 2;

// 全局变量
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// 自定义回调类，处理蓝牙连接和断开事件
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Client connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Client disconnected!");
    }
};

// 自定义回调类，处理特征值的写入事件
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        const char* valueCStr = pCharacteristic->getValue().c_str();
        std::string value = std::string(valueCStr);

        // 打印接收到的原始数据，方便调试
        Serial.print("Received raw data: ");
        Serial.println(value.c_str());

        // 检查接收到的数据长度
        if (value.length() > 0) {
            // 将十六进制字符串转换为十进制整数
            char* endptr;
            long hexValue = strtol(value.c_str(), &endptr, 16);

            // 检查转换是否成功
            if (*endptr == '\0') {
                Serial.print("Converted hex value: ");
                Serial.println(hexValue);

                // 根据转换后的十进制值控制 LED 灯
                if (hexValue == 1) {
                    digitalWrite(ledPin, HIGH);
                } else if (hexValue == 0) {
                    digitalWrite(ledPin, LOW);
                } else {
                    Serial.println("Invalid hex value received!");
                }
            } else {
                Serial.println("Invalid hex string received!");
            }
        } else {
            Serial.println("Received empty data!");
        }
    }
};

void setup() {
    // 初始化串口通信，用于调试信息输出
    Serial.begin(115200);

    // 初始化 LED 引脚
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // 初始化 BLE 设备
    BLEDevice::init("ESP32_BLE_LED");

    // 创建 BLE 服务器
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // 创建 BLE 服务
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // 创建 BLE 特征
    pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );

    pCharacteristic->setCallbacks(new MyCallbacks());

    // 添加客户端特征配置描述符
    pCharacteristic->addDescriptor(new BLE2902());

    // 启动服务
    pService->start();

    // 启动蓝牙广播
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // 可以在这里添加其他任务
    delay(1000);
}
