This design for a **Tangible "Token-Based" IoT Hardware Wallet** is optimized for low-literacy users. Rather than relying on text-heavy menus or complex PIN codes, this hardware wallet uses physical RFID tokens representing tangible currency/voucher values (e.g., Green Token = $10, Gold Token = $50) paired with intuitive visual icons, color-coded screens, sound tones, and physical servo vault locking. 

# **1. System Architecture & User Flow** 

```
[ Ultrasonic Sensor ] ---> Detects User Proximity ---> Wakes Up Display
[ RFID Token Tapped ] ---> MFRC522 Reader Scans UID  ---> Verifies Value
[ Processing ]       ---> Updates Wallet Balance    ---> Triggers Feedback
                           ├── 1.8" TFT LCD: Visual Icon (Green Check /
Coins)
                           ├── 16x2 LCD: Visual Progress Bar / Balance
                           ├── Active Buzzer: Audio Tone (Success / Fail)
                           └── Servo Motor: Unlocks Physical Token Gate
```

1. **Presence Wake-Up:** The HC-SR04 Ultrasonic sensor detects proximity (< 25 cm) to activate the wallet screens from sleep mode. 

2. **Visual & Icon-Based Interface:** The 1.8" TFT LCD displays full-color visual cues (large icons, color-coded feedback screens, and progress bars). 

3. **Tangible Token Reader:** User taps a physical RFID token or card. Each token UID maps to a token value or transaction function (Deposit, Withdraw, Check Balance). 

4. **Physical Vault Actuation:** Upon successful validation, the Servo motor opens a token compartment or latch for 3 seconds before auto-locking. 

5. **Auditory Feedback:** Distinct audio tones reinforce every action (e.g., high double beep for success, low buzz for rejection). 

# **2. Components List** 

|**Component**|**Quan**|**tity**<br>**Purpose in Project**|**Operating V**|
|---|---|---|---|
|**ESP32 DevKit V1**|1|Master Microcontroller with Wi-Fi/BT|5V (USB) / 3.3|
|**MFRC522 RFID Reader**|1|Reads physical 13.56 MHz tokens/cards|**3.3V strictly**|
|**1.8" ST7735 SPI TFT LCD**|1|Primary icon-based visual GUI (128x160)|3.3V / 5V|
|**16x2 LCD with I2C Backpack**|1|Secondary high-contrast numeric display|5V|
|**HC-SR04 Ultrasonic Sensor**|1|Non-contact proximity user detection|5V|
|**SG90 Micro Servo Motor**|1|Physical locking/latch mechanism|5V|
|**Active Buzzer**|1|Sound cue/tone generator|3.3V / 5V|
|**OV7670 / ESP32-CAM**|1|Audit snapshot / security photo capture|3.3V|
|**RFID Cards / Keyfobs**|3+|Tangible Tokens ($10, $50, Admin)|N/A|



**Component Quantity 5V 2A DC Power Adapter** 1 

**Operating V** 5V DC 

**Purpose in Project** Power supply for ESP32 and Servo 

# **<mark>3. Circuit Pin Mapping</mark>** 

Because the ESP32 shares its hardware **SPI bus** between the ST7735 TFT LCD and the MFRC522 RFID reader, distinct Chip Select (CS) and Reset pins are assigned: 

|**Device**|**Module Pin**|**ESP32 Pin**|**Notes**|
|---|---|---|---|
|**RFID RC522**|SDA / SS|**GPIO 5**|SPI Chip Select|
||SCK|**GPIO 18**|Shared SPI Clock|
||MOSI|**GPIO 23**|Shared SPI Master Out|
||MISO|**GPIO 19**|SPI Master In|
||RST|**GPIO 17**|RFID Reset|
|**1.8" ST7735 TFT**|CS|**GPIO 15**|TFT Chip Select|
||DC / A0|**GPIO 2**|Data/Command Pin|
||RES / RST|**GPIO 4**|TFT Reset|
||SCL / SCK|**GPIO 18**|Shared SPI Clock|
||SDA / MOSI|**GPIO 23**|Shared SPI Master Out|
|**16x2 I2C LCD**|SDA|**GPIO 21**|I2C Data|
||SCL|**GPIO 22**|I2C Clock|
|**HC-SR04**|TRIG|**GPIO 12**|Ultrasonic Trigger|
||ECHO|**GPIO 13**|Ultrasonic Echo|
|**Servo Motor**|Signal|**GPIO 14**|PWM Control|
|**Buzzer**|VCC / IO|**GPIO 27**|Audio Feedback|





