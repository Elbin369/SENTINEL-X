# Sentinel-X — Modular Cybersecurity Monitoring & Intrusion-Detection Simulator

> **Project type:** Educational embedded-security / IoT monitoring prototype  
> **Platform:** ESP32 in Wokwi  
> **Primary goal:** Demonstrate how sensor monitoring, authentication, a web honeypot, event logging, threat scoring, attack classification, and automated responses can work together in one modular system.

Sentinel-X is an educational cybersecurity project built around an ESP32. It combines physical-environment sensors with a keypad-based login flow and a small HTTP honeypot. Sensor events and suspicious web requests are treated as security events, recorded, assigned threat points, and used to update the system's status.

The project is designed for learning and demonstration. It is **not a production alarm system, certified intrusion detector, or replacement for a professionally engineered security product**. The ESP32/Wokwi environment and simulated network behavior also impose limits on what can be tested.

---

## Table of contents

- [1. Motivation](#1-motivation)
- [2. Project objectives](#2-project-objectives)
- [3. System overview](#3-system-overview)
- [4. Hardware and pin connections](#4-hardware-and-pin-connections)
- [5. Software and libraries](#5-software-and-libraries)
- [6. Modules 1–10](#6-modules-110)
- [7. Threat-scoring model](#7-threat-scoring-model)
- [8. Alert, buzzer, and recovery behavior](#8-alert-buzzer-and-recovery-behavior)
- [9. Web honeypot endpoints](#9-web-honeypot-endpoints)
- [10. Setup and run in Wokwi](#10-setup-and-run-in-wokwi)
- [11. Testing checklist](#11-testing-checklist)
- [12. Serial Monitor and debugging](#12-serial-monitor-and-debugging)
- [13. Security considerations and limitations](#13-security-considerations-and-limitations)
- [14. Future improvements](#14-future-improvements)
- [15. Repository structure](#15-repository-structure)
- [16. Learning outcomes](#16-learning-outcomes)
- [17. Credits](#17-credits)

---

## 1. Motivation

Many security systems monitor only one kind of signal. A basic alarm may react to motion, while a web application monitor may record suspicious HTTP requests. Sentinel-X explores how multiple kinds of events can be collected in one system and treated as possible signs of a security incident.

The project connects four ideas:

1. **Physical monitoring** — detect motion, nearby objects, unusual light, and unusual temperature or humidity.
2. **Access control** — require a PIN before granting an authenticated session.
3. **Network deception** — expose simulated, tempting web paths through a small honeypot so accesses can be recorded as suspicious activity.
4. **Security-event handling** — log events, calculate a threat score, classify activity, update status indicators, and trigger bounded alerts.

The main learning objective is to move from simply reading sensor values to building a small, event-driven security workflow.

## 2. Project objectives

- Read several sensors using an ESP32.
- Show system information on an SSD1306 OLED display.
- Provide keypad-based PIN entry and a session timeout.
- Detect repeated failed PIN attempts and enter a lockdown state.
- Run a small HTTP server with honeypot-style endpoints.
- Record sensor, authentication, and honeypot events.
- Calculate a cumulative threat score from event types.
- Classify threat levels and update the system status.
- Give short, understandable buzzer alerts instead of an endlessly repeated tone.
- Show the status dashboard during an event and return to the login screen when the display period ends.
- Test both detection and recovery paths, not only the initial alarm.

## 3. System overview

```text
                  ┌──────────────────────────┐
                  │        ESP32              │
                  │   Sentinel-X controller   │
                  └────────────┬─────────────┘
                               │
       ┌───────────────────────┼────────────────────────┐
       │                       │                        │
┌──────▼──────┐        ┌───────▼────────┐       ┌───────▼────────┐
│ Environment │        │ Access control │       │ HTTP honeypot  │
│ DHT22 / LDR │        │ 4×4 keypad/PIN │       │ /admin /login  │
│ PIR / HC-SR04│       │ session/lockout │       │ /config /debug │
└──────┬──────┘        └───────┬────────┘       │ /device        │
       │                       │                └───────┬────────┘
       └───────────────────────┼────────────────────────┘
                               ▼
                    ┌─────────────────────┐
                    │ Event logging       │
                    │ Threat score        │
                    │ Attack classification│
                    └──────────┬──────────┘
                               ▼
                    ┌─────────────────────┐
                    │ Automated response  │
                    │ OLED / RGB LED      │
                    │ Buzzer / lockdown   │
                    └─────────────────────┘
```

### High-level event flow

1. Read sensor values and check the keypad/network server.
2. Compare readings or actions against configured rules.
3. When a new event occurs, record it once rather than on every loop iteration.
4. Apply the configured score for score-bearing events.
5. Classify the current threat level and update the LED/status display.
6. Trigger a short alert pattern when appropriate.
7. When the cause clears, record recovery and allow the system to return to its normal state. A historical threat score may remain elevated until the project defines a score-decay or reset policy.

## 4. Hardware and pin connections

The following mapping is the project wiring used in the Wokwi design. Keep the code and diagram consistent if you change any pin.

| Component | ESP32 connection | Purpose |
|---|---:|---|
| SSD1306 OLED SDA | GPIO 21 | I²C data |
| SSD1306 OLED SCL | GPIO 22 | I²C clock |
| OLED I²C address | `0x3C` | OLED device address |
| RGB LED — red | GPIO 25 | Warning / critical indication |
| RGB LED — green | GPIO 26 | Normal / secure indication |
| RGB LED — blue | GPIO 27 | Authenticated / informational indication |
| Buzzer | GPIO 18 | Audible alerts and keypad feedback |
| DHT22 data | GPIO 4 | Temperature and humidity |
| PIR output | GPIO 5 | Motion detection |
| LDR analog output | GPIO 34 | Light-level measurement |
| HC-SR04 trigger | GPIO 12 | Ultrasonic trigger pulse |
| HC-SR04 echo | GPIO 35 | Ultrasonic echo input |
| 4×4 keypad rows | GPIO 13, 14, 16, 17 | Keypad row signals |
| 4×4 keypad columns | GPIO 19, 23, 32, 33 | Keypad column signals |

### Wiring notes

- The OLED uses I²C and is initialized on GPIO 21/22.
- The RGB LED in the current simulation is treated as **common cathode**. If using a physical LED, confirm its type and use suitable current-limiting resistors.
- GPIO 34 and GPIO 35 are input-only pins on the ESP32. They are appropriate for the LDR analog input and HC-SR04 echo input, respectively.
- For a physical HC-SR04, its echo output may be 5 V. Use an appropriate voltage divider or level shifter before connecting it to an ESP32 GPIO. Do not assume the Wokwi simulation removes this electrical requirement on real hardware.
- Check the sensor modules' voltage requirements before moving from simulation to a physical circuit.

## 5. Software and libraries

The project is written for the Arduino framework on ESP32. Typical dependencies used by this design include:

- `WiFi.h` — ESP32 Wi-Fi connectivity.
- `WebServer.h` — lightweight HTTP server and endpoint handlers.
- `Wire.h` — I²C communication.
- `Adafruit_GFX.h` and `Adafruit_SSD1306.h` — OLED rendering.
- `DHT.h` — DHT22 readings.
- `Keypad.h` — 4×4 keypad scanning.

Use the library versions supported by the ESP32 board package and the Wokwi project. If compilation reports a missing library, install the named dependency or add it to the Wokwi project configuration.

## 6. Modules 1–10

The project is organized into modules so each feature can be tested independently before being combined. The exact implementation details depend on the current sketch version; use this section as the architecture and behavior guide, then keep it aligned with the code as the project evolves.

### Module 1 — Core controller and system initialization

**Purpose:** Start the ESP32 and initialize the main hardware and software components.

**Responsibilities:**

- Configure sensor, LED, buzzer, keypad, and display pins.
- Start serial output for debugging.
- Initialize the OLED over I²C.
- Initialize the DHT22 and other sensors.
- Establish safe startup values for system status and authentication state.
- Show a startup message before normal operation begins.

**Expected result:** The ESP32 boots, the display initializes, and the project enters its monitoring loop without sensor or display initialization errors.

### Module 2 — PIN authentication and session handling

**Purpose:** Simulate a local access-control mechanism.

**Responsibilities:**

- Read keypad input and build a PIN entry.
- Mask the entered PIN on the display where supported.
- Compare the submitted PIN against the demonstration PIN configured in the sketch (the project has used `2580` as a demo PIN).
- Grant an authenticated session after a successful PIN entry.
- End the session after the configured inactivity/session period (currently designed around 60 seconds).
- Count failed attempts and raise a security event when the configured limit is reached.

**Expected result:** A valid PIN permits the demo session; invalid PINs are rejected and logged. The PIN is a demonstration credential, not a secure production credential store.

### Module 3 — Failed-attempt protection and lockdown

**Purpose:** Demonstrate a basic response to repeated failed authentication attempts.

**Responsibilities:**

- Track consecutive failed PIN attempts.
- Raise the threat score when appropriate.
- Enter lockdown after the configured number of failed attempts (the demo has used five attempts).
- Show a clear lockdown state on the display and use the warning LED/buzzer.
- Require the project's defined recovery procedure to leave lockdown.

**Expected result:** Repeated incorrect PIN attempts produce a visible security response instead of silently allowing unlimited retries.

**Important:** This is a teaching simulation. A robust real system would also need rate limiting, secure credential storage, tamper resistance, and a carefully designed recovery process.

### Module 4 — Environmental and physical sensor monitoring

**Purpose:** Detect readings that fall outside configured expected ranges.

**Sensors and example rules:**

- **DHT22:** temperature and humidity monitoring. The current project rules have included temperature above 40 °C and humidity above 70% as alert thresholds.
- **LDR:** monitor the analog light reading. The configured normal range has been approximately 50–3000 ADC units; values outside the configured range can be treated as abnormal.
- **HC-SR04:** detect a nearby object. The approach threshold is approximately 100 cm, with very small/invalid readings filtered (the project has used a lower bound around 5 cm).
- **PIR:** detect motion from its digital output.

Thresholds are configuration values, not universal safety limits. Adjust them for the sensor, room, and use case. DHT22 measurements are not instantaneous and should be treated with appropriate sampling intervals.

**Expected result:** Sensor values are sampled, compared with configured rules, and reported as events when a state transition occurs. The implementation should also detect when the reading returns to its normal range.

### Module 5 — HTTP honeypot simulation

**Purpose:** Create decoy web paths that can attract or record unexpected access attempts.

The ESP32 connects to the Wokwi guest Wi-Fi network and starts a small HTTP server. The project has registered these demonstration endpoints:

| Endpoint | Demonstration purpose |
|---|---|
| `/admin` | Simulated administrative page access |
| `/login` | Simulated login endpoint access |
| `/config` | Simulated configuration endpoint access |
| `/debug` | Simulated debug endpoint access |
| `/device` | Simulated device-information endpoint access |

Accesses to these paths can be logged and treated as honeypot activity. This is a small embedded decoy, not a full web application and not a production honeypot platform.

**Expected result:** The serial output reports the server startup and incoming endpoint requests; the event system records relevant access attempts and can increase the threat score.

### Module 6 — Threat scoring engine

**Purpose:** Convert different event types into a simple numeric risk indicator.

Example event weights used in the project plan:

| Event | Example score increase |
|---|---:|
| Failed PIN attempt | +5 |
| Repeated/suspicious login behavior | +20 |
| Honeypot endpoint access | +30 |
| Sensor anomaly | +20 |
| HTTP request flood | +30 |
| Physical intrusion event | +25 |

The actual score weights should be kept in one configuration area in the code so they can be adjusted without editing many functions. Apply each score when an event is recorded—not continuously while a sensor remains active. Use cooldowns or event-transition logic to avoid counting the same ongoing condition on every loop.

**Example:** A sensor anomaly (+20) followed by a honeypot access (+30) and a physical intrusion event (+25) produces a score of 75, assuming no score cap or decay is applied. The planned score bands should be documented alongside the classifier in the sketch.

**Expected result:** A numeric score that summarizes the event history. It is a heuristic for demonstration, not a calibrated probability that an attack is occurring.

### Module 7 — Attack and threat classification

**Purpose:** Turn a numeric score and event context into an understandable status.

The classifier can map scores to categories such as:

- **LOW / SECURE:** little or no suspicious activity.
- **MEDIUM / ALERT:** one or more lower-severity events have occurred.
- **HIGH:** multiple or higher-weight events have accumulated.
- **CRITICAL:** the score crosses the configured critical threshold.
- **LOCKDOWN:** a separate access-control state that should take priority over ordinary score labels.

Keep the exact thresholds consistent with the code. A classification should not be presented as proof of an attack; sensor faults, environmental changes, and authorized activity can also create events.

**Expected result:** The OLED and RGB LED reflect the current state using clear, consistent labels.

### Module 8 — Automated response

**Purpose:** Make the system react to detected events without requiring manual intervention for every event.

Possible responses in the integrated design include:

- Update the system status and threat level.
- Set the RGB LED to the configured normal, warning, or critical color.
- Trigger a finite buzzer pattern for a new alert event.
- Keep lockdown active when the authentication protection has been triggered.
- Display a status dashboard temporarily, then return to the PIN-entry screen when appropriate.
- Record recovery when a condition clears.

**Buzzer design requirement:** The sensor loop must not restart a continuous tone on every iteration. Prefer a non-blocking pattern driven by `millis()`—for example, a short beep, silence, another short beep, and then silence. A persistent condition may remain visible as an alert, but that should not automatically mean the buzzer sounds continuously.

**Expected result:** Alerts are noticeable but bounded, and the controller continues reading sensors and serving HTTP requests during the beep sequence.

### Module 9 — Event logging and event history

**Purpose:** Preserve a readable record of what the system detected.

Events can include:

- Startup and network-server status.
- Successful or failed authentication.
- Lockdown and recovery actions.
- Temperature, humidity, or light anomalies and recovery.
- PIR motion transitions.
- Ultrasonic proximity transitions.
- Honeypot endpoint requests.
- Threat-score changes and classifications.

Each event should ideally include a timestamp or elapsed-time value, event type, a short explanation, and any score change. In the Wokwi prototype, serial output and in-memory event counters/logs are useful for demonstrations. Unless persistent storage is implemented, logs may be lost when the ESP32 resets.

**Expected result:** A tester can correlate a sensor or HTTP action with the event message and threat-score change.

### Module 10 — Integration, validation, and final demonstration

**Purpose:** Confirm that the individual modules operate together and that normal conditions can be recovered from.

This module is the integration and test stage rather than a separate sensor. Verify that:

- All modules use consistent status variables and event types.
- A single sustained sensor condition does not create repeated events every loop.
- The buzzer pattern finishes and the buzzer returns to silence.
- Sensor recovery clears the active condition.
- The OLED returns to the PIN-entry screen after the configured dashboard interval when no active alert or lockdown requires the dashboard.
- The web server remains responsive while the buzzer and display update.
- Lockdown cannot be bypassed by a normal sensor recovery event.
- The project compiles and the Wokwi simulation passes the test checklist below.

**Expected result:** A repeatable end-to-end demonstration of monitoring, detection, scoring, classification, response, logging, and recovery.

## 7. Threat-scoring model

The scoring engine is intentionally simple. Each qualifying event contributes a configured number of points. The resulting score is used to choose a threat label and display color.

A conceptual example:

```text
Failed PIN attempt        +5
Honeypot endpoint access +30
Physical intrusion       +25
----------------------------
Example total             60
```

Use these safeguards in the implementation:

- **Score once per event:** Do not add points every loop while a sensor stays active.
- **Use edge/state transitions:** Log “motion detected” when motion changes from inactive to active, and log “motion cleared” when it becomes inactive.
- **Use cooldowns where appropriate:** Repeated network requests can be grouped or rate-limited so a flood is detected without producing unbounded duplicate records.
- **Define score bounds:** If the design uses a 0–100 display, cap the score at 100 or clearly explain any other policy.
- **Define reset/decay behavior:** A cumulative score can remain high after the physical condition clears. Clearing an active sensor alert and lowering the historical threat score are separate decisions. Do not silently conflate them.
- **Keep lockdown separate:** Lockdown is an access-control state; it should not disappear merely because the temperature or distance returns to normal.

Suggested severity bands (make these match the actual code):

| Score | Example label | Example indicator |
|---:|---|---|
| 0–19 | LOW / SECURE | Green |
| 20–49 | MEDIUM / ALERT | Amber or blue/yellow combination, depending on the LED implementation |
| 50–79 | HIGH | Red |
| 80–100 | CRITICAL | Red and a clear critical label |

The thresholds above are a documentation suggestion. If the current sketch uses different thresholds, update this table to match the implementation exactly.

## 8. Alert, buzzer, and recovery behavior

A security monitor needs to communicate both **detection** and **recovery**. An alarm that never stops after a sensor returns to normal is difficult to interpret and may indicate that the active-state logic is not separated from the sound pattern.

### Intended behavior

| Trigger | On detection | When condition clears |
|---|---|---|
| Person/object enters the configured proximity zone | Record one proximity event and play a short beep pattern | Rearm after the object moves outside the zone; stop/refrain from repeating the sound continuously |
| PIR changes from no motion to motion | Record one motion event and play a short pattern | Record/recognize motion-clear state; rearm for the next motion event |
| Temperature/humidity crosses configured threshold | Record an environmental alert and play a short pattern | Record environmental recovery once values return within limits |
| LDR reading crosses configured abnormal range | Record a light anomaly and play a short pattern | Record recovery once the reading returns to the normal range |
| Honeypot endpoint receives suspicious access | Record request and update threat score; optionally play a bounded alert | Do not repeat the same alert sound indefinitely for a single request |
| Lockdown is entered | Keep lockdown status visible and use the configured warning behavior | Exit only through the authorized recovery procedure |

### Recommended implementation principle

Use two separate states:

1. **Condition state:** Is the sensor or security condition currently active?
2. **Buzzer-pattern state:** Is a finite sound sequence currently being played?

A condition can remain active while a three-beep pattern finishes. When the condition clears, update the condition state and log recovery. Do not call an unlimited `tone()` repeatedly from `loop()` or a status-update function. If a continuous alarm is specifically required for lockdown, make that a separate, intentional policy with an explicit recovery path.

### OLED behavior

- During an event, show a dashboard with system status, threat score, event count, and environment readings.
- After the configured display interval, return to the PIN-entry screen when there is no active alert and the system is not locked down.
- While lockdown is active, keep the lockdown information visible until the defined recovery process succeeds.
- Ensure display refreshes do not repeatedly restart the buzzer.

> The table above describes the desired behavior. Confirm it against the latest sketch and Wokwi run before claiming that every recovery path has been validated.

## 9. Web honeypot endpoints

The honeypot is a small decoy HTTP surface. It is intended to help demonstrate logging and threat scoring when someone requests a path that looks administrative or sensitive.

### Starting the server

The simulation uses the Wokwi guest network (`Wokwi-GUEST`) in the configured environment. On successful startup, the serial monitor should report the Wi-Fi connection and the local server address. The project has previously displayed an address such as `10.0.0.2` or `10.10.0.2` in the simulation; use the **current address printed by your run**, not a hard-coded address from an old screenshot.

### Testing endpoints

From a browser or HTTP client that can reach the simulated ESP32, request each registered endpoint:

```text
http://<ESP32-IP>/admin
http://<ESP32-IP>/login
http://<ESP32-IP>/config
http://<ESP32-IP>/debug
http://<ESP32-IP>/device
```

Replace `<ESP32-IP>` with the address shown in the Serial Monitor. If the simulator or network environment does not permit access from your browser, use the project's supported serial-input simulation path if available.

**Safety note:** These are demonstration endpoints. Do not expose the prototype to the public internet or use it to collect real credentials. A honeypot should be isolated, monitored, and designed to avoid storing sensitive user data.

## 10. Setup and run in Wokwi

1. Open the Sentinel-X Wokwi project.
2. Confirm the ESP32 board and wiring match the pin table in this README.
3. Check that the sketch includes the required libraries.
4. Start the simulation.
5. Open the Serial Monitor and confirm the startup messages.
6. Confirm the OLED initializes and displays the login/startup screen.
7. Test PIN entry on the 4×4 keypad.
8. Change sensor inputs using Wokwi's sensor controls to test normal and abnormal values.
9. Test each honeypot endpoint using the current IP printed by the simulation, if network access is supported.
10. Verify that the event log, score, status, LED, buzzer, and OLED agree with each test.
11. Test recovery by returning the sensor values to normal and confirming the alert behavior stops/rearms correctly.
12. Save a screenshot or short recording of the successful test sequence for the project portfolio.

### Configuration values to review

Before presenting the project, check the sketch for the current values of:

- Demonstration PIN.
- Maximum failed PIN attempts.
- Session timeout.
- Temperature and humidity thresholds.
- LDR normal range.
- Ultrasonic proximity threshold and minimum valid distance.
- Threat-score weights and classification boundaries.
- Sensor polling interval and event cooldowns.
- OLED dashboard display interval.
- Buzzer on/off durations and number of beeps.

This README lists values previously used or planned for the project. The current source code is the final authority if any value has changed.

## 11. Testing checklist

Use this checklist during final integration. Mark a test as passed only after observing it in the current Wokwi simulation or on the actual hardware.

### Startup and display

- [ ] ESP32 starts without compilation/runtime errors.
- [ ] OLED initializes at address `0x3C`.
- [ ] Serial Monitor prints useful startup diagnostics.
- [ ] Login screen appears when no alert is active.
- [ ] Dashboard shows status, threat score, event count, and environmental values during an event.
- [ ] Dashboard returns to PIN entry after its display interval when appropriate.

### Authentication

- [ ] Correct demo PIN grants a session.
- [ ] Incorrect PIN is rejected and logged.
- [ ] Failed attempts affect the score as configured.
- [ ] The configured failed-attempt limit enters lockdown.
- [ ] Normal sensor recovery does not clear lockdown.
- [ ] Authorized recovery exits lockdown and records the action.
- [ ] Session timeout works as intended.

### Sensors and recovery

- [ ] DHT22 temperature above the threshold creates one alert event.
- [ ] DHT22 humidity above the threshold creates one alert event.
- [ ] Returning temperature/humidity to normal records recovery.
- [ ] LDR values outside the configured range create an alert.
- [ ] Returning LDR values to the normal range records recovery.
- [ ] PIR motion creates one event on activation, not one event per loop.
- [ ] PIR inactive state rearms motion detection.
- [ ] HC-SR04 distance below the configured threshold creates one proximity event.
- [ ] Moving the object outside the threshold rearms proximity detection.
- [ ] Invalid ultrasonic readings are ignored safely.

### Buzzer and LED

- [ ] A normal alert produces the configured finite beep pattern.
- [ ] The buzzer becomes silent when the pattern finishes.
- [ ] A persistent sensor condition does not restart the pattern continuously unless explicitly configured.
- [ ] A new independent event can trigger a new pattern.
- [ ] RGB LED color agrees with system status.
- [ ] Lockdown indication remains visible until authorized recovery.

### Honeypot, scoring, and logs

- [ ] Wi-Fi/server startup status is reported.
- [ ] Each configured honeypot endpoint responds as expected.
- [ ] Endpoint access is logged with a meaningful event label.
- [ ] Score changes once per qualifying event.
- [ ] Attack/threat classification follows the configured score bands.
- [ ] Request-flood logic, if implemented, distinguishes repeated requests from ordinary single requests.
- [ ] The system continues processing sensor readings while serving requests and playing the buzzer pattern.

## 12. Serial Monitor and debugging

The Serial Monitor is useful for confirming the order of events. Keep logs concise and include the event type and score change where possible.

Useful things to look for:

- Wi-Fi connection and server startup messages.
- Sensor values and threshold transitions.
- PIN success/failure and lockdown transitions.
- Honeypot endpoint names requested.
- Threat score before/after an event.
- Environmental, motion, and proximity recovery messages.
- Buzzer pattern start/completion messages, if implemented.

If logs appear too quickly, avoid adding long blocking delays to the main loop as a first fix. Prefer a timestamped log interval or a non-blocking `millis()` check so the sensor, keypad, and web server continue to be serviced.

If the buzzer stays on, search the sketch for every `tone()`, `noTone()`, and buzzer-related call. Make sure the continuous status-update function does not repeatedly call `tone()` and that every finite pattern reaches a completion branch that calls `noTone()`.

## 13. Security considerations and limitations

- **Educational prototype:** The project demonstrates concepts; it is not certified or suitable for protecting real property.
- **Demo PIN:** A PIN hard-coded in source code is not secure credential storage. Never use a personal or production password in a public repository.
- **No cryptographic trust boundary:** The prototype's local authentication and HTTP endpoints should not be treated as secure remote administration.
- **No HTTPS by default:** A simple embedded HTTP server does not provide TLS protection. Do not transmit real credentials over it.
- **Volatile logs:** Unless persistent storage or a remote log collector is implemented, events may be lost on reset or power loss.
- **Heuristic scoring:** Scores are manually assigned weights, not statistically validated probabilities.
- **Sensor reliability:** PIR sensors can have warm-up delays and false triggers; ultrasonic sensors can misread soft, angled, or distant surfaces; LDR values depend on calibration; DHT readings have tolerances and sampling delays.
- **Simulation versus hardware:** Wokwi demonstrates logic but does not validate electrical safety, enclosure design, sensor placement, power stability, or real-world attack resistance.
- **Network accessibility:** The Wokwi guest network and simulator networking may limit which clients can reach the server.
- **Privacy:** Avoid logging real credentials or personal information. Keep tests within systems you own or have permission to assess.

## 14. Future improvements

Potential extensions for a stronger cybersecurity portfolio project:

1. Add a persistent event log using suitable flash storage or an SD card.
2. Send structured events to a local dashboard or SIEM such as an Elasticsearch-based stack.
3. Add a configurable score decay/reset policy and document why it is safe.
4. Add rate limiting and request-window analysis for the HTTP honeypot.
5. Use a small, explicit finite-state machine for `SECURE`, `AUTHENTICATED`, `ALERT`, `HIGH`, `CRITICAL`, and `LOCKDOWN` states.
6. Add unit-style tests for scoring and classification functions.
7. Add sensor calibration and hysteresis to reduce repeated alerts near thresholds.
8. Add event IDs, elapsed timestamps, and consistent severity fields.
9. Add a secure configuration method rather than hard-coding credentials.
10. If connecting to a real network, isolate the device and add appropriate authentication, encrypted transport, update controls, and logging safeguards.
11. Document each test with setup, expected output, actual output, and a screenshot.
12. Add a threat model identifying assets, entry points, trust boundaries, abuse cases, and mitigations.

## 15. Repository structure

A clean repository could use this layout:

```text
Sentinel-X/
├── README.md              # Project overview, wiring, modules, and test plan
├── sketch.ino             # ESP32 Arduino source
├── diagram.json           # Wokwi circuit diagram
├── libraries.txt          # Optional: project library list, if used
├── docs/
│   ├── architecture.png   # System architecture diagram
│   ├── wiring.png         # Wiring/simulation screenshot
│   └── test-results.md    # Actual test observations and evidence
└── screenshots/
    ├── startup.png
    ├── alert-state.png
    └── honeypot-log.png
```

Only include files that actually exist in the repository. If your Wokwi project keeps the diagram and sketch in its online project rather than in GitHub, link to that project from this README.

## 16. Learning outcomes

By building Sentinel-X, a learner can practice:

- ESP32 programming and sensor integration.
- GPIO, I²C, analog input, and digital input concepts.
- Keypad input and basic session-state handling.
- HTTP server endpoints and honeypot concepts.
- Event-driven programming and finite-state thinking.
- Threat scoring and rule-based classification.
- Alert handling, recovery logic, and non-blocking timing with `millis()`.
- Serial debugging and systematic integration testing.
- Security documentation, limitations analysis, and evidence-based reporting.

## 17. Credits

**Project:** Sentinel-X  
**Author:** Elbin Sam  
**Platform:** ESP32 / Arduino framework / Wokwi  
**Focus:** Embedded cybersecurity, intrusion detection concepts, sensor monitoring, and honeypot-based event logging

If this project is developed collaboratively, add the contributors and their roles here.

---

## Project status

Sentinel-X is an educational modular prototype. Update this section as you complete testing:

- [ ] Modules 1–5 validated in the current simulation.
- [ ] Modules 6–7 threat scoring and classification validated.
- [ ] Module 8 finite buzzer pattern and recovery behavior validated.
- [ ] Module 9 event logging validated.
- [ ] Module 10 end-to-end tests completed.
- [ ] README wiring and thresholds checked against the latest source code.

**Important:** Do not mark a checkbox complete until you have confirmed the behavior in the current code and simulation.
