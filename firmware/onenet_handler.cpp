#include "onenet_handler.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define WIFI_SSID      "wwto"
#define WIFI_PASSWORD  "123456888"

#define ONENET_SERVER   "mqtts.heclouds.com"
#define ONENET_PORT     1883
#define ONENET_CLIENT_ID "glass"
#define ONENET_USERNAME  "t9F16g4u08"
#define ONENET_PASSWORD  "version=2018-10-31&res=products%2Ft9F16g4u08%2Fdevices%2Fglass&et=1848384715&method=md5&sign=1gUavuCm3%2Bolse2sL5ZnVQ%3D%3D"

#define ONENET_DATA_STREAM "gps"

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);
static bool onenetConnected = false;

void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    
    Serial.print("Connecting WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long start = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connect failed");
    }
}

void connectOneNET() {
    if (mqttClient.connected()) return;
    
    Serial.print("Connecting OneNET...");
    
    if (mqttClient.connect(ONENET_CLIENT_ID, ONENET_USERNAME, ONENET_PASSWORD)) {
        Serial.println("connected");
        onenetConnected = true;
        
        String subTopic = String("$dp/" ONENET_CLIENT_ID "/cmd/#");
        mqttClient.subscribe(subTopic.c_str());
        
    } else {
        Serial.print("failed, rc=");
        Serial.println(mqttClient.state());
        onenetConnected = false;
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("\nMessage arrived [");
    Serial.print(topic);
    Serial.print("] ");
    
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void onenet_send_test_data() {
    if (!mqttClient.connected()) {
        Serial.println("OneNET not connected, skip test");
        return;
    }
    
    String payload = String("{") +
        "\"id\":\"999\"," +
        "\"version\":\"1.0\"," +
        "\"params\":{" +
            "\"Latitude\":{\"value\":39.904200}," +
            "\"Longitude\":{\"value\":116.407400}," +
            "\"Speed\":{\"value\":0.0}" +
        "}" +
    "}";
    
    const char* topic = "$sys/t9F16g4u08/glass/thing/property/post";
    
    boolean success = mqttClient.publish(topic, 0, false, payload.c_str());
    
    if (success) {
        Serial.println("\n[OneNET] Test data uploaded successfully!");
        Serial.println("[OneNET] Test Payload: " + payload);
    } else {
        Serial.println("[OneNET] Test upload failed");
    }
}

void onenet_init() {
    connectWiFi();
    
    mqttClient.setServer(ONENET_SERVER, ONENET_PORT);
    mqttClient.setCallback(mqttCallback);
    
    connectOneNET();
    
    delay(1000);
    
    onenet_send_test_data();
    
    Serial.println("OneNET handler initialized");
}

void onenet_upload_gps(double lat, double lng, double speed) {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected, skip upload");
            return;
        }
    }
    
    if (!mqttClient.connected()) {
        connectOneNET();
        if (!mqttClient.connected()) {
            Serial.println("OneNET not connected, skip upload");
            return;
        }
    }
    
    static int msgId = 123;
    String payload = String("{") +
        "\"id\":\"" + String(msgId) + "\"," +
        "\"version\":\"1.0\"," +
        "\"params\":{" +
            "\"Latitude\":{\"value\":" + String(lat, 6) + "}," +
            "\"Longitude\":{\"value\":" + String(lng, 6) + "}," +
            "\"Speed\":{\"value\":" + String(speed, 2) + "}" +
        "}" +
    "}";
    
    const char* topic = "$sys/t9F16g4u08/glass/thing/property/post";
    
    boolean success = mqttClient.publish(topic, 0, false, payload.c_str());
    
    if (success) {
        Serial.println("\n[OneNET] GPS data uploaded");
        Serial.println("[OneNET] Payload: " + payload);
        msgId++;
    } else {
        Serial.println("[OneNET] Upload failed");
    }
}

bool onenet_is_connected() {
    return onenetConnected && mqttClient.connected();
}