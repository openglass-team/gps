#include "gps_handler.h"
#include "onenet_handler.h"
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
  
  onenet_init();
  
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
    Serial.write(c);
    gps.encode(c);
    count++;
  }

  unsigned long sinceLast = now - lastGpsUpdate;
  if (sinceLast < 1000) {
    return;
  }

  Serial.print("[GPS] satellites=");
  Serial.print(gps.satellites.value());
  Serial.print(" valid=");
  Serial.print(gps.location.isValid());
  Serial.print(" connected=");
  Serial.println(connected);

  if (gps.location.isValid()) {
    Serial.print("[GPS] Lat=");
    Serial.print(gps.location.lat(), 6);
    Serial.print(" Lng=");
    Serial.print(gps.location.lng(), 6);
    Serial.print("m HDOP=");
    Serial.println(gps.hdop.hdop());
  }

  if (!connected) {
    lastGpsUpdate = now;
    return;
  }

  uint8_t gpsBuffer[16];
  gpsBuffer[0] = gps_frame_count & 0xFF;
  gpsBuffer[1] = (gps_frame_count >> 8) & 0xFF;

  if (gps.location.isValid()) {
    gpsBuffer[2] = 1;
    gpsBuffer[3] = gps.satellites.value();

    double lat = gps.location.lat();
    double lng = gps.location.lng();
    double spd = gps.speed.mps();

    Serial.print("[GPS] SEND #");
    Serial.print(gps_frame_count);
    Serial.print(" lat=");
    Serial.print(lat, 6);
    Serial.print(" lng=");
    Serial.print(lng, 6);
    Serial.print("m spd=");
    Serial.print(spd, 1);
    Serial.println("m/s");

    memcpy(&gpsBuffer[4], &lat, 4);
    memcpy(&gpsBuffer[8], &lng, 4);
    memcpy(&gpsBuffer[12], &spd, 4);
    
    onenet_upload_gps(lat, lng, spd);
  } else {
    gpsBuffer[2] = 0;
    memset(&gpsBuffer[3], 0, 13);
    Serial.println("[GPS] SEND no fix");
  }

  gpsDataCharacteristic->setValue(gpsBuffer, sizeof(gpsBuffer));
  gpsDataCharacteristic->notify();
  gps_frame_count++;
  lastGpsUpdate = now;
}
