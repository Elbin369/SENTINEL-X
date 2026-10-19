# 🪖 Smart Helmet -- Temperature Detection & Fall Alarm System

A smart helmet safety system built and tested in **Wokwi** using an
**ESP32**, an **NTC temperature sensor**, **MPU6050
accelerometer/gyroscope**, **SSD1306 OLED display**, potentiometer, and
buzzer.

The system continuously monitors helmet temperature and sudden
acceleration. If the temperature exceeds the user-defined threshold or a
fall/impact is detected, the buzzer produces an audible alarm.

View : https://wokwi.com/projects/476942596005096449

------------------------------------------------------------------------

## 📌 Project Overview

The goal of this project is to create a basic embedded safety system for
a smart helmet.

The system provides two main safety functions:

1.  🌡️ **Temperature Detection**
    -   Measures temperature using an NTC thermistor.
    -   Allows the user to set a temperature threshold using a
        potentiometer.
    -   Activates the buzzer when the measured temperature exceeds the
        threshold.
2.  🪖 **Fall / Impact Detection**
    -   Uses an MPU6050 motion sensor.
    -   Calculates the total acceleration magnitude.
    -   Detects a possible fall or strong impact when acceleration
        exceeds the configured limit.
    -   Activates the buzzer when a fall is detected.

The current temperature, threshold, acceleration, and system status are
displayed on the OLED.

------------------------------------------------------------------------

## ✨ Features

-   🌡️ Real-time temperature monitoring
-   🎚️ Adjustable temperature threshold
-   🪖 Fall/impact detection using MPU6050
-   🔊 Audible alarm using a passive buzzer
-   📟 Real-time OLED status display
-   🖥️ Serial Monitor output for debugging
-   ⚡ ESP32-based implementation
-   🧪 Fully testable in Wokwi
-   🔌 No external cloud service required

------------------------------------------------------------------------

## 🧰 Hardware Components

  Component                      Quantity Purpose
  ------------------------- ------------- ----------------------------------
  ESP32 Development Board               1 Main microcontroller
  NTC Temperature Sensor                1 Temperature measurement
  Potentiometer                         1 Temperature threshold adjustment
  MPU6050                               1 Fall/impact detection
  0.96-inch SSD1306 OLED                1 Display
  Passive Buzzer                        1 Safety alarm
  Jumper Wires                As required Connections
  Breadboard                            1 Prototyping

> The project is currently implemented as a Wokwi simulation, so the
> exact physical component model can be changed when building the real
> prototype.

------------------------------------------------------------------------

## 🔌 Pin Configuration

  ESP32 GPIO   Component           Function
  ------------ ------------------- --------------------------
  GPIO 35      NTC sensor          Temperature analog input
  GPIO 34      Potentiometer       Threshold analog input
  GPIO 25      Buzzer              Alarm output
  I²C SDA      SSD1306 / MPU6050   I²C data
  I²C SCL      SSD1306 / MPU6050   I²C clock

The OLED and MPU6050 share the ESP32's I²C bus.

------------------------------------------------------------------------

## ⚙️ How It Works

### 1. Temperature Measurement

The ESP32 reads the analog value from the NTC sensor using:

``` cpp
analogRead(NTC_PIN);
```

The ADC value is converted into temperature using the thermistor's Beta
coefficient.

The project uses:

``` cpp
const float BETA = 3950;
```

The calculated temperature is displayed in Celsius.

------------------------------------------------------------------------

### 2. Temperature Threshold

The potentiometer is connected to GPIO 34.

Its ADC value is mapped to a threshold range of:

``` text
20°C – 60°C
```

For example:

``` text
Temperature = 47.6°C
Threshold   = 35.0°C
```

Since:

``` text
47.6°C > 35.0°C
```

the temperature alarm is activated.

------------------------------------------------------------------------

### 3. Fall / Impact Detection

The MPU6050 provides acceleration values along three axes:

``` text
X
Y
Z
```

The program calculates total acceleration magnitude:

``` text
A = √(Ax² + Ay² + Az²)
```

The current fall threshold is:

``` cpp
const float FALL_THRESHOLD = 25.0;
```

If:

``` text
Acceleration > 25 m/s²
```

the system considers it a possible fall/impact.

------------------------------------------------------------------------

### 4. Alarm Logic

The alarm is activated when either condition is true:

``` cpp
bool tempAlert = (celsius > tempThreshold);
bool fallDetected = (accelMagnitude > FALL_THRESHOLD);
```

The final logic is:

``` cpp
if (tempAlert || fallDetected) {
    tone(BUZZER_PIN, 1000);
} else {
    noTone(BUZZER_PIN);
}
```

### Why `tone()` is used

The project uses a passive piezo buzzer.

A passive buzzer requires an oscillating signal at an audible frequency.
Therefore:

``` cpp
tone(BUZZER_PIN, 1000);
```

generates a 1 kHz tone.

Simply using:

``` cpp
digitalWrite(BUZZER_PIN, HIGH);
```

does not continuously generate an audible frequency for a passive
buzzer.

------------------------------------------------------------------------

## 📟 OLED Display

The SSD1306 OLED displays:

``` text
--- SMART HELMET ---

Temp: XX.X C
Thresh: XX.X C
Impact: XX.X m/s2
STATUS: NORMAL
```

Possible status messages are:

``` text
NORMAL
```

``` text
OVERHEAT!
```

``` text
FALL DETECTED!
```

If both conditions occur simultaneously, fall detection has priority in
the displayed status.

------------------------------------------------------------------------

## 🖥️ Serial Monitor

The ESP32 also prints sensor information to the Serial Monitor at:

``` text
115200 baud
```

Example:

``` text
Temp: 47.6 C | Thresh: 35.0 C | Accel: 9.8
```

This is useful for debugging and testing sensor values.

------------------------------------------------------------------------

## 🧪 Example Test

### Normal Condition

``` text
Temperature: 30°C
Threshold:   35°C
Acceleration: 9.8 m/s²

STATUS: NORMAL
BUZZER: OFF
```

### High Temperature

``` text
Temperature: 47.6°C
Threshold:   35°C

STATUS: OVERHEAT!
BUZZER: ON
```

### Fall / Impact

``` text
Acceleration: > 25 m/s²

STATUS: FALL DETECTED!
BUZZER: ON
```

------------------------------------------------------------------------

## 🧱 Software & Libraries

The project is written in **Arduino C++** for ESP32.

Required libraries:

-   `Wire.h`
-   `Adafruit GFX Library`
-   `Adafruit SSD1306`
-   `Adafruit MPU6050`
-   `Adafruit Unified Sensor`

The project can be simulated using **Wokwi**.

------------------------------------------------------------------------

## 📁 Suggested Project Structure

``` text
smart-helmet/
│
├── README.md
├── sketch.ino
├── diagram.json
└── libraries.txt
```

Depending on the Wokwi project setup, the main source file may use a
different filename.

------------------------------------------------------------------------

## 🚀 Running the Project in Wokwi

1.  Open the Wokwi ESP32 project.
2.  Make sure all components are connected correctly.
3.  Install/include the required Adafruit libraries.
4.  Upload or run the Arduino C++ code.
5.  Open the Serial Monitor.
6.  Adjust the potentiometer to change the temperature threshold.
7.  Change the NTC temperature in the simulator.
8.  Move or simulate a strong acceleration using the MPU6050.
9.  Observe the OLED and buzzer response.

------------------------------------------------------------------------

## 🔧 Important Configuration

### Temperature Threshold

The current threshold range is:

``` cpp
map(analogValuePOT, 0, 4095, 20, 60);
```

You can change the range if required.

For example:

``` cpp
map(analogValuePOT, 0, 4095, 30, 50);
```

would provide a 30°C--50°C adjustment range.

### Fall Threshold

The current value is:

``` cpp
const float FALL_THRESHOLD = 25.0;
```

This value should be experimentally calibrated for a real helmet because
simulation values and real-world motion can differ.

------------------------------------------------------------------------

## ⚠️ Limitations

This is a prototype/simulation and should not be treated as a certified
safety device.

-   The NTC temperature calculation depends on the sensor
    characteristics and circuit configuration.
-   The fall detector uses acceleration magnitude and can produce false
    positives during sudden movements.
-   A real helmet would require proper sensor placement and calibration.
-   A real-world fall detection system should use additional logic such
    as impact duration, orientation change, and motion confirmation.
-   The simulated temperature and acceleration behavior may not exactly
    match physical hardware.
-   The system does not currently send an emergency notification to a
    phone or emergency contact.

------------------------------------------------------------------------

## 🔮 Future Improvements

Possible future versions can include:

-   📱 Bluetooth/mobile notification
-   🚨 Emergency SMS alert
-   📍 GPS location sharing after a fall
-   🔋 Battery monitoring
-   🔌 Charging circuit
-   🌡️ More accurate temperature sensing
-   🧠 Improved fall-detection algorithm
-   📊 Data logging
-   ☁️ IoT/cloud monitoring
-   🔊 Different buzzer patterns for temperature and fall events
-   💡 Warning LEDs
-   🪖 Helmet-wear detection
-   🚀 Low-power sleep mode

------------------------------------------------------------------------

## 🎯 Project Objective

The main objective is to demonstrate how an embedded system can combine
multiple sensors to improve helmet safety.

The system combines:

``` text
NTC Sensor
     │
     ▼
Temperature Detection ───┐
                         │
                         ▼
                    Alarm Logic
                         │
MPU6050                  │
     │                   │
     ▼                   │
Fall Detection ──────────┤
                         │
                         ▼
              ┌──────────────────┐
              │      ESP32        │
              └──────────────────┘
                    │       │
                    ▼       ▼
                  OLED    Buzzer
```

------------------------------------------------------------------------

## 👨‍💻 Project Status

**Status:** Working prototype / Wokwi simulation

The current implementation successfully demonstrates:

-   Temperature monitoring
-   Adjustable temperature threshold
-   Fall/impact detection
-   OLED status display
-   Serial monitoring
-   Audible alarm activation

------------------------------------------------------------------------

## 📜 License

This project is intended for educational, academic, and prototype
development purposes.

You are free to modify and improve the project for learning and
experimentation.