<!-- Start of picture text -->
Wiring a 128°160 SPI ST7735 TFT display to an ESP32-C3 Super Mini }<br>S56<br>ees 4ee<br>_————<br>line———rF wwe elie<br>lO. Oo @ Oc oo) - ay<br>Ledley T°)ers) etOn i e ee—H<br>9 whee 4, Oh ate<br>temperature °C a ima @ _<br>2 Tn +0<br>A rad display ESP32C3) fe. |<br>& Fi% humidityiia GNDvec — ®> GND3v3 if.jee188<br>z scl —® 2 |E><br>49.8 SDARES — >e 40 \23BE<br>bccs —>—> 51 2oT]<br>ESP32-C3 + SHT21 BL —> 3v3<br>OUIN| e<br>® nm @ bso |<br>© FGW 2024<br><!-- End of picture text -->

```
ESP32 SPI Display Connections Layout. Source:
thesolaruniverse - WordPress.com
```

**Note on OV7670 Integration:** A standalone OV7670 camera module requires 16+ parallel pins, which consumes all available GPIOs when combined with SPI displays and sensors. For production builds, connect an **ESP32-CAM** as a secondary camera node over UART (TX2/RX2 pins on GPIO 16/17) to capture audit photos without pin contention. 

# **4. Complete Arduino IDE Code** 

**Required Libraries in Arduino IDE:** 

Install these via **Sketch -> Include Library -> Manage Libraries** : 

 `MFRC522` by GitHubCommunity 

- `Adafruit GFX Library` <mark>by Adafruit</mark> 

- `Adafruit ST7735 and ST7789 Library` <mark>by Adafruit</mark> 

- `LiquidCrystal I2C` <mark>by Frank de Brabander</mark> 

- `ESP32Servo` <mark>by Kevin Harrington</mark> 

## C++ 

```
#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
```

```
// --- PIN DEFINITIONS ---
#define RFID_SS_PIN    5
#define RFID_RST_PIN   17
#define TFT_CS         15
#define TFT_DC         2
#define TFT_RST        4
#define TRIG_PIN       12
#define ECHO_PIN       13
#define SERVO_PIN      14
#define BUZZER_PIN     27
```

```
// --- INSTANTIATE OBJECTS ---
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo vaultServo;
// --- WALLET DATA ---
int walletBalance = 50; // Initial token balance
// --- TOKEN UIDs (Customize for your RFID Tags) ---
String TOKEN_10_UID = "A3 B2 C1 04";  // Adds $10 / 10 Credits
String TOKEN_50_UID = "54 F3 D2 11";  // Adds $50 / 50 Credits
void setup() {
  Serial.begin(115200);
  // Pin Modes
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  // Initialize I2C LCD
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
```

```
  // Initialize SPI & RFID
  SPI.begin();
  rfid.PCD_Init();
  // Initialize ST7735 TFT Display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  // Initialize Servo
  vaultServo.attach(SERVO_PIN);
  vaultServo.write(0); // Locked position
  // Boot Feedback
  playBeep(1000, 100);
  playBeep(1500, 100);
  renderIdleUI();
}
void loop() {
  // 1. Proximity Sensor Check
  long distance = readUltrasonicCM();
  if (distance < 25 && distance > 0) {
    // User is near: Wake up UI
    renderUserPresentUI();
    // 2. Check for RFID Token
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String readUID = getCardUID();
      Serial.println("Token Scanned UID: " + readUID);
      if (readUID == TOKEN_10_UID) {
        processTransaction(10, "TOKEN +10");
      } else if (readUID == TOKEN_50_UID) {
        processTransaction(50, "TOKEN +50");
      } else {
        processUnknownToken();
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      delay(2000);
      renderIdleUI();
    }
  } else {
    renderIdleUI();
  }
  delay(200);
}
// --- HELPER FUNCTIONS ---
long readUltrasonicCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
```

