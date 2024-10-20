/*
  Software serial multiple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystal_I2C.h>

#define relay1 A0
#define relay2 A1

SoftwareSerial mySerial(5, 4);  // RX, TX
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define PN532_SS 10
Adafruit_PN532 nfc(PN532_SS);
uint8_t registered_uid_1[] = { 179, 226, 114, 48, 0, 0, 0 };  // Buffer to store the
int acc,wait = 0;

unsigned long writelast = 0;
String nano1return;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  lcd.init();
  // turn on the backlight
  lcd.backlight();

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1)
      ;  // halt
  }

  mySerial.begin(9600);

  // nfc.setPassiveActivationRetries(0xFF);
  Serial.println("Waiting for an ISO14443A card");
  lcd.setCursor(0, 0);
  lcd.print("Waiting for an ");
  lcd.setCursor(0, 1);
  lcd.print("Card");
}

void loop(void) {
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  // Serial.println(success);


  if (success) {
    wait = 0;
    acc = 0;
    // Serial.println("Found a card!");
    // Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    // Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(" 0x");
      Serial.print(uid[i]);
      if (uid[i] == registered_uid_1[i]) {
        acc += 1;
      } else {
        acc -= 1;
      }
    }
    Serial.print('\n');

    if (acc >= 4) {
      Serial.println("Access Granted");
      mySerial.print("Capture1");
      mySerial.print('\n');

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Granted");
      writelast = millis();

      digitalWrite(relay1, HIGH);
      delay(1000);
      digitalWrite(relay1, LOW);
      delay(1000);
      digitalWrite(relay2, HIGH);
      delay(1000);
      digitalWrite(relay2, LOW);
      delay(1000);
    } else if (acc <= -4) {
      Serial.println("Access Denied");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Denied");
      writelast = millis();
    }
    Serial.println("");
    delay(1000);
  } else {
    // PN532 probably timed out waiting for a card
    Serial.println("Timed out waiting for a card");
  }

  if(mySerial.available() > 0){
    nano1return = mySerial.readStringUntil('\n');
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(nano1return);
    wait = 1;
    writelast = millis();
  }
  Serial.print(writelast + String(" | "));
  Serial.println(wait);
  if (millis() - writelast >= 1000 && wait == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waiting for an ");
    lcd.setCursor(0, 1);
    lcd.print("Card");
    wait = 0;
  }
}
