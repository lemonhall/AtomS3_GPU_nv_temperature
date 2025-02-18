/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5AtomS3 sample source code
*                          配套  M5AtomS3 示例源代码
* Visit for more information: https://docs.m5stack.com/en/core/AtomS3
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/core/AtomS3
*
* Describe: MQTT.
* Date: 2022/12/19
*******************************************************************************
*/
#include "M5AtomS3.h"
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

// Configure the name and password of the connected wifi and your MQTT Serve
// host.  配置所连接wifi的名称、密码以及你MQTT服务器域名
const char* ssid        = "xxxxxxxxxxxxxxxxxxx";
const char* password    = "xxxxxxxxxxxxxxxxxxx";
const char* mqtt_server = "192.168.50.232";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// 新增：记录上次重启的时间
unsigned long lastRebootTime = 0;

// 记录上次收到 MQTT 消息的时间
unsigned long lastMQTTMsgTime = 0;

void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();

void setup() {
    AtomS3.begin();
    setupWifi();
    client.setServer(mqtt_server,1883);  // Sets the server details.  配置所连接的服务器
    client.setCallback(callback);  // Sets the message callback function.  设置消息回调函数

    // 初始化上次重启时间
    lastRebootTime = millis();
}

void loop() {
    if (!client.connected()) {
        reConnect();
    }
    client.loop();  // This function is called periodically to allow clients to
                    // process incoming messages and maintain connections to the
                    // server.
    // 定期调用此函数，以允许主机处理传入消息并保持与服务器的连接

    unsigned long now = millis();  // Obtain the host startup duration.  获取主机开机时长
    if (now - lastMsg > 2000) {
        lastMsg = now;
        ++value;
    }

    // 如果 1 分钟内没有收到 MQTT 消息，则休眠
    if (now - lastMQTTMsgTime > 60000) {
        // 在此处添加休眠的相关代码
        Serial.println("Going to sleep due to no MQTT messages for 1 minutes.");
        // 在此处调用相关的关机函数或执行关机操作
        M5.Display.sleep();
    }

    // 新增：每 1 小时（3600000 毫秒）重启设备
    if (now - lastRebootTime > 3600000 ) {
        ESP.restart();
    }
}

void setupWifi() {
    delay(10);
    AtomS3.Lcd.print("Connecting to Network...");
    Serial.printf("Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);  // Set the mode to WiFi station mode.  设置模式为WIFI站模式
    WiFi.begin(ssid, password);  // Start Wifi connection.  开始wifi连接

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nSuccess\n");
    AtomS3.Lcd.println("Success");
    AtomS3.Lcd.println("For communication information see serial port");
    AtomS3.Lcd.fillScreen(BLACK);  // Set BLACK to the background color.  将黑色设置为底色
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    M5.Display.wakeup();
    AtomS3.Lcd.setRotation(1);
    AtomS3.Lcd.fillScreen(BLACK);  // Set BLACK to the background color.  将黑色设置为底色
    AtomS3.Lcd.setCursor(8, 30);
    // 将 payload 转换为整数
    int payloadValue = atoi((const char*)payload);

    // 根据 payload 值设置字体颜色
    if (payloadValue < 50) {
        AtomS3.Lcd.setTextColor(GREEN);  // 设置为绿色
    } else if (payloadValue >= 50 && payloadValue < 70) {
        AtomS3.Lcd.setTextColor(YELLOW);  // 设置为黄色
    } else {
        AtomS3.Lcd.setTextColor(RED);  // 设置为绿色
    }
    AtomS3.Lcd.setTextSize(10);  // Set the font size to 2.  设置字体大小为2
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        AtomS3.Lcd.print((char)payload[i]);
    }
    Serial.println();

    // 收到回调时更新时间
    lastMQTTMsgTime = millis();
    
    delay(200);
}

void reConnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID.  创建一个随机的客户端ID
        String clientId = "M5Stack-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect.  尝试重新连接
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            // ... and resubscribe.  重新订阅话题
            client.subscribe("lemon_gpu_temp");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println("try again in 5 seconds");
            delay(5000);
        }
    }
}