```
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 20000); // 20ms timeout
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}
String getCardUID() {
  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.byte[i] < 0x10) uidStr += "0";
    uidStr += String(rfid.uid.byte[i], HEX);
    if (i < rfid.uid.size - 1) uidStr += " ";
  }
  uidStr.toUpperCase();
  return uidStr;
}
void processTransaction(int amount, String label) {
  walletBalance += amount;
  // Sound Success Chord
  playBeep(1200, 150);
  playBeep(1800, 200);
  // Graphical TFT Feedback
  tft.fillScreen(ST7735_GREEN);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(3);
  tft.setCursor(30, 30);
  tft.print("[ OK ]");
  tft.setTextSize(2);
  tft.setCursor(20, 80);
  tft.print("BAL: $");
  tft.print(walletBalance);
  // 16x2 LCD Text
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCEPTED!");
  lcd.setCursor(0, 1);
  lcd.print("BAL: $" + String(walletBalance));
  // Unlock Servo Latch
  vaultServo.write(90);
  delay(3000);
  vaultServo.write(0);
}
void processUnknownToken() {
  // Sound Error Tone
  playBeep(400, 400);
  // TFT Red Error Screen
  tft.fillScreen(ST7735_RED);
```

<mark>`tft.setTextColor(ST7735_WHITE); tft.setTextSize(3); tft.setCursor(30, 40); tft.print("[ X ]"); tft.setTextSize(2); tft.setCursor(15, 90); tft.print("REJECTED"); // LCD Text lcd.clear(); lcd.setCursor(0, 0); lcd.print("INVALID TOKEN"); } void renderIdleUI() { tft.fillScreen(ST7735_BLACK); tft.setTextColor(ST7735_BLUE); tft.setTextSize(2); tft.setCursor(10, 40); tft.print("TAP TOKEN"); // Draw Icon Circle tft.drawCircle(80, 100, 20, ST7735_YELLOW); tft.fillCircle(80, 100, 15, ST7735_YELLOW); lcd.clear(); lcd.setCursor(0, 0); lcd.print("TANGIBLE WALLET"); lcd.setCursor(0, 1); lcd.print("READY TO SCAN"); } void renderUserPresentUI() { tft.fillScreen(ST7735_BLACK); tft.setTextColor(ST7735_GREEN); tft.setTextSize(2); tft.setCursor(20, 20); tft.print("USER NEAR"); tft.setTextSize(2); tft.setTextColor(ST7735_WHITE); tft.setCursor(10, 70); tft.print("BAL: $"); tft.print(walletBalance); } void playBeep(int frequency, int duration) { tone(BUZZER_PIN, frequency, duration); delay(duration); noTone(BUZZER_PIN); }`</mark> Want me to add Wi-Fi cloud balance synchronization (Blynk/MQTT) to this ESP32 code? <mark>Yes</mark> 

**<mark>You said Also provide me this project full circuit diagram for the connection</mark>** <mark>Also provide me this project full circuit diagram for the connection</mark> 

Here is the **complete circuit connection diagram and power wiring breakdown** for the Tangible Token-Based IoT Hardware Wallet. 

# **1. Complete System Wiring Schematic** 



<!-- Start of picture text -->
[oa]aaarisex |<br>ize) sriss |<br>an? esp32io.com<br>~— ‘ : ——~eaN<br>3355535 i@!) i mass: Lh ces<br>me<br>=oa .<br>oo<br>RFID RC522 and Servo Interconnection with ESP32. Source:<br>ESP32 Tutorial<br>                                      +-------------------------+<br>                                      |   5V 2A DC POWER SUPPLY |<br>                                      +------------+------------+<br>                                                   |<br>                                     +-------------+-------------+<br>                                     |                           |<br>                                  +--+--+                     +--+--+<br>                                  | 5V  |                     | GND |<br>                                  +--+--+                     +--+--+<br>                                     |                           |<br>            +------------------------+---------------------------<br>+------------------------+<br>            |                        |                           |<br>|<br>            v                        v                           v<br>v<br>      +-----------+            +-----------+               +-----------+<br>+-----------+<br>      |  ESP32    |            | 16x2 LCD  |               | HC-SR04   |<br>| SG90 Servo|<br>      | DevKit V1 |            | (I2C)     |               | Ultrasonic|<br>| Motor     |<br>      +-----+-----+            +-----+-----+               +-----+-----+<br>+-----+-----+<br>            | V5 (VIN)               | VCC                       | VCC<br>| VCC<br><!-- End of picture text -->



