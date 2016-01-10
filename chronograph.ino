


/*
 Chronograph with SD recording.
 Time Keeper v2.3
 based on SD Example.
 by 2014 Akio Fujimoto.
 I appriciate to Arduino team, and its library!
 
 
 The circuit:
 * SD card attached to SPI bus as follows:
 CM  Pin role         Pin No.
 ++ CS                 8
 ++ MOSI              11
 ++ MISO              12
 ++ SCLK              13
 ++ Hardware Reserved 10
 ++ LCD_RS            2
 ++ LCD_RW            GND
 ++ LCD_Enable        3
 ++ DataBit4..7       4..7
 ++ SW_record_time    9
 !+ SW_stop_timekeep  0
 ++ SW_start          A0
 ++ SW_pause          A1
 ++ SW_reset_time     A2
 
 Warning! can't use Serial functions.
 */

#define hardwareSS 10  // don't touch it!
#define CS 8 // you can change this except 10.
#include <SD.h>
#include <KanaLiquidCrystal.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/io.h>
#include <avr/interrupt.h>

File myFile;
const int sw = 9;
const int sw_owari = 0;
const int sw_start = 14;  //  A0
const int sw_pause = 15;  //  A1
const int sw_reset = 16;  //  A2
const int timeDelta = 3;  //    3/100 second resolution.
const int dTime = 999;

int state = 0;
int oldState = 0;
int state2 = 0;
int oldState2 = 0;


int val = 0;
int oldVal;
int hour = 0;
int minute = 0;
int second = 0;
int sSec = 0;
int curTime = 0;
int processTime = 0;
char hPad = ' ';
char mPad = ' ';
char sPad = ' ';
int iter = 0;  // variable for iteration.
int time = 100; // key scan iterations. Entire loop must be 1 second.
//int dms = 268;  // delay in usec. used in keyscan iteration.
//int dms = 259;  // okureteru
// revised algorhythm. "(big wait)delay -> (bit wait)delayMicroseconds"
int dms = 787;  // inc. to slow. dec. to fast.
//int dms = 793 // accurate in loop range is between 7-8ms.
int dTime2 = 17;

int eeAddress = 0;
int eeValue = dms;
KanaLiquidCrystal lcd(2, 3, 4, 5, 6, 7);

byte gaiji_pause[8] ={
  B11011,
  B11011,
  B11011,
  B11011,
  B11011,
  B11011,
  B11011,
};

byte gaiji_start[8] = {
  B11000,
  B11100,
  B11110,
  B11111,
  B11110,
  B11100,
  B11000,
  B00000,
};
byte gaiji_sd[8] = {
  B11100,
  B10010,
  B10001,
  B10001,
  B10001,
  B11111,
  B00000,
  B00000,
};

// 4bitMode  (rs, rw, d4, d5, d6, d7)

void setup()
{

  delay(1000);
  lcd.createChar(0, gaiji_pause);
  lcd.createChar(1, gaiji_start);
  lcd.createChar(2, gaiji_sd);
  lcd.begin(16,2);
  lcd.kanaOn();

  dms = eeRead(eeAddress);


  //  lcd.noBlink();
  pinMode(sw, INPUT_PULLUP);  // time record sw.
  pinMode(sw_owari, INPUT_PULLUP);  // time record stop sw.
  pinMode(sw_start, INPUT_PULLUP);  // chrono start sw.
  pinMode(sw_pause, INPUT_PULLUP);  // chrono pause sw.
  pinMode(sw_reset, INPUT_PULLUP);  // chrono reset sw.
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  /*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  */
  Serial.println(dms);
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(hardwareSS, OUTPUT);

  if (!SD.begin(CS)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    if(digitalRead(sw) == LOW){
      myFile.println(time);
    }
    myFile.println("testing 1, 2, 3.");
    // close the file:

    Serial.println("done.");
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    //    myFile.close();
    lcd.clear();
    lcd.print(F("ｽﾀｰﾄ ﾎﾞﾀﾝｦ ｵｽﾄ"));
    lcd.setCursor(0,1);
    lcd.print(F("ｹｲｿｸ ｦ ｶｲｼ ｼﾏｽ"));
    lcd.setCursor(15,0);
    lcd.kanaOff();
    lcd.write(byte(2));
    lcd.kanaOn();
    Serial.println("waiting for trigger.");
    //sw_start
    while(1){
      if(digitalRead(sw_start) == LOW)
        break;
    }
    lcd.clear();
    lcd.setCursor(14,0);
    lcd.write(byte(1));  // (gaiji_start);

  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  myFile = SD.open("test.txt", FILE_WRITE);
  while (myFile.available()) {
    myFile.read();
  }


}

void loop(){
  // nothing happens after setup
  delay(dTime2);
  delayMicroseconds(dms);
  __asm__("nop");
  curTime = 0;
  curTime = millis();
  sSec += timeDelta;
  //if(time % 3600 > 0){
  //  hour++;
  //}else if(time%60 > 0){
  //  minute++;
  //  if(minute>=60){minute=0;time++;}
  //}
  //if(minute>=60){hour++;}
  if(minute>=60){
    minute=0;
    hour++;
  }
  if(second>=60){
    second = 0;
    minute++;
  }
  if(sSec>=100){
    sSec=0;
    second++;
  }
  /*

   if(hour/10 == 0){
   hPad = ' ';
   }
   else{
   hPad = ' ';
   }
   if(minute/10 == 0){
   mPad = ' ';
   }
   else{
   mPad = ' ';
   }
   if(second/10 == 0){
   sPad = ' ';
   }
   else{
   sPad = ' ';
   }
   
   */

  // sw_pause, and adjust time.
  state = digitalRead(sw_pause);
  if(state == LOW && oldState == HIGH){
    Serial.println("Paused!");
    lcd.setCursor(14,0);
    lcd.write(byte(0));  // (gaiji_pause);
    delay(10);
    myFile.println(F("PAUSED AT"));
    myFile.println(hour);
    myFile.println(minute);
    myFile.println(second);
    myFile.println(sSec);

    myFile.println();
    while(1){
      lcd.kanaOff();
      lcd.setCursor(14,0);
      lcd.write(byte(0));
      delay(100);
      lcd.kanaOn();
      if(digitalRead(sw_pause)==LOW) {
        eeWrite(eeAddress, eeValue);
        Serial.print("EEPROM ");
        Serial.print(eeAddress);
        Serial.print(", ");
        Serial.println(eeValue);

        break;
      }

      timeAdjust();

    }
    lcd.clear();
  }

  // sw_reset
  state2 = digitalRead(sw_reset);
  if(state2 == LOW && oldState2 == HIGH){
    Serial.println("reset!");
    lcd.home();
    lcd.print(F("                "));
    lcd.setCursor(0,1);
    lcd.print(F("                "));
    hour = 0;
    minute = 0;
    second = 0;
    sSec = 0;
    lcd.home();
  }


  // sw (record time.)
  if(digitalRead(sw) == LOW){  // negative logic.

    Serial.println("button");
    lcd.setCursor(15,0);
    lcd.write(0xFF);
    delay(10);
    myFile.println(hour);
    myFile.println(minute);
    myFile.println(second);
    myFile.println(sSec);
    myFile.println();
    lcd.setCursor(15,0);
    lcd.print(" ");
    //    myFile.close();
  } 
  if(digitalRead(sw_owari) == LOW){  // negative logic.

    Serial.println("End of File.");
    lcd.setCursor(15,0);
    lcd.write(0xFF);
    delay(10);
    myFile.println(hour);
    myFile.println(minute);
    myFile.println(second);
    myFile.println(sSec);

    myFile.println("EOF");
    lcd.setCursor(15,0);
    lcd.print(" ");
    closeFile();
    lcd.clear();

    while(1){
      lcd.print("End of file.");
      lcd.setCursor(0,1);
      lcd.print("Remove SD Card...");
    }
  }

  //  Serial.println(millis()-curTime);
  /*
  Serial.print("Time :");
   Serial.print(hour);
   Serial.print(":");
   Serial.print(mPad);
   Serial.print(minute);
   Serial.print(".");
   Serial.print(sPad);
   Serial.print(second);
   Serial.print(sPad);
   Serial.print(sSec);
   Serial.println();
   
   */

  lcd.setCursor(0,0);
  //  lcd.clear();
  //  lcd.print("Delay :");
  //  lcd.setCursor(0,1);
  //  lcd.print("Time :");
  lcd.setCursor(0,1);
  lcd.print(hour);
  if(second%2 == 0){  // even second.
    lcd.print(" ");
  }
  else{  // odd second.
    lcd.print(":");
  }   
  lcd.print(mPad);
  lcd.print(minute);
  lcd.print(".");
  lcd.print(sPad);
  lcd.print(second);
  lcd.print("' ");
  lcd.print(sSec);
  lcd.print("  ");

  lcd.setCursor(7,0);

  processTime = millis() - curTime;
  lcd.print(processTime);
  lcd.print("ms     ");

  while(iter<time){
    keycheck();
    iter++;
  }
  iter=0;
  oldVal = val;
  oldState = state;
  oldState2 = state2;

}

void closeFile(){
  myFile.close();
}
/*
 char removePad(char s){
 s.erase();
 return s;
 }
  */

void keycheck(){
  if(digitalRead(sw) == LOW){  // negative logic.
    Serial.println("button");
    lcd.setCursor(15,0);
    lcd.write(0xFF);
    delay(10);
    myFile.println(hour);
    myFile.println(minute);
    myFile.println(second);
    myFile.println(sSec);
    myFile.println();
    lcd.setCursor(15,0);
    lcd.print(" ");
    //    myFile.close();
  } 
  if(digitalRead(sw_owari) == LOW){  // negative logic.

    Serial.println("End of File.");
    lcd.setCursor(15,0);
    lcd.write(0xFF);
    delay(10);
    myFile.println(hour);
    myFile.println(minute);
    myFile.println(second);
    myFile.println(sSec);
    myFile.println("EOF");
    lcd.setCursor(15,0);
    lcd.print(" ");
    closeFile();
    lcd.clear();

    while(1){
      lcd.print("End of file.");
      lcd.setCursor(0,1);
      lcd.print("Remove SD Card...");
    }
  }
}

void timeAdjust(){  //in pause menu.
  if(digitalRead(sw_start)==LOW){
    dms--;
  }
  else if(digitalRead(sw_reset)==LOW){
    dms++;
  }
  if(dms<=0) dms = 0;
  lcd.setCursor(10,0);
  lcd.write(0x7E);   //  -> 
  lcd.print(dms);
  eeValue = dms;
  //  lcd.write(0xE4);   // micro
  lcd.print(F("mS"));
}

void eeWrite(int x, int y){
  EEPROM.write(x, y/256); // times 256 counts.
  EEPROM.write(x+1, y%256);  // times <=255 counts.
}
int eeRead(int x){
  volatile int value = EEPROM.read(x)*256 + EEPROM.read(x+1);
  return value;
}
