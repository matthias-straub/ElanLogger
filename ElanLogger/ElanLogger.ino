#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
// make sure to use old SdFat library v1.0.3 
#include <SdFat.h> 
#include <SdFatConfig.h>
#include <SysCall.h>

// Lotus Elan SE / M100 ALDL data logger to ElanScan format
// (w) in 2017 for Elan owners by Matthias Straub
// provided as is - use at your own risk
// Version 1.12, 2.10.2021, stability improvement, better error tolerance and prevent overflow of serbuf, reduced frequency to 5 datasets per second, do not sync fs every dataset
// Version 1.1, 5.7.2017, added LED signalling (1s interval -> success, 200ms interval -> fail)
// Version 1.0, 4.7.2017, initial version
//
// Interface needed: 
//   tx   -> 100 ohms resistor  -> ALDL G
//   rx   -> 100 ohms resistor  -> ALDL G
//   gnd  -> diode              -> ALDL G
//   gnd  ->                    -> ALDL A
//   V in ->                    -> ALDL F
//
// important: increase SERIAL_RX_BUFFER_SIZE in HardwareSerial.h from 64 to 128 first to hold all mode 1 data
//

#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include "SdFat.h"

struct DBL
{
unsigned long f:29; // filler
unsigned long m:23;
unsigned int e:11;
unsigned int s:1;
}
dbl;

union FLTCONV
{
  struct
  {
unsigned long m:23;
unsigned int e:8;
unsigned int s:1;
  }
  p;
  float f;
}
flt;

union DBLCONV
{
  struct DBL p;
  byte b[8];
}
da;

void float2DA(float number)
{
  flt.f = number;
  da.p.s = flt.p.s;
  da.p.e = flt.p.e-127 +1023;
  da.p.m = flt.p.m;
}


RTC_DS1307 rtc;
 
String filename = "";
String finalname = "";
char fname[13];
char finname[13];
unsigned char serbuf[100];
int sercount;
float runtime,starttime;
long packetcount;
int missedcount;
int synccounter;

const uint8_t chipSelect = SS;

SdFat sd;
SdFile datei;
#define error(msg) sd.errorHalt(F(msg))

void ended(int pause) {
  SPI.end();
  pinMode(LED_BUILTIN, OUTPUT);
  while (true){
   digitalWrite(LED_BUILTIN, HIGH);
   delay(pause);
   digitalWrite(LED_BUILTIN, LOW);
   delay(pause);
  }
}


void setup() {
  synccounter=0;
  char zero=0;
  char one=1;
  char fourtyseven=0x47;
  
  packetcount=0;
  missedcount=0;
  Serial.begin(8192);
   
  if (! rtc.begin()) {
    ended(100);
  }
  if (! rtc.isrunning()) {
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // set initial time from host PC
  }
  
  DateTime now = rtc.now();

  filename=String(now.year()%10)+String(now.month()%10);
  if (now.day()<10) filename+="0";
  filename+=String(now.day());
  if (now.hour()<10) filename+="0";
  filename+=String(now.hour());
  if (now.minute()<10) filename+="0";
  filename+=String(now.minute());
  finalname=filename+".ecu";
  filename=filename+".ec_";
  filename.toCharArray(fname,13);
  finalname.toCharArray(finname,13);
    
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) { 
     //sd.initErrorHalt();
     ended(100);
  }
  if (datei.open(fname, O_CREAT | O_WRITE | O_EXCL)) {
    
    //write elanscan header
    for (int i=0;i<4;i++) datei.write(&zero,1);
    datei.write(&one,1);
    for (int i=0;i<260;i++) datei.write(&zero,1);
    datei.write(&fourtyseven,1);
    for (int i=0;i<7;i++) datei.write(&zero,1);
    
    if (!datei.sync() || datei.getWriteError()) {
     error("WriteError!");
     ended(100);
    }
  }
  sercount=0;
  starttime=(float)millis()/1000.0;
}

void loop() {
  unsigned char command[]={0xF4,0x56,0x01,0xB5};  // mode 1 request with check sum
  unsigned char checksum;
  unsigned long looptime;
  int i;
  looptime=millis();
  runtime=((float)looptime/1000.0)-starttime;
  Serial.flush();                               // From Ron - Wait for empty send buffer
  Serial.write(command,4);                      // Send Command to ECM 
  Serial.flush();                               // Wait for command to be sent to the ECM
  
  checksum=0;
  for (int i = 4; i < sercount-1; i++){
      checksum+=serbuf[i];
  }
  if (sercount==71) {  // only ecu replies with correct size
   if ((unsigned char)(0x100-checksum)==serbuf[sercount-1]) {   // checksum okay
    for (i = 7; i < sercount-1; i++){
        datei.write(serbuf[i]);
    }
    float2DA(runtime);
    for (i=0; i<8; i++)
    {
     datei.write(da.b[i]);
    }
    packetcount++;
    missedcount=0;
    synccounter++;
    if (synccounter==100) { // sync SD only every 100 packets
     datei.sync();
     synccounter=0;
    }
   }
  }
  else {
    missedcount++;
    if (missedcount>10) {  // quit after 10 missed replies
     datei.sync();
     datei.seekSet(0);
     long sizefile=71*packetcount;
     datei.write((char *)&sizefile,4);
     datei.seekSet(269);
     datei.write((char *)&packetcount,4);
     datei.sync(); // to make sure
     datei.close(); 
     sd.rename(fname,finname);
     delay(1000);
     ended(1000);
    }
  }
  sercount=0;

  int timeleft=200-(millis()-looptime);
  if (timeleft>0) delay(timeleft);
  
  while (Serial.available()) {
    unsigned char inChar = (unsigned char)Serial.read();
    serbuf[sercount]= inChar;
    sercount++;
    if (sercount>90) { // prevent buffer from overflowing
      sercount=90;
    }
  }
}

