#include "gps_handler.h"
#include <TinyGPS++.h>
#include <BLEDevice.h>

#define GPS_RX 7
#define GPS_TX 8

extern BLECharacteristic *gpsDataCharacteristic;

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(1);
static uint16_t gps_frame_count = 0;
static unsigned long lastGpsUpdate = 0;

void gps_init() {
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
}

void gps_send_if_due(unsigned long now, bool connected) {
  // 限制每轮最多读 50 字节，防止阻塞 loop 导致看门狗复位
  int count = 0;
  while (gpsSerial.available() && count < 50) {
    gps.encode(gpsSerial.read());
    count++;
  }

  if (now - lastGpsUpdate < 1000 || !connected) {
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

    memcpy(&gpsBuffer[4], &lat, 4);
    memcpy(&gpsBuffer[8], &lng, 4);
    memcpy(&gpsBuffer[12], &alt, 4);
    memcpy(&gpsBuffer[16], &spd, 4);
    gpsBuffer[20] = course & 0xFF;
    gpsBuffer[21] = (course >> 8) & 0xFF;
  } else {
    gpsBuffer[2] = 0;
    memset(&gpsBuffer[3], 0, 19);
  }

  gpsDataCharacteristic->setValue(gpsBuffer, sizeof(gpsBuffer));
  gpsDataCharacteristic->notify();
  gps_frame_count++;
  lastGpsUpdate = now;
}
