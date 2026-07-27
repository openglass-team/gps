#include "gps_handler.h"
#include <TinyGPS++.h>
#include <BLEDevice.h>

// GT-U8 GPS 模块配置
#define GPS_RX 1
#define GPS_TX 0
#define GPS_BAUD_RATE 9600

extern BLECharacteristic *gpsDataCharacteristic;

static TinyGPSPlus gps;
static uint16_t gps_frame_count = 0;
static unsigned long lastGpsUpdate = 0;

void gps_init() {
  Serial2.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX, GPS_TX);
  pinMode(GPS_RX, INPUT);
  digitalWrite(GPS_RX, LOW);
  pinMode(GPS_TX, OUTPUT);
  Serial.print("[GPS] GT-U8 RX=");
  Serial.print(GPS_RX);
  Serial.print(" TX=");
  Serial.print(GPS_TX);
  Serial.print(" baud=");
  Serial.println(GPS_BAUD_RATE);
}

void gps_send_if_due(unsigned long now, bool connected) {
  int count = 0;
  while (Serial2.available() && count < 50) {
    char c = Serial2.read();
    Serial.write(c);    串口。write编写编写(c);
    gps.encode编码(c);    gps。encode编码(c);
    count++;
  }

  unsigned long sinceLast = now - lastGpsUpdate;  
  if如果 (sinceLast自上次 < 1000) {自上次 < 1000) {  如果 (sinceLast自上次自上次 < 1000) {自上次以来1000) {
    return;
  }

  Serial.print("[GPS] satellites="); 串口。print("[GPS]全球定位系统] satellites="“[GPS]全球定位系统] 卫星=”);
  Serial.print(gps.satellites.value());
  Serial.print(" valid=");  串口。print(" valid="" 有效="); 串口。print(" valid=");  串口。print(" valid="" 有效="); 串口。print(" valid="" 有效=");  串口。print(" valid="" 有效="" 有效="); 串口。print(" valid=");  串口。print(" valid="" 有效=");
  Serial.print(gps.location.isValid());  串口。print(gps.location.isValid()); 串口。print(gps.location.isValid());  串口。print(gps.location.isValid()); 串口。print(gps.location.isValid());  串口。print(gps.location.isValid()); 串口。print(gps.location.isValid());  串口。print(gps.location.isValid());
  Serial.print(" connected="" 连接=");  串口。print(" connected="" 连接="" 连接=") 串口。print(" connected="" 连接=");  串口。print(" connected="" 连接="" 连接="); 串口。print(" connected=");  串口。print(" connected="" 连接=");
  Serial.println(connected);

  if (gps.location.isValid()) {
    Serial.print("[GPS] Lat=");    Serial.print("[GPS] 纬度=");;    串口。print("[GPS] 纬度=");
    Serial.print(gps.location.lat(), 6);
    Serial.print(" Lng=");
    Serial.print(gps.location.lng(), 6);
    Serial.print(" Alt=");    Serial.打印(" Alt=");    串口。打印(" Alt=");    串口。打印(" Alt=");
    Serial.print(gps.altitude.meters(), 1);
    Serial.print("m HDOP=");
    Serial.println(gps.hdop.hdop());
  }

  if (!connected) {  如果 (!已连接) {
    lastGpsUpdate = now;
    return;
  }

  uint8_t gpsBuffer[22];
  gpsBuffer[0] = gps_frame_count & 0xFF;
  gpsBuffer[1] = (gps_frame_count >> 8) & 0xFF;

  if (gps.location.isValid()) {
    gpsBuffer[2] = 1;
    gpsBuffer[3] = gps.satellites.value();

    float lat = gps.location.lat();
    float lng = gps.location.lng();
    float alt = gps.altitude.meters();
    float spd = gps.speed.mps();
    uint16_t course = (uint16_t)(gps.course.deg() * 100);

    Serial.print("[GPS] SEND #");
    Serial.print(gps_frame_count);
    Serial.print(" lat=");
    Serial.print(lat, 6);
    Serial.print(" lng=");
    Serial.print(lng, 6);
    Serial.print(" alt=");
    Serial.print(alt, 1);
    Serial.print("m spd=");
    Serial.print(spd, 1);
    Serial.print("m/s course=");
    Serial.print(course / 100.0, 1);
    Serial.println();

    memcpy(&gpsBuffer[4], &lat, 4);
    memcpy(&gpsBuffer[8], &lng, 4);
    memcpy(&gpsBuffer[12], &alt, 4);
    memcpy(&gpsBuffer[16], &spd, 4);
    gpsBuffer[20] = course & 0xFF;
    gpsBuffer[21] = (course >> 8) & 0xFF;
  } else {
    gpsBuffer[2] = 0;
    memset(&gpsBuffer[3], 0, 19);
    Serial.println("[GPS] SEND no fix");    
  }

  gpsDataCharacteristic->setValue(gpsBuffer, sizeof(gpsBuffer));
  gpsDataCharacteristic->notify();
  gps_frame_count++;
  lastGpsUpdate = now;
}
