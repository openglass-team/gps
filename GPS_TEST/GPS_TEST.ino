#include <TinyGPS++.h>
#include <Arduino.h>

TinyGPSPlus gps;
uint16_t gps_frame_count = 0;
unsigned long lastGpsUpdate = 0;
unsigned long lastSentencesWithFix = 0;

uint8_t computeChecksum(const char* sentence) {
    uint8_t cs = 0;
    for (int i = 1; sentence[i] != '\0' && sentence[i] != '*'; i++) {
        cs ^= (uint8_t)sentence[i];
    }
    return cs;
}

// 用正确校验和重建句子
char nmea_sentences[3][2][100];

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== GPS 模拟 v13 - 自动校验和 ===");
    
    // 构建所有句子并计算正确校验和
    const char* raw[][2] = {
        {
            "GPRMC,123519,A,3114.3212,N,12129.3856,E,2.5,180.0,270726,0.0,W,A",
            "GPGGA,123519,3114.3212,N,12129.3856,E,1,12,0.8,15.2,M,0.0,M,,"
        },
        {
            "GPRMC,123520,A,3954.3167,N,11623.4500,E,3.2,090.0,270726,0.0,W,A",
            "GPGGA,123520,3954.3167,N,11623.4500,E,1,10,0.9,50.0,M,0.0,M,,"
        },
        {
            "GPRMC,123521,V,,,,,,,,,,N",
            "GPGGA,123521,,,,,0,0,99.99,,,,,,"
        }
    };
    
    for (int scene = 0; scene < 3; scene++) {
        for (int s = 0; s < 2; s++) {
            // 构建完整句子（带 $ 但先不带校验和）
            char temp[100];
            temp[0] = '$';
            strcpy(temp + 1, raw[scene][s]);
            
            uint8_t cs = computeChecksum(temp);
            
            sprintf(nmea_sentences[scene][s], "$%s*%02X", raw[scene][s], cs);
            
            Serial.print("场景");
            Serial.print(scene);
            Serial.print(" 句子");
            Serial.print(s);
            Serial.print(": ");
            Serial.println(nmea_sentences[scene][s]);
        }
    }
    
    lastSentencesWithFix = gps.sentencesWithFix();
}

void loop() {
    static int scene = 0;
    static unsigned long lastSceneSwitch = 0;

    if (millis() - lastSceneSwitch > 5000) {
        lastSceneSwitch = millis();
        
        Serial.println();
        Serial.print(">>> 场景");
        Serial.print(scene);
        Serial.println(" <<<");

        for (int s = 0; s < 2; s++) {
            const char* sentence = nmea_sentences[scene][s];
            Serial.print("  注入");
            Serial.print(s);
            Serial.print(": ");
            Serial.println(sentence);
            
            for (int i = 0; sentence[i] != '\0'; i++) {
                gps.encode(sentence[i]);
            }
            gps.encode('\r');
            gps.encode('\n');
        }
        
        unsigned long newFixCount = gps.sentencesWithFix();
        bool gotFix = (newFixCount > lastSentencesWithFix);
        lastSentencesWithFix = newFixCount;
        
        Serial.print("  -> 新增有效句子: ");
        Serial.println(gotFix ? "YES ✓" : "NO ✗");
        if (gps.location.isValid()) {
            Serial.print("  -> 纬度: ");
            Serial.println(gps.location.lat(), 6);
            Serial.print("  -> 经度: ");
            Serial.println(gps.location.lng(), 6);
            Serial.print("  -> 卫星: ");
            Serial.println(gps.satellites.value());
        }
        
        scene = (scene + 1) % 3;
    }

    unsigned long now = millis();
    if (now - lastGpsUpdate >= 1000) {
        uint8_t buf[22];
        buf[0] = gps_frame_count & 0xFF;
        buf[1] = (gps_frame_count >> 8) & 0xFF;

        bool hasFix = gps.location.isValid() && (gps.satellites.value() > 0);
        if (hasFix) {
            buf[2] = 1;
            buf[3] = gps.satellites.value();
            float lat = gps.location.lat();
            float lng = gps.location.lng();
            float alt = gps.altitude.meters();
            float spd = gps.speed.mps();
            uint16_t course = (uint16_t)(gps.course.deg() * 100);
            memcpy(&buf[4], &lat, 4);
            memcpy(&buf[8], &lng, 4);
            memcpy(&buf[12], &alt, 4);
            memcpy(&buf[16], &spd, 4);
            buf[20] = course & 0xFF;
            buf[21] = (course >> 8) & 0xFF;
        } else {
            buf[2] = 0;
            memset(&buf[3], 0, 19);
        }

        Serial.print("帧");
        Serial.print(gps_frame_count);
        Serial.print(" 有效:");
        Serial.print(buf[2]);
        Serial.print(" 卫星:");
        Serial.println(buf[3]);
        
        gps_frame_count++;
        lastGpsUpdate = now;
    }
}