<!-- Start of picture text -->
            | GND                    | GND                       | GND<br>| GND<br>            |                        |                           |<br>|<br>            |-- GPIO 21 (SDA) ------>| SDA                       |<br>|<br>            |-- GPIO 22 (SCL) ------>| SCL                       |<br>|<br>            |                                                    |<br>|<br>            |-- GPIO 12 (Trig) --------------------------------->| TRIG<br>|<br>            |-- GPIO 13 (Echo) <---------------------------------| ECHO<br>|<br>            |<br>|<br>            |-- GPIO 14 (PWM)<br>----------------------------------------------------------->| Control<br>            |<br>            |                  +-----------------------+<br>            |-- 3.3V Pin ----->| 3.3V POWER RAIL (BUS) |<br>            |                  +-----------+-----------+<br>            |                              |<br>            |               +--------------+--------------+<br>            |               |                             |<br>            |               v                             v<br>            |       +---------------+             +---------------+<br>            |       | MFRC522 RFID  |             | ST7735 1.8"   |<br>            |       | Module        |             | TFT LCD       |<br>            |       +-------+-------+             +-------+-------+<br>            |               | 3.3V                        | VCC/LED<br>            |               | GND                         | GND<br>            |               |                             |<br>            +--- GPIO 18 -->| SCK  (Shared SPI Clock) --->| SCL / SCK<br>            +--- GPIO 23 -->| MOSI (Shared SPI MOSI) --->| SDA / MOSI<br>            |--- GPIO 19 <--| MISO                        |<br>            |--- GPIO 5 --->| SDA / SS (Chip Select)      |<br>            |--- GPIO 17 -->| RST                         |<br>            |--- GPIO 15 -------------------------------->| CS (Chip Select)<br>            |--- GPIO 2  -------------------------------->| DC / A0<br>            |--- GPIO 4  -------------------------------->| RES / RST<br>            |<br>            |-- GPIO 27 (Signal) --> Active Buzzer (+) ---> GND (-)<br>2. Master Pin Interconnection Table<br><!-- End of picture text -->

|**Component Module**|**Module Pin**|**ESP32 Pin**|**Logic Voltage**|
|---|---|---|---|
|**ESP32 Microcontroller**|VIN / 5V|External 5V Rail|5V DC|
||GND|Common GND Rail|0V|
|**MFRC522 RFID Reader**|VCC|**3.3V Pin**|3.3V Strictly|
||GND|Common GND|0V|
||SCK|**GPIO 18**|3.3V|
||MOSI|**GPIO 23**|3.3V|



|**Component Module**|**Module Pin**|**ESP32 Pin**|**Logic Voltage**|
|---|---|---|---|
||MISO|**GPIO 19**|3.3V|
||SDA / SS|**GPIO 5**|3.3V|
||RST|**GPIO 17**|3.3V|
|**1.8" ST7735 TFT LCD**|VCC / LED|**3.3V Pin**|3.3V|
||GND|Common GND|0V|
||SCL / SCK|**GPIO 18**|3.3V|
||SDA / MOSI|**GPIO 23**|3.3V|
||CS|**GPIO 15**|3.3V|
||DC / A0|**GPIO 2**|3.3V|
||RES / RST|**GPIO 4**|3.3V|
|**16x2 I2C LCD Display**|VCC|5V Rail|5V|
||GND|Common GND|0V|
||SDA|**GPIO 21**|3.3V/5V|
||SCL|**GPIO 22**|3.3V/5V|
|**HC-SR04 Ultrasonic**|VCC|5V Rail|5V|
||GND|Common GND|0V|
||TRIG|**GPIO 12**|3.3V Output|
||ECHO|**GPIO 13**|5V (use 1k/2k divider)|
|**SG90 Servo Motor**|VCC (Red)|5V Power Rail|5V|
||GND (Brown)|Common GND|0V|
||Signal (Orange)|**GPIO 14**|3.3V PWM|
|**Active Buzzer**|VCC (+)|**GPIO 27**|3.3V|
||GND (-)|Common GND|0V|



# **<mark>3. Power Distribution & Wiring Safety Rules</mark>** 

1. **Common Ground Line (Crucial):** Connect the GND pins of the ESP32, 5V Power Supply, RFID module, Servo, TFT screen, and I2C LCD together to a **single common ground bus rail** . Unstable grounds cause sensor misreads and screen flicker. 

2. **Dedicated 5V Rail for Servo Motor:** Do **NOT** power the SG90 Servo motor directly from the ESP32 `3V3` or `VIN` pin via USB power alone. Servos draw voltage spikes (up to 500mA under movement load) that cause ESP32 brownout reset cycles. Connect the Servo VCC directly to an external 5V 2A power source. 

3. **Voltage Compatibility (RFID Reader):** The MFRC522 RFID reader operates **strictly at 3.3V** . Connecting the RFID VCC pin to 5V will burn out the chip instantly. 

4. **SPI Bus Bus-Sharing (RFID & TFT):** Both the MFRC522 and ST7735 TFT display share `GPIO 18 (SCK)` and `GPIO 23 (MOSI)` . They are 

distinguished using separate Chip Select lines ( `GPIO 5` for RFID and `GPIO 15` for TFT). 

Would you like a step-by-step soldering and breadboard layout guide for assembling this project? 

<mark>Yes</mark> 

**You said Yes, please provide a step-by-step breadboard and soldering wiring guide for assembling this project…** 

Yes, please provide a step-by-step breadboard and soldering wiring guide for assembling this project. 

## 1 

<mark>Establish Power Rails & Common Ground</mark> _<mark>`Prevents brownout resets and module damage`</mark>_ 

Before placing components, establish clear power zones on your breadboard: 

