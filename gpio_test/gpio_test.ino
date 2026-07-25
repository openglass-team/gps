#define CAMERA_MODEL_XIAO_ESP32S3

// XIAO ESP32S3 D0-D10 引脚对应的 GPIO 编号
// D0=GPIO44, D1=GPIO43, D2=GPIO2,  D3=GPIO3,  D4=GPIO4
// D5=GPIO5,  D6=GPIO6,  D7=GPIO7,  D8=GPIO8,  D9=GPIO9
// D10=GPIO10
const int gpioPins[] = { 44, 43, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

const int numPins = sizeof(gpioPins) / sizeof(gpioPins[0]);

// D-Pin 名称对照表
const char* pinNames[] = { "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10" };

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=================================");
  Serial.println("XIAO ESP32S3 D0-D10 Pin Test");
  Serial.println("=================================");
  Serial.print("Testing ");
  Serial.print(numPins);
  Serial.println(" pins (D0 ~ D10)...");
  Serial.println();

  // 将所有引脚初始化为输出模式
  for (int i = 0; i < numPins; i++) {
    pinMode(gpioPins[i], OUTPUT);
  }

  Serial.println("All pins initialized as OUTPUT.");
  Serial.println("Cycle: All HIGH -> All LOW -> One-by-One HIGH");
  Serial.println("Use multimeter to measure each pin voltage.");
  Serial.println("=================================");
  Serial.println();
}

void loop() {
  // 阶段 1: 全部设为 HIGH
  Serial.println(">>> Phase 1: ALL pins HIGH (~3.3V)");
  for (int i = 0; i < numPins; i++) {
    digitalWrite(gpioPins[i], HIGH);
  }
  delay(3000);

  // 阶段 2: 全部设为 LOW
  Serial.println(">>> Phase 2: ALL pins LOW (~0V)");
  for (int i = 0; i < numPins; i++) {
    digitalWrite(gpioPins[i], LOW);
  }
  delay(3000);

  // 阶段 3: 逐个引脚设为 HIGH
  Serial.println(">>> Phase 3: One-by-One pin HIGH");
  for (int i = 0; i < numPins; i++) {
    // 先把所有引脚设为 LOW
    for (int j = 0; j < numPins; j++) {
      digitalWrite(gpioPins[j], LOW);
    }
    // 单独设置当前引脚为 HIGH
    digitalWrite(gpioPins[i], HIGH);
    Serial.print("  ");
    Serial.print(pinNames[i]);
    Serial.print(" (GPIO");
    Serial.print(gpioPins[i]);
    Serial.println(") HIGH (~3.3V) -- measure NOW");
    delay(1500);
  }

  Serial.println();
  Serial.println("=== Test cycle complete. Repeating... ===");
  Serial.println();
  delay(1000);
}