#include <SoftPWM_timer.h>
#include <SoftPWM.h>

#include <SD.h>
#include <LiquidCrystal.h>
#include "lcd.h"
#include "weigand.h"

#define LCD_BLUE A4
#define LCD_GREEN A3
#define LCD_RED A2
#define LCD_RS A0
#define LCD_ENABLE A1
#define LCD_D4 8
#define LCD_D5 7
#define LCD_D6 6
#define LCD_D7 5

#define RFID_D0 2
#define RFID_D1 3

#define SD_CS 10

//AltSoftSerial BLEMini;

extern uint8_t* __brkval;
extern uint8_t __heap_start;
int freeRam () {
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

Weigand w(RFID_D0, RFID_D1);
LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

boolean startupError = false;



void setLCDColor(lcdstatus s) {
  switch(s) {
  case error: 
    lcdWriteColor(0xff, 0x00, 0x00);
    break;   
  case warning: 
    lcdWriteColor(0xff, 0x22, 0x00);
    break;    
  case info: 
    lcdWriteColor(0x00, 0xff, 0xff);
    break;    
  case green:
    lcdWriteColor(0x00, 0xff, 0x00);
    break;
  default: 
    lcdWriteColor(0xff, 0xff, 0xff);
  }
}

void lcdRainbow() {
  unsigned int rgbColor[3];
   
  // Start off with red.
  rgbColor[0] = 255;
  rgbColor[1] = 0;
  rgbColor[2] = 0;
   
  // Choose the colours to increment and decrement.
  for (int decColor = 0; decColor < 3; decColor += 1) {
    int incColor = decColor == 2 ? 0 : decColor + 1;
     
    // cross-fade the two colours.
    for(int i = 0; i < 255; i += 1) {
      rgbColor[decColor] -= 1;
      rgbColor[incColor] += 1;
      SoftPWMSet(LCD_RED, rgbColor[0]); 
      SoftPWMSet(LCD_GREEN, rgbColor[1]);
      SoftPWMSet(LCD_BLUE, rgbColor[2]);
      delay(5);
    }
  }
}


void lcdWriteColor(int r, int g, int b) {
  SoftPWMSet(LCD_RED, map(r, 0, 255, 255, 0)); 
  SoftPWMSet(LCD_GREEN, map(g, 0, 255, 255, 0));
  SoftPWMSet(LCD_BLUE, map(b, 0, 255, 255, 0));
}

void lcdWriteLine1(char text[]) {
  lcd.setCursor(0, 0);
  lcd.print(text); 
}

void lcdWriteLine2(char text[]) {
  lcd.setCursor(0, 1);
  lcd.print(text); 
}

void lcdWriteCard(unsigned int bits, uint64_t card) {
  lcdClear();
  
  lcd.setCursor(0, 0); 
  lcd.print(F("Size: "));
  lcd.print(bits);
  
  lcd.setCursor(0, 1);
  lcd.print(card, HEX);
}

void lcdClear() {
  lcd.clear(); 
}






void readD0() {
  w.add0();
}  

void readD1() {
  w.add1();
}

void setup() {
  Serial.begin(57600);
  Serial.println(F("[*] Setting up..."));
  
  lcd.begin(16, 2);
  lcdClear();
  lcdWriteLine1("Starting up...");
   
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  SoftPWMBegin();

  attachInterrupt(0, readD0, FALLING);
  attachInterrupt(1, readD1, FALLING);

  w.resetdata();

  pinMode(SD_CS, OUTPUT);
  if(!SD.begin(SD_CS)) {
    Serial.println("[!] SD card failed or not present.");
    setLCDColor(error);
    lcdClear();
    lcdWriteLine1("No SD Card Found");
    startupError = true;
  }
  
  if(startupError) {
    delay(5000); //Give our users a chance to read the error instead of coloring the rainbow.
  } else {
    lcdRainbow(); // Wait for a little bit to let the data lines settle.  Since we've got time, lets show off.
  }
  
  lcdClear();  
  Serial.println("[*] Setup complete.");
}

void loop() {
  if(w.loop()) {
     Serial.println("");
     setLCDColor(green);
     uint64_t card = w.parsecard();
     lcdWriteCard(w.getcount(), w.parsecard());

    
    Serial.print("[*] Got data: '");
    Serial.print(w.getcount());
    Serial.print(",");
    Serial.print(card, BIN);
    Serial.print(",");
    Serial.print(card, HEX);
    Serial.println("'");

     delay(3000);
     
     sdWriteCard();
     sendBluetooth();

     delay(2000);
     w.resetdata();
     lcdClear();
   } else {
     setLCDColor(info);
     lcdWriteLine1("Hunting...");
     lcd.setCursor(0,1);
     lcd.print(freeRam());
     lcd.print(" b");
   } 
}

void sdWriteCard() {
  File csvFile = SD.open("cards.csv", FILE_WRITE);
  if(csvFile) {
    uint64_t card = w.parsecard();
    csvFile.print(w.getcount());
    csvFile.print(",");
    csvFile.print(card, BIN);
    csvFile.print(",");
    csvFile.print(card, HEX);
    csvFile.println("");
    csvFile.close();
    
    Serial.print("[*] Got data: '");
    Serial.print(w.getcount());
    Serial.print(",");
    Serial.print(card, BIN);
    Serial.print(",");
    Serial.print(card, HEX);
    Serial.println("'");
  } else {
    lcdClear();
    setLCDColor(error);
    lcdWriteLine1("Error writing.");
    lcdWriteLine2("to file."); 
  }
}

void sendBluetooth() {
  uint64_t card = w.parsecard();
  //if (BLEMini.available()) {
  //  char buf[128];
  //  sprintf(buf, "%llu", (unsigned long long) card);	
  //  int len = strlen(buf);

   // BLEMini.write();
   // for (int i = 0; i < len; i++) {
   //   BLEMini.write(buf[i]);
   // }
  }


 
