/*
 Chronograph with SD recording.
 Time Keeper v2.3
 based on SD Example.
 by 2014 A.Fujimoto.
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
 ++ SW_stop_timekeep  0
 */
#define hardwareSS 10  // don't touch it!
#define CS 8 // you can change this except 10.
#include <SD.h>
#include <LiquidCrystal.h>
#include <avr/io.h>
#include <avr/interrupt.h>
File myFile;
const int sw = 9;
const int owari = 0;
const int dTime = 999;
int val = 0;
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
int dTime2 = 17;
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
// 4bitMode  (rs, rw, d4, d5, d6, d7)
void setup()
{
  lcd.begin(16,2);
  lcd.noBlink();
  pinMode(sw, INPUT_PULLUP);  // time record sw.
  pinMode(owari, INPUT_PULLUP);  // time record stop sw.
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
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
void loop()
{
  // nothing happens after setup
  delay(dTime2);
  delayMicroseconds(dms);
  __asm__("nop");
  curTime = 0;
  curTime = millis();
  sSec += 3;
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
  if(digitalRead(owari) == LOW){  // negative logic.
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
  Serial.println(millis()-curTime);
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
  lcd.setCursor(0,0);
  lcd.clear();
  //  lcd.print("Delay :");
  //  lcd.setCursor(0,1);
  //  lcd.print("Time :");
  lcd.setCursor(5,1);
  lcd.print(hour);
  if(second%2 == 0){
    lcd.print(":");
  }
  else{
    lcd.print(" ");
  } 
  lcd.print(mPad);
  lcd.print(minute);
  lcd.print(".");
  lcd.print(sPad);
  lcd.print(second);
  lcd.print(sPad);
  lcd.print(sSec);
  lcd.setCursor(7,0);
  processTime = millis() - curTime;
  lcd.print(processTime);
  lcd.print("ms");
  while(iter<time){
    keycheck();
    iter++;
  }
  iter=0;
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
  if(digitalRead(owari) == LOW){  // negative logic.
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
