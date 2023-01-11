#include <U8g2lib.h>  //U8g2 Library for the SSD1306 OLED
#include <Arduino.h>
#include <EEPROM.h>
//Define switch pins
#define SW4 4
#define SW3 10
#define SW2 9
#define SW1 8
//Define debug LED Pin
#define ledPin 13
//Define DRV8833 motor driver pins
#define AIN1 5
#define AIN2 6
//Define Global Variables
int Mode = 0;
int Dtime;
int Htime;
int screenrotation = 0;
unsigned long presstime = 0;
// U8g2 Instance for OLED
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R2, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);
void setup() {
  //initialize pins
  pinMode(ledPin, OUTPUT);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  //set motor driver to off during startup
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  //Get saved data from EEPROM
  Dtime = get_String(0, 1).toInt();
  Htime = get_String(5, 6).toInt();
  screenrotation = EEPROM.read(10);
  //initialize OLED and display the menu for first time
  u8g2.begin();
  u8g2.setFlipMode(screenrotation);
  u8g2.clearBuffer();
  menu(Mode);
  u8g2.sendBuffer();
}


void menu(int n) {
  //Draw main UI
  u8g2.drawLine(0, 21, 128, 21);
  u8g2.drawLine(0, 22, 128, 22);
  u8g2.drawLine(0, 23, 128, 23);
  switch (n) {
    case 0:
      u8g2.setFont(u8g2_font_baby_tf);
      u8g2.setCursor(5, 18);
      u8g2.print("Dispense Time");
      u8g2.drawDisc(32, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(53, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(75, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(96, 28, 3, U8G2_DRAW_ALL);
      u8g2.setFont(u8g2_font_tenthinnerguys_tf);
      u8g2.setCursor(75, 18);
      u8g2.print(Dtime);
      u8g2.print("ms");
      break;
    case 1:
      u8g2.setFont(u8g2_font_baby_tf);
      u8g2.setCursor(23, 14);
      u8g2.print("Continuous Dispense");
      u8g2.drawCircle(32, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawDisc(53, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(75, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(96, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawBox(8, 8, 9, 9);
      u8g2.drawTriangle(110, 7, 110, 16, 118, 12);
      break;
    case 2:
      u8g2.setFont(u8g2_font_baby_tf);
      u8g2.setCursor(22, 14);
      u8g2.print("  Continuous Retract");
      u8g2.drawCircle(32, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(53, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawDisc(75, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(96, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawBox(8, 8, 9, 9);
      u8g2.drawTriangle(110, 7, 110, 16, 118, 12);
      break;
    case 3:
      u8g2.setFont(u8g2_font_baby_tf);
      u8g2.setCursor(5, 18);
      u8g2.print("Retract Time");
      u8g2.drawCircle(32, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(53, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawCircle(75, 28, 3, U8G2_DRAW_ALL);
      u8g2.drawDisc(96, 28, 3, U8G2_DRAW_ALL);
      u8g2.setFont(u8g2_font_tenthinnerguys_tf);
      u8g2.setCursor(75, 18);
      u8g2.print(Htime);
      u8g2.print("ms");
      break;
  }
}


void Motor(int n) {
  //function to drive motor
  if (n == 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
  } else if (n == 1) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  } else if (n == 2) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  }
}

void MotorD() {
  Motor(1);
  delay(Dtime);
  Motor(0);
}
void MotorH() {
  Motor(2);
  delay(Htime);
  Motor(0);
}

void set_String(int a, int b, String str) {
  EEPROM.write(a, str.length());
  for (int i = 0; i < str.length(); i++) {
    EEPROM.write(b + i, str[i]);
  }
}

String get_String(int a, int b) {
  String data = "";
  for (int i = 0; i < EEPROM.read(a); i++) {
    data += char(EEPROM.read(b + i));
  }
  return data;
}

void loop() {

  //check if mode button (SW2) is pressed
  if (digitalRead(SW2) == LOW) {
    presstime = millis();
    delay(20);
    if (digitalRead(SW2) == LOW) {
      Mode++;
      if (Mode > 3) Mode = 0;
      digitalWrite(ledPin, HIGH);
      u8g2.clearBuffer();
      menu(Mode);
      if (Mode == 1 or Mode == 2) u8g2.drawRFrame(5, 5, 15, 15, 5);
      u8g2.sendBuffer();
      Motor(0);
      while (digitalRead(SW2) == LOW) {
        if (millis() - presstime > 3000) {  //Check if the mode button is pressed for 3S. If then flip screen to 180
          screenrotation = !screenrotation;
          EEPROM.write(10, screenrotation);
          u8g2.setFlipMode(screenrotation);
          u8g2.clearBuffer();
          menu(Mode);
          if (Mode == 1 or Mode == 2) u8g2.drawRFrame(5, 5, 15, 15, 5);
          u8g2.sendBuffer();
          Motor(0);
          delay(2000);
        }
      }
      digitalWrite(ledPin, LOW);
    }
  }

  //check if + and - buttons were pressed or not
  switch (Mode) {
    case 0:
      if (digitalRead(SW1) == LOW) {
        delay(20);
        if (digitalRead(SW1) == LOW) {
          Dtime -= 1;
          if (Dtime < 0) Dtime = 0;
          set_String(0, 1, String(Dtime));
          digitalWrite(ledPin, HIGH);
          u8g2.clearBuffer();
          menu(Mode);
          u8g2.sendBuffer();
          Serial.println(Dtime);
          Serial.println(Htime);
          while (digitalRead(SW1) == LOW)
            ;
          digitalWrite(ledPin, LOW);
        }
      }
      if (digitalRead(SW3) == LOW) {
        delay(20);
        if (digitalRead(SW3) == LOW) {
          Dtime += 1;
          if (Dtime > 3000) Dtime = 3000;
          set_String(0, 1, String(Dtime));
          digitalWrite(ledPin, HIGH);
          u8g2.clearBuffer();
          menu(Mode);
          u8g2.sendBuffer();
          Serial.println(Dtime);
          Serial.println(Htime);
          while (digitalRead(SW3) == LOW)
            ;
          digitalWrite(ledPin, LOW);
        }
      }
      break;
    case 1:
      if (digitalRead(SW1) == LOW) {
        delay(20);
        if (digitalRead(SW1) == LOW) {

          digitalWrite(ledPin, HIGH);
          u8g2.clearBuffer();
          menu(Mode);
          u8g2.drawRFrame(5, 5, 15, 15, 5);
          u8g2.sendBuffer();
          Motor(0);
          while (digitalRead(SW1) == LOW)
            ;
          digitalWrite(ledPin, LOW);
        }
      }
      if (digitalRead(SW3) == LOW) {
        delay(20);
        if (digitalRead(SW3) == LOW) {
          u8g2.clearBuffer();
          menu(Mode);
          u8g2.drawRFrame(106, 5, 15, 15, 5);
          u8g2.sendBuffer();
          Motor(1);
          while (digitalRead(SW3) == LOW)
            ;
          digitalWrite(ledPin, LOW);
        }
      }
      break;
    case 2:
      if (digitalRead(SW1) == LOW) {
        delay(20);
        if (digitalRead(SW1) == LOW) {

          digitalWrite(ledPin, HIGH);
          u8g2.clearBuffer();
          menu(Mode);
          u8g2.drawRFrame(5, 5, 15, 15, 5);
          u8g2.sendBuffer();
          Motor(0);
          while (digitalRead(SW1) == LOW)
            ;
          digitalWrite(ledPin, LOW);
        }
      }
      if (digitalRead(SW3) == LOW) {
        delay(20);
        if (digitalRead(SW3) == LOW) {
          u8g2.clearBuffer();
          menu(Mode);
          u8g2.drawRFrame(106, 5, 15, 15, 5);
          u8g2.sendBuffer();
          Motor(2);
          while (digitalRead(SW3) == LOW)
            ;
          digitalWrite(ledPin, LOW);
        }
      }
      break;
    case 3:
      if (digitalRead(SW1) == LOW) {
        delay(20);
        if (digitalRead(SW1) == LOW) {
          Htime -= 1;
          if (Htime < 0) Htime = 0;
          set_String(5, 6, String(Htime));
          digitalWrite(ledPin, HIGH);
          u8g2.clearBuffer();
          menu(Mode);
          u8g2.sendBuffer();
          Serial.println(Dtime);
          Serial.println(Htime);
          while (digitalRead(SW1) == LOW)
            ;
          digitalWrite(ledPin, LOW);
        }
      }
      if (digitalRead(SW3) == LOW) {
        delay(20);
        if (digitalRead(SW3) == LOW) {
          Htime += 1;
          if (Htime > 1000) Htime = 1000;
          set_String(5, 6, String(Htime));
          digitalWrite(ledPin, HIGH);
          u8g2.clearBuffer();
          menu(Mode);
          u8g2.sendBuffer();
          Serial.println(Dtime);
          Serial.println(Htime);
          while (digitalRead(SW3) == LOW)
            ;
          digitalWrite(ledPin, LOW);
        }
      }
      break;
  }

  //Check if dispense button is pressed or not.If pressed start dispensing as per the set speed
  if (digitalRead(SW4) == LOW) {
    delay(20);
    if (digitalRead(SW4) == LOW) {
      digitalWrite(ledPin, HIGH);
      MotorD();
      MotorH();
      while (digitalRead(SW4) == LOW)
        ;
      digitalWrite(ledPin, LOW);
    }
  }
}
