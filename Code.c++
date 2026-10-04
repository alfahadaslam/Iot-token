#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ESP32Servo.h>

// --- PIN MAPPING (ESP32 30-PIN / NO GPIO 17) ---
#define RFID_SS_PIN    5
#define RFID_RST_PIN   26  // Remapped RST pin
#define TFT_CS         15
#define TFT_DC         2
#define TFT_RST        4
#define TRIG_PIN       12
#define ECHO_PIN       13
#define SERVO_PIN      14
#define BUZZER_PIN     27

// Custom color definition (16-bit 565 RGB)
#define ST7735_DARKCYAN 0x03E0

// --- INSTANTIATE HARDWARE OBJECTS ---
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Servo vaultServo;

// --- WALLET DATA ---
int walletBalance = 50; // Starting Balance ($50)

// --- SINGLE TOKEN CONFIGURATION ---
String MY_SINGLE_CARD_UID = "A3 B2 C1 04"; 
String DUMMY_REJECT_UID   = "XX XX XX XX"; 

// UI State Management Flags (Fixes screen redraw lock)
bool isUserPresent = false;

// Forward Declarations
long readUltrasonicCM();
String getCardUID();
void playToneSequence(bool success);
void drawIdleScreen();
void drawUserApproachScreen();
void drawSuccessScreen(int addedAmount);
void drawErrorScreen();
void drawProgressBar();
void smoothServoMove(int targetAngle);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("--- ESP32 Hardware Wallet Starting ---");

  // Initialize GPIO Modes
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize SPI Bus & MFRC522 RFID Reader
  SPI.begin();
  rfid.PCD_Init();

  // Initialize 1.8" ST7735 TFT LCD Screen
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1); // Landscape orientation (160x128)
  
  // Initialize Servo Actuator
  vaultServo.attach(SERVO_PIN);
  vaultServo.write(0); // Locked state (0°)

  // Startup Chime
  playToneSequence(true);

  // Draw Initial Idle Display
  drawIdleScreen();
}

void loop() {
  long distance = readUltrasonicCM();

  // 1. Proximity Check: Is user closer than 25 cm?
  if (distance > 0 && distance < 25) {
    // Only redraw the screen when transitioning from away -> present
    if (!isUserPresent) {
      drawUserApproachScreen();
      isUserPresent = true;
    }

    // 2. Scan for physical RFID card tap on MFRC522 reader
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String readUID = getCardUID();
      Serial.println("Scanned Token UID: " + readUID);

      // Check if scanned card matches your programmed card
      if (readUID == MY_SINGLE_CARD_UID) {
        walletBalance += 10;          // Add $10 to balance
        drawSuccessScreen(10);        // Green success screen + unlock servo + sound
      } else {
        drawErrorScreen();            // Red error screen + stay locked + buzz
      }

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      
      // Reset state and redraw approach UI
      isUserPresent = false;
      drawIdleScreen();
    }
  } else {
    // User stepped away: Return to Standby Screen
    if (isUserPresent || distance >= 25) {
      drawIdleScreen();
      isUserPresent = false;
    }
  }

  delay(100);
}

// --- HARDWARE UTILITY FUNCTIONS ---

long readUltrasonicCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 20000); // 20ms timeout
  if (duration == 0) return 999;
  return (duration * 0.034) / 2;
}

String getCardUID() {
  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) uidStr += " ";
  }
  uidStr.toUpperCase();
  return uidStr;
}

// Smooth low-current stepped servo rotation
void smoothServoMove(int targetAngle) {
  int currentAngle = vaultServo.read();
  int step = (targetAngle > currentAngle) ? 3 : -3;
  
  for (int pos = currentAngle; pos != targetAngle; pos += step) {
    vaultServo.write(pos);
    delay(25);
  }
  vaultServo.write(targetAngle);
}

void playToneSequence(bool success) {
  if (success) {
    tone(BUZZER_PIN, 1200, 120);
    delay(140);
    tone(BUZZER_PIN, 1800, 180);
    delay(200);
    noTone(BUZZER_PIN);
  } else {
    tone(BUZZER_PIN, 300, 400);
    delay(450);
    noTone(BUZZER_PIN);
  }
}

// --- GRAPHICAL UI FUNCTIONS ---

void drawIdleScreen() {
  tft.fillScreen(ST7735_BLACK);
  
  // Draw Central Coin Symbol
  tft.drawCircle(80, 50, 30, ST7735_CYAN);
  tft.drawCircle(80, 50, 29, ST7735_CYAN);
  tft.fillCircle(80, 50, 22, ST7735_BLUE);
  tft.fillCircle(80, 50, 10, ST7735_YELLOW);

  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 95);
  tft.print("TAP TOKEN");
}

void drawUserApproachScreen() {
  tft.fillScreen(ST7735_BLACK);

  // Card Container
  tft.fillRoundRect(10, 15, 140, 98, 8, ST7735_BLUE);
  tft.drawRoundRect(10, 15, 140, 98, 8, ST7735_WHITE);

  // Balance Icon & Counter
  tft.fillCircle(35, 45, 12, ST7735_YELLOW);
  tft.setTextColor(ST7735_BLACK, ST7735_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(30, 38);
  tft.print("$");

  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(3);
  tft.setCursor(60, 35);
  tft.print(walletBalance);

  // Status Indicator Bar
  tft.fillRect(10, 80, 140, 33, ST7735_DARKCYAN);
  tft.setTextSize(1);
  tft.setCursor(25, 92);
  tft.setTextColor(ST7735_GREEN);
  tft.print("READY TO SCAN...");
}

void drawSuccessScreen(int addedAmount) {
  // Vibrant Green Success Screen
  tft.fillScreen(ST7735_GREEN);

  // Large White Checkmark Symbol
  tft.fillCircle(80, 40, 25, ST7735_WHITE);
  tft.fillTriangle(68, 40, 76, 50, 92, 30, ST7735_GREEN);

  // Value Display
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(3);
  tft.setCursor(35, 75);
  tft.print("+$");
  tft.print(addedAmount);

  // Audio & Motor Action
  playToneSequence(true);
  smoothServoMove(90);      // Unlock Vault Gate (90°)
  drawProgressBar();         // Keep unlocked for 3 seconds
  smoothServoMove(0);       // Relock Vault Gate (0°)
}

void drawErrorScreen() {
  // Bold Red Error Screen
  tft.fillScreen(ST7735_RED);

  // Large White Cross Symbol
  tft.fillCircle(80, 45, 25, ST7735_WHITE);
  tft.drawLine(68, 33, 92, 57, ST7735_RED);
  tft.drawLine(69, 33, 93, 57, ST7735_RED);
  tft.drawLine(68, 57, 92, 33, ST7735_RED);
  tft.drawLine(69, 57, 93, 33, ST7735_RED);

  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 85);
  tft.print("REJECTED");

  playToneSequence(false);
  delay(1500);
}

void drawProgressBar() {
  int barWidth = 120;
  int barHeight = 10;
  int startX = 20;
  int startY = 110;

  tft.drawRect(startX, startY, barWidth, barHeight, ST7735_BLACK);

  for (int i = 0; i <= (barWidth - 4); i += 4) {
    tft.fillRect(startX + 2, startY + 2, i, barHeight - 4, ST7735_BLACK);
    delay(80);
  }
}
