// 引入蓝牙低功耗（BLE）设备相关的库，用于实现 BLE 功能
#include <BLEDevice.h>
// 引入 BLE 工具库，提供一些辅助功能
#include <BLEUtils.h>
// 引入 BLE 服务器库，用于创建 BLE 服务器
#include <BLEServer.h>
// 引入 ESP32 专用的舵机库，用于控制舵机
#include <ESP32Servo.h>  

/*-------------------------------------------
              硬件配置宏定义
-------------------------------------------*/
// 定义 BLE 服务的 UUID，用于标识 BLE 服务，客户端需要通过此 UUID 来识别服务
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// 定义 BLE 特征的 UUID，用于标识服务中的具体特征，客户端通过此 UUID 来与该特征进行交互
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// 定义 LED 连接的 GPIO 引脚，用于控制 LED 的亮灭
const int ledPin = 2;        
// 定义舵机信号连接的 GPIO 引脚，用于向舵机发送控制信号
const int servoPin = 22;     
// 创建一个舵机对象，用于控制舵机的转动
Servo myServo;               

/*-------------------------------------------
            BLE回调类（连接状态处理）
-------------------------------------------*/
// 自定义一个类，继承自 BLEServerCallbacks，用于处理 BLE 服务器的连接和断开事件
class MyServerCallbacks: public BLEServerCallbacks {
    // 当有客户端连接到 BLE 服务器时，此函数会被调用
    void onConnect(BLEServer* pServer) {
        // 连接成功后，将 LED 引脚设置为高电平，点亮 LED
        digitalWrite(ledPin, HIGH);
    }

    // 当客户端与 BLE 服务器断开连接时，此函数会被调用
    void onDisconnect(BLEServer* pServer) {
        // 断开连接后，将 LED 引脚设置为低电平，熄灭 LED
        digitalWrite(ledPin, LOW);
        // 重新启动 BLE 广播，以便其他客户端可以发现该设备
        pServer->startAdvertising();
    }
};

/*-------------------------------------------
          BLE特征回调类（数据接收处理）
-------------------------------------------*/
// 自定义一个类，继承自 BLECharacteristicCallbacks，用于处理 BLE 特征的写入事件
class MyCharCallbacks: public BLECharacteristicCallbacks {
    // 当客户端向 BLE 特征写入数据时，此函数会被调用
    void onWrite(BLECharacteristic *pCharacteristic) {
        // 获取客户端写入的数据，并将其转换为 Arduino 的 String 类型
        String cmd = pCharacteristic->getValue().c_str();
        // 查找数据中冒号（:）的位置，用于分割命令类型和参数
        int sepIndex = cmd.indexOf(':');
        
        // 如果找到了冒号，说明数据格式符合要求
        if(sepIndex != -1) {
            // 从数据中提取命令类型，即冒号之前的部分
            String type = cmd.substring(0, sepIndex);
            // 从数据中提取参数，并将其转换为整数类型，即冒号之后的部分
            int angle = cmd.substring(sepIndex+1).toInt();

            // 如果命令类型为 "SERVO"，表示要控制舵机
            if(type == "SERVO") {
                // 将角度限制在 0 到 180 度之间，避免舵机超出其转动范围
                angle = constrain(angle, 0, 180);  
                // 向舵机写入指定的角度，控制舵机转动到该角度
                myServo.write(angle);              
                // 通过串口输出提示信息，显示舵机已设置的角度
                Serial.print("舵机角度已设置：");
                Serial.println(angle);
            }
            // 如果命令类型为 "LED"，表示要控制 LED
            else if(type == "LED") {
                // 根据参数的值（0 或 1），将 LED 引脚设置为低电平或高电平，控制 LED 的亮灭
                digitalWrite(ledPin, angle ? HIGH : LOW);
            }
        }
    }
};

/*-------------------------------------------
              全局BLE对象声明
-------------------------------------------*/
// 声明一个指向 BLE 服务器的指针，用于操作 BLE 服务器
BLEServer *pServer;
// 声明一个指向 BLE 服务的指针，用于操作 BLE 服务
BLEService *pService;
// 声明一个指向 BLE 特征的指针，用于操作 BLE 特征
BLECharacteristic *pCharacteristic;

/*-------------------------------------------
                 初始化配置
-------------------------------------------*/
// setup 函数在程序启动时只执行一次，用于初始化硬件和 BLE 相关设置
void setup() {
    // 初始化串口通信，设置波特率为 115200，用于调试信息的输出
    Serial.begin(115200);
    
    // 硬件初始化
    // 将 LED 引脚设置为输出模式，以便可以控制其电平高低
    pinMode(ledPin, OUTPUT);
    // 初始时将 LED 引脚设置为低电平，熄灭 LED
    digitalWrite(ledPin, LOW);
    
    // 舵机初始化
    // 设置舵机的工作频率为 50Hz，这是标准的舵机控制频率
    myServo.setPeriodHertz(50);          
    // 将舵机连接到指定的引脚，并设置脉宽范围为 500 - 2500 微秒，这是常见的舵机脉宽范围
    myServo.attach(servoPin, 500, 2500); 
    // 将舵机的初始位置设置为 90 度
    myServo.write(90);                   

    // BLE 初始化
    // 初始化 BLE 设备，并设置设备名称为 "ESP32-Servo"，方便客户端搜索
    BLEDevice::init("ESP32-Servo");
    // 创建一个 BLE 服务器对象
    pServer = BLEDevice::createServer();
    // 为 BLE 服务器设置自定义的回调类，用于处理连接和断开事件
    pServer->setCallbacks(new MyServerCallbacks());

    // 在 BLE 服务器上创建一个服务，使用之前定义的服务 UUID
    pService = pServer->createService(SERVICE_UUID);
    // 在服务中创建一个特征，使用之前定义的特征 UUID，并设置该特征支持写入操作
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    // 为特征设置自定义的回调类，用于处理特征的写入事件
    pCharacteristic->setCallbacks(new MyCharCallbacks());
    
    // 启动 BLE 服务，使服务可以被客户端访问
    pService->start();
    // 获取 BLE 广播对象，用于设置广播参数
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    // 在广播中添加服务的 UUID，让客户端可以识别该服务
    pAdvertising->addServiceUUID(SERVICE_UUID);
    // 启动 BLE 广播，使设备可以被其他 BLE 设备发现
    BLEDevice::startAdvertising();
    // 通过串口输出提示信息，表明 BLE 已启动
    Serial.println("BLE已启动");
}

// loop 函数会不断循环执行，这里只是简单地延迟 1 秒，可根据需要添加其他任务
void loop() {
    delay(1000);
}
