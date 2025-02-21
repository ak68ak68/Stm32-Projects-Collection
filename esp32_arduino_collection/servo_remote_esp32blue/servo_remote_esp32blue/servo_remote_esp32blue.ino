#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// 定义服务和特征的 UUID
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// 定义指示灯连接的引脚
const int ledPin = 2;

// 全局变量
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// 自定义回调类，用于处理蓝牙连接和断开事件
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

// 自定义回调类，用于处理特征值的写入事件
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      // 获取特征值并转换为 std::string
      const char* valueCStr = pCharacteristic->getValue().c_str();
      std::string value = std::string(valueCStr);

      if (value.length() > 0) {
        Serial.println("Received Value: " + String(value.c_str()));

        // 根据接收到的值控制指示灯
        if (value[0] == '1') {
          digitalWrite(ledPin, HIGH);
        } else if (value[0] == '0') {
          digitalWrite(ledPin, LOW);
        }
      }
    }
};

void setup() {
  // 初始化串口通信
  Serial.begin(115200);

  // 初始化指示灯引脚
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 创建蓝牙设备
  BLEDevice::init("ESP32_BLE_LED");

  // 创建蓝牙服务器
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 创建蓝牙服务
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 创建蓝牙特征
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
