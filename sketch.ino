#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MPU6050
Adafruit_MPU6050 mpu;

// Pin Definitions
#define NTC_PIN 35
#define POT_PIN 34
#define BUZZER_PIN 25

// Constants
const float BETA = 3950;
const float FALL_THRESHOLD = 25.0;

void setup() {

  Serial.begin(115200);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println(F("SSD1306 allocation failed"));

    for (;;) {
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 10);
  display.println("--- SMART HELMET ---");
  display.println("Initializing...");

  display.display();

  delay(1000);

  // MPU6050
  if (!mpu.begin()) {

    Serial.println("Failed to find MPU6050 chip");

    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("MPU6050 Error!");
    display.display();

    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
}


void loop() {

  // =====================================================
  // 1. READ TEMPERATURE
  // =====================================================

  int analogValueNTC = analogRead(NTC_PIN);

  float celsius =
      1.0 /
          (log(1.0 / (4095.0 / analogValueNTC - 1.0)) / BETA
           + 1.0 / 298.15)
      - 273.15;


  // =====================================================
  // 2. READ TEMPERATURE THRESHOLD
  // =====================================================

  int analogValuePOT = analogRead(POT_PIN);

  float tempThreshold =
      map(analogValuePOT, 0, 4095, 20, 60);


  // =====================================================
  // 3. READ MPU6050
  // =====================================================

  sensors_event_t a, g, temp;

  mpu.getEvent(&a, &g, &temp);


  // Calculate acceleration magnitude

  float accelMagnitude =
      sqrt(
          a.acceleration.x * a.acceleration.x +
          a.acceleration.y * a.acceleration.y +
          a.acceleration.z * a.acceleration.z
      );


  // =====================================================
  // 4. ALARM CONDITIONS
  // =====================================================

  bool tempAlert =
      (celsius > tempThreshold);

  bool fallDetected =
      (accelMagnitude > FALL_THRESHOLD);


  // =====================================================
  // 5. BUZZER
  // =====================================================

  if (tempAlert || fallDetected) {

    // Passive buzzer needs a frequency
    tone(BUZZER_PIN, 1000);

  } else {

    noTone(BUZZER_PIN);
  }


  // =====================================================
  // 6. OLED DISPLAY
  // =====================================================

  display.clearDisplay();

  display.setCursor(0, 0);

  display.println("--- SMART HELMET ---");

  display.print("Temp: ");
  display.print(celsius, 1);
  display.println(" C");

  display.print("Thresh: ");
  display.print(tempThreshold, 1);
  display.println(" C");

  display.print("Impact: ");
  display.print(accelMagnitude, 1);
  display.println(" m/s2");

  display.print("STATUS: ");

  if (fallDetected) {

    display.println("FALL DETECTED!");

  }
  else if (tempAlert) {

    display.println("OVERHEAT!");

  }
  else {

    display.println("NORMAL");
  }

  display.display();


  // =====================================================
  // 7. SERIAL MONITOR
  // =====================================================

  Serial.print("Temp: ");
  Serial.print(celsius);

  Serial.print(" C | Thresh: ");
  Serial.print(tempThreshold);

  Serial.print(" C | Accel: ");
  Serial.println(accelMagnitude);


  delay(200);
}