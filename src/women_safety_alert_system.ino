/*
  Women's Safety Alert System
  Arduino Uno + GPS (NEO-6M) + GSM (SIM800L) + I2C LCD + Panic Button + Buzzer

  On panic-button press: fetches GPS location, sends an emergency SMS with
  coordinates to pre-saved contacts, and sounds a buzzer. A "STOP" SMS
  reply silences the buzzer and resets the system.

  Author: Sreelakshmi T B
  Built as part of the Maven Silicon Embedded System Design certification.
*/

#include <LiquidCrystal_I2C.h>   // I2C LCD driver
#include <Wire.h>                // I2C communication
#include <String.h>
#include <SoftwareSerial.h>

// LCD: address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// GPS module on pins 10 (RX), 11 (TX)
SoftwareSerial gps(10, 11);

// Reference string used to identify a valid GPGGA NMEA sentence
char *gpsRefString = "$GPGGA";
String gpsRespString = "";
String latitude = "No Range ";
String longitude = "No Range ";
int gpsRespCharCnt = 0;
bool gps_status = 0;      // GPS lock flag
int isGsmCmdRecv = 0;     // Incoming GSM command flag

const int buttonPin = 7;  // Panic button
bool btnState = HIGH;

#define BUZZER_PIN 13

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();

  Serial.begin(9600);   // GSM
  gps.begin(9600);      // GPS

  lcd.print("Alert system for");
  lcd.setCursor(0, 1);
  lcd.print("Women in Automobile");
  delay(1000);

  gsm_init();

  lcd.clear();
  lcd.print("GPS Initializing");
  lcd.setCursor(0, 1);
  lcd.print("No GPS Range");
  get_gps();
  delay(1000);

  lcd.clear();
  lcd.print("GPS RANGE FOUND");
  lcd.setCursor(0, 1);
  lcd.print("GPS is Ready");
  delay(1000);

  lcd.clear();
  lcd.print("System ready");
  isGsmCmdRecv = 0;
}

void loop() {
  get_gps();  // Continuously update GPS coordinates

  btnState = !digitalRead(buttonPin);  // active-low
  if (btnState) {
    btnState = 0;
    tracking(1);  // Alert to first contact
    tracking(2);  // Alert to second contact
  }

  serialEvent();  // Check for incoming GSM SMS commands

  if (isGsmCmdRecv) {
    isGsmCmdRecv = 0;
    digitalWrite(BUZZER_PIN, LOW);  // STOP received -> silence buzzer
  }
}

// Listen for an incoming "STOP" SMS
void serialEvent() {
  while (Serial.available()) {
    if (Serial.find("STOP")) {
      isGsmCmdRecv = 1;
      break;
    } else {
      isGsmCmdRecv = 0;
    }
  }
}

// Read one GPS sentence and validate it as a GPGGA string
void gpsEvent() {
  gpsRespString = "";
  while (1) {
    while (gps.available() > 0) {
      char inChar = (char)gps.read();
      gpsRespString += inChar;
      gpsRespCharCnt++;

      if (gpsRespCharCnt < 7) {
        if (gpsRespString[gpsRespCharCnt - 1] != gpsRefString[gpsRespCharCnt - 1]) {
          gpsRespCharCnt = 0;
          gpsRespString = "";
        }
      }

      if (inChar == '\r') {
        if (gpsRespCharCnt > 65) {
          gps_status = 1;
          break;
        } else {
          gpsRespCharCnt = 0;
        }
      }
    }
    if (gps_status) break;
  }
}

// Initialize the GSM module with the required AT commands
void gsm_init() {
  lcd.clear();
  lcd.print("Finding Module");

  bool at_flag = 1;
  while (at_flag) {
    Serial.println("AT");
    while (Serial.available() > 0) {
      if (Serial.find("OK")) at_flag = 0;
    }
    delay(1000);
  }

  lcd.clear();
  lcd.print("Module connected");
  delay(1000);

  bool echo_flag = 1;
  while (echo_flag) {
    Serial.println("ATE0");
    while (Serial.available() > 0) {
      if (Serial.find("OK")) echo_flag = 0;
    }
    delay(1000);
  }

  lcd.clear();
  lcd.print("Finding Network");

  bool at_cmfg_flag = 1;
  while (at_cmfg_flag) {
    Serial.println("AT+CMGF=1");
    while (Serial.available() > 0) {
      if (Serial.find("OK")) at_cmfg_flag = 0;
    }
    delay(1000);
  }

  bool at_cnmi_flag = 1;
  while (at_cnmi_flag) {
    Serial.println("AT+CNMI=2,2,0,0");
    while (Serial.available() > 0) {
      if (Serial.find("OK")) at_cnmi_flag = 0;
    }
    delay(1000);
  }

  bool at_csmp_flag = 1;
  while (at_csmp_flag) {
    Serial.println("AT+CSMP=17,167,0,0");
    while (Serial.available() > 0) {
      if (Serial.find("OK")) at_csmp_flag = 0;
    }
    delay(1000);
  }

  bool net_flag = 1;
  while (net_flag) {
    Serial.println("AT+CPIN?");
    while (Serial.available() > 0) {
      if (Serial.find("+CPIN: READY")) net_flag = 0;
    }
    delay(1000);
  }

  lcd.clear();
  lcd.print("Network found..");
  delay(1000);
  lcd.clear();
}

// Parse the GPS sentence and extract latitude/longitude
void get_gps() {
  gps_status = 0;
  int x = 0;
  while (gps_status == 0) {
    gpsEvent();
    int str_length = gpsRespCharCnt;
    latitude = "";
    longitude = "";
    int comma = 0;

    while (x < str_length) {
      if (gpsRespString[x] == ',') comma++;
      if (comma == 2) latitude += gpsRespString[x + 1];
      else if (comma == 4) longitude += gpsRespString[x + 1];
      x++;
    }

    int l1 = latitude.length();
    latitude[l1 - 1] = ' ';
    l1 = longitude.length();
    longitude[l1 - 1] = ' ';

    lcd.clear();
    lcd.print("Lat: ");
    lcd.print(latitude);
    lcd.setCursor(0, 1);
    lcd.print("Long: ");
    lcd.print(longitude);

    gpsRespCharCnt = 0;
    x = 0;
    str_length = 0;
    delay(1000);
  }
}

// Prepare the GSM module to send an SMS to a given contact
void init_sms(int mob_cnt) {
  bool at_cmfg_flag = 1;
  while (at_cmfg_flag) {
    Serial.println("AT+CMGF=1");
    while (Serial.available() > 0) {
      if (Serial.find("OK")) at_cmfg_flag = 0;
    }
    delay(1000);
  }

  bool at_cmgs_flag = 1;
  String number = "+91XXXXXXXXXX";  // <-- replace with an emergency contact number
  while (at_cmgs_flag) {
    Serial.print("AT+CMGS=\"");
    Serial.print(number);
    Serial.println("\"");
    while (Serial.available() > 0) {
      if (Serial.find(">")) at_cmgs_flag = 0;
    }
    delay(1000);
  }
}

// Send the SMS body (Ctrl+Z terminator)
void send_sms() {
  Serial.write((char)26);
  Serial.write(26);
}

// Update the LCD after sending an SMS
void lcd_status() {
  lcd.clear();
  lcd.print("Message sent");
  delay(1000);
  lcd.clear();
  lcd.print("System ready");
}

// Full emergency flow: buzzer on, compose + send SMS, update display
void tracking(int mob_cnt) {
  digitalWrite(BUZZER_PIN, HIGH);
  init_sms(mob_cnt);

  Serial.print("Emergency! lat: ");
  Serial.print(latitude);
  Serial.print(" long: ");
  Serial.print(longitude);

  send_sms();
  delay(2000);
  lcd_status();
}
