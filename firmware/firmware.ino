#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <WiFi.h>
#include "gps_handler.h"
#include "onenet_handler.h"

//
// BLE UUIDs
//

#define DEVICE_INFORMATION_SERVICE_UUID (uint16_t)0x180A
#define MANUFACTURER_NAME_STRING_CHAR_UUID (uint16_t)0x2A29
#define MODEL_NUMBER_STRING_CHAR_UUID (uint16_t)0x2A24
#define FIRMWARE_REVISION_STRING_CHAR_UUID (uint16_t)0x2A26
#define HARDWARE_REVISION_STRING_CHAR_UUID (uint16_t)0x2A27

#define BATTERY_SERVICE_UUID (uint16_t)0x180F
#define BATTERY_LEVEL_CHAR_UUID (uint16_t)0x2A19

static BLEUUID serviceUUID("19B10000-E8F2-537E-4F6C-D104768A1214");
static BLEUUID gpsDataUUID("19B10003-E8F2-537E-4F6C-D104768A1214");

BLECharacteristic *gpsDataCharacteristic;
BLECharacteristic *batteryLevelCharacteristic;

//
// State
//

bool connected = false;

uint8_t batteryLevel = 100;
unsigned long lastBatteryUpdate = 0;

//
// BLE Callbacks
//

class ServerHandler : public BLEServerCallbacks {
  void onConnect(BLEServer *server) {
    connected = true;
    Serial.println("BLE Connected");
  }

  void onDisconnect(BLEServer *server) {
    connected = false;
    Serial.println("BLE Disconnected");
    BLEDevice::startAdvertising();
  }
};

void configure_ble() {
  BLEDevice::init("OpenGlass");
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerHandler());

  BLEService *service = server->createService(serviceUUID);

  gpsDataCharacteristic = service->createCharacteristic(
    gpsDataUUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLE2902 *ccc = new BLE2902();
  ccc->setNotifications(true);
  gpsDataCharacteristic->addDescriptor(ccc);

  BLEService *deviceInfoService = server->createService(DEVICE_INFORMATION_SERVICE_UUID);
  BLECharacteristic *manufacturerNameCharacteristic = deviceInfoService->createCharacteristic(
    MANUFACTURER_NAME_STRING_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *modelNumberCharacteristic = deviceInfoService->createCharacteristic(
    MODEL_NUMBER_STRING_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *firmwareRevisionCharacteristic = deviceInfoService->createCharacteristic(
    FIRMWARE_REVISION_STRING_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *hardwareRevisionCharacteristic = deviceInfoService->createCharacteristic(
    HARDWARE_REVISION_STRING_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ);

  manufacturerNameCharacteristic->setValue("Based Hardware");
  modelNumberCharacteristic->setValue("OpenGlass");
  firmwareRevisionCharacteristic->setValue("1.0.1");
  hardwareRevisionCharacteristic->setValue("Seeed Xiao ESP32S3 Sense");

  BLEService *batteryService = server->createService(BATTERY_SERVICE_UUID);
  batteryLevelCharacteristic = batteryService->createCharacteristic(
    BATTERY_LEVEL_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  ccc = new BLE2902();
  ccc->setNotifications(true);
  batteryLevelCharacteristic->addDescriptor(ccc);
  batteryLevelCharacteristic->setValue(&batteryLevel, 1);

  service->start();
  deviceInfoService->start();
  batteryService->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(service->getUUID());
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMaxPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE initialized");
}

void updateBatteryLevel() {
  batteryLevelCharacteristic->setValue(&batteryLevel, 1);
  batteryLevelCharacteristic->notify();
}

//
// Main
//

void setup() {
  Serial.begin(115200);
  Serial.println("OpenGlass GPS Starting...");

  configure_ble();
  gps_init();

  Serial.println("Ready");
}

void loop() {
  unsigned long now = millis();

  gps_send_if_due(now, connected);
  
  // 处理MQTT消息
  onenet_loop();

  if (now - lastBatteryUpdate > 60000) {
    updateBatteryLevel();
    lastBatteryUpdate = now;
  }

  delay(20);
}