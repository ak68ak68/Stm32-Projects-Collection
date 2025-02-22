#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// BLE服务UUID和特征UUID（需保证唯一性）
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
const int ledPin = 2;  // LED连接的GPIO引脚

// 自定义BLE特征回调类
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        /*--------------------------------------------------
          【关键修正点1】数据类型转换
          原错误：直接将Arduino String赋值给std::string
          现修正：显式转换数据类型
        --------------------------------------------------*/
        String arduinoString = pCharacteristic->getValue(); // 获取Arduino String类型数据
        std::string value(arduinoString.c_str(), arduinoString.length()); // 转换为std::string

        /*--------------------------------------------------
          调试输出：原始字节数据
          格式示例：Received raw data bytes: 01 
        --------------------------------------------------*/
        Serial.print("Received raw data bytes: ");
        for (size_t i = 0; i < value.length(); ++i) {
            Serial.print(static_cast<unsigned char>(value[i]), HEX); // 强制无符号转换
            Serial.print(" ");
        }
        Serial.println();

        /*--------------------------------------------------
          数据有效性检查
          要求：单字节数据 + 有效值（0x00或0x01）
        --------------------------------------------------*/
        if (value.length() == 1) {
            uint8_t byteValue = static_cast<uint8_t>(value[0]); // 提取字节值
            
            if (byteValue == 0x00 || byteValue == 0x01) {
                // LED控制逻辑
                digitalWrite(ledPin, byteValue ? HIGH : LOW);
                Serial.print("LED state: ");
                Serial.println(byteValue ? "ON" : "OFF");
            } else {
                Serial.println("ERROR: Invalid value! Send 0x00 (OFF) or 0x01 (ON)");
            }
        } else {
            Serial.println("ERROR: Invalid length! Send exactly 1 byte");
        }
    }
};

// BLE相关对象声明
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

void setup() {
    Serial.begin(115200);  // 初始化串口
    pinMode(ledPin, OUTPUT);  // 配置LED引脚
    
    /*--------------------------------------------------
      BLE初始化流程
    --------------------------------------------------*/
    BLEDevice::init("ESP32_LED_Controller");  // 设备名称
    pServer = BLEDevice::createServer();      // 创建BLE服务器
    pService = pServer->createService(SERVICE_UUID); // 创建服务
    
    // 创建可写特征
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    
    pCharacteristic->setCallbacks(new MyCallbacks()); // 设置回调
    pService->start(); // 启动服务
    
    // 广播配置
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID); // 广播服务UUID
    pAdvertising->setScanResponse(true);        // 允许扫描响应
    BLEDevice::startAdvertising();             // 开始广播
    Serial.println("BLE Ready!");               // 初始化完成提示
}

void loop() {
    // BLE事件通过回调自动处理，无需额外操作
    delay(2000);
}
