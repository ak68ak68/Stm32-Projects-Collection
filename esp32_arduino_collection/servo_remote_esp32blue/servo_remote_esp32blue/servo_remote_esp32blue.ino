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
        // 将 String 类型转换为 const char* 类型
        const char* valueCStr = pCharacteristic->getValue().c_str();
        // 使用 std::string 的构造函数将 const char* 转换为 std::string 类型
        std::string value = std::string(valueCStr);

        if (value.length() > 0) {
            Serial.println("Received Value: " + String(value.c_str()));

            // 根据接收到的值控制 LED 灯
            if (value[0] == '1') {
                digitalWrite(ledPin, HIGH);
            } else if (value[0] == '0') {
                digitalWrite(ledPin, LOW);
            }
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