- **5V Rail (Red Power Bus #1):** Connect directly to your external 5V DC supply (or ESP32 `VIN` if using standard 5V USB input). 

- **3.3V Rail (Red Power Bus #2):** Connect directly to the `3V3` pin of the ESP32. **Do not connect 5V to this rail.** 

- **Common Ground Bus (Blue Rail):** Tie the GND of the ESP32, external 5V power supply, RFID module, TFT screen, 16x2 LCD, and Servo motor together on a single shared rail. 

**Critical Safety Rule:** The MFRC522 RFID reader **must** be wired to the 3.3V rail. Supplying 5V to the RFID module will permanently destroy its onboard IC. 

## 2 

<mark>Mount the ESP32 and Wire I2C Lines</mark> _<mark>`Sets up core processing and display control`</mark>_ 

1. Place the **ESP32 DevKit V1** across the center divider of the breadboard so pins on both sides remain accessible. 

2. Connect **ESP32 VIN** to the 5V Rail and **ESP32 GND** to the Common Ground Bus. 

<mark>3. Wire the</mark> **<mark>16x2 I2C LCD</mark>** <mark>:</mark> 

   - `VCC` <mark>to 5V Rail</mark> 

   - `GND` <mark>to Common Ground Rail</mark> 

   - `SDA` <mark>to</mark> **<mark>GPIO 21</mark>** 

   - `SCL` <mark>to</mark> **<mark>GPIO 22</mark>** 

3 

<mark>Wire the Shared 3.3V SPI Bus (RFID & TFT LCD)</mark> _<mark>`Connects low-voltage visual and reader peripherals`</mark>_ 

Both the **ST7735 1.8" TFT** display and the **MFRC522 RFID** reader share the hardware SPI clock and data lines: 

## 1. **Shared Bus Connections:** 

   - <mark>Connect both RFID</mark> `SCK` <mark>and TFT</mark> `SCL/SCK` <mark>to</mark> **<mark>GPIO 18</mark>** <mark>.</mark> 

   - <mark>Connect both RFID</mark> `MOSI` <mark>and TFT</mark> `SDA/MOSI` <mark>to</mark> **<mark>GPIO 23</mark>** <mark>.</mark> 

<mark>2.</mark> **<mark>MFRC522 Dedicated Lines:</mark>** 

   - `VCC` <mark>to</mark> **<mark>3.3V Rail</mark>** 

   - `GND` <mark>to Common Ground Rail</mark> 

   - `MISO` <mark>to</mark> **<mark>GPIO 19</mark>** 

   - `SDA (SS)` <mark>to</mark> **<mark>GPIO 5</mark>** 

   - `RST` <mark>to</mark> **<mark>GPIO 17</mark>** 

## <mark>3.</mark> **<mark>ST7735 TFT Dedicated Lines:</mark>** 

- `VCC` <mark>and</mark> `LED` <mark>backlight pins to</mark> **<mark>3.3V Rail</mark>** 

- `GND` <mark>to Common Ground Rail</mark> 

- `CS` <mark>to</mark> **<mark>GPIO 15</mark>** 

- `DC / A0` <mark>to</mark> **<mark>GPIO 2</mark>** 

- `RES / RST` <mark>to</mark> **<mark>GPIO 4</mark>** 

## 4 

<mark>Connect 5V Sensors, Servo Actuator, and Buzzer</mark> _<mark>`Provides tangible token feedback and presence sensing`</mark>_ 

## 1. **HC-SR04 Ultrasonic Proximity Sensor:** 

- `VCC` <mark>to 5V Rail |</mark> `GND` <mark>to Common Ground Rail</mark> 

- `TRIG` <mark>to</mark> **<mark>GPIO 12</mark>** 

- `ECHO` to **GPIO 13** (Note: If echo signals drift, run ECHO through a 1k Ohm / 2k Ohm resistor voltage divider to step 5V down to 3.3V). 

## <mark>2.</mark> **<mark>SG90 Servo Motor (Vault Latch):</mark>** 

- `VCC` (Red) strictly to the external 5V Power Rail (never directly from the ESP32 GPIO pins). 

- `GND` <mark>(Brown/Black) to Common Ground Rail.</mark> 

- `Signal` <mark>(Orange/Yellow) to</mark> **<mark>GPIO 14</mark>** <mark>.</mark> 

## <mark>3.</mark> **<mark>Active Audio Buzzer:</mark>** 

- <mark>Positive (</mark> `+` <mark>) pin to</mark> **<mark>GPIO 27</mark>** 

- <mark>Negative (</mark> `-` <mark>) pin to Common Ground Rail.</mark> 

5 

<mark>Breadboard Testing & Initial Sanity Check</mark> _<mark>`Verify connections prior to soldering`</mark>_ 

1. Power on the system with a 5V 2A wall adapter or powered USB hub. 

2. Adjust the potentiometer knob on the back of the **16x2 I2C backpack** until characters are clearly visible. 

<mark>3. Open the</mark> **<mark>Arduino IDE Serial Monitor</mark>** <mark>at</mark> `115200` <mark>baud rate.</mark> 

4. Move your hand within 20 cm of the HC-SR04 sensor to verify presence activation. 

5. Tap an RFID card over the MFRC522 reader and record the output UIDs to insert into your Arduino sketch variables ( `TOKEN_10_UID` and `TOKEN_50_UID` ). 

## 6 

## <mark>Soldering & Permanent Perfboard Assembly</mark> 

```
Steps for turning your breadboard layout into a durable
hardware wallet
```

Once breadboard testing passes, migrate the hardware to a **7x9 cm doublesided fiberglass perfboard** : 

1. **Solder Female Socket Headers:** Never solder the ESP32, RFID module, or TFT display directly onto the board. Solder **female pin headers** instead so components can easily be unplugged if replacement is needed. 

2. **Solder Power Bus Tracks:** Run 22 AWG solid-core wire across the board perimeter for **5V** , **3.3V** , and **GND** power rails. 

3. **Add Decoupling Capacitors:** Solder a **100µF electrolytic capacitor** directly across the 5V and GND pins of the servo motor connector. This buffers current surges during motor rotation and prevents ESP32 brownouts. 

4. **Strain Relief & External Cable Leads:** Solder JST connectors or screw terminals for the external 5V DC jack and SG90 servo wire runs to prevent solder joint fatigue during usage. 

