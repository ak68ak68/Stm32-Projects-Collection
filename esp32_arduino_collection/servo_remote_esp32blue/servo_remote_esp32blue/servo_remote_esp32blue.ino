#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
const int ledPin = 2;  // 假设LED连接GPIO2

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        // 正确获取二进制数据（兼容Arduino String类型）
        String arduinoString = pCharacteristic->getValue();
        std::string value(arduinoString.c_str(), arduinoString.length());

        // 调试输出：显示原始字节
        Serial.print("Received raw data bytes: ");
        for (size_t i = 0; i < value.length(); ++i) {
            Serial.print(static_cast<unsigned char>(value[i]), HEX);
            Serial.print(" ");
        }
        Serial.println();

        // 数据有效性检查
        if (value.length() == 1) {
            uint8_t byteValue = static_cast<uint8_t>(value[0]);
            
            if (byteValue == 0x00 || byteValue == 0x01) {
                digitalWrite(ledPin, byteValue ? HIGH : LOW);
                Serial.print("LED state: ");
                Serial.println(byteValue ? "ON" : "OFF");
            } else {
                Serial.println("Invalid value! Send 0x00 (OFF) or 0x01 (ON)");
            }
        } else {
            Serial.println("Invalid length! Send exactly 1 byte");
        }
    }
};

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    
    BLEDevice::init("ESP32_LED_Controller");
    pServer = BLEDevice::createServer();
    pService = pServer->createService(SERVICE_UUID);
    
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    
    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
    Serial.println("BLE Ready!");
}

void loop() {
    // 保持空循环，BLE事件通过回调处理
    delay(2000);
}
