/*-(Import required libraries)-*/
#include <Time.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <LowPower.h> // source: https://github.com/rocketscream/Low-Power
#include <DS3232RTC.h> //source: https://github.com/JChristensen/DS3232RTC 

/*-( Declare Constants )-*/
#define ECHO_TO_SERIAL 1 // echo data to serial port for SD card reader
const int wakeUpPin = 2;
#define greenLEDpin 3
#define redLEDpin 4
#define RELAY_1 5
#define DHT1PIN 6 //DHT22 humidity and temperature sensor on digital pin 6 
#define DHT2PIN 7 //DHT22 humidity and temperature sensor on digital pin 7
#define ONE_WIRE_BUS 8 // All thermocouples using 1-Wire protocol on digital pin 8
#define RELAY_2 9
const int chipSelect = 10;
#define DHTTYPE DHT22
#define RELAY_ON HIGH
#define RELAY_OFF LOW
#define resolution 5

//*-( Declare objects )-*
DHT dht1(DHT1PIN, DHTTYPE);
DHT dht2(DHT2PIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//hardware address of each thermocouple
byte tc1[8] = {0x3B, 0x1A, 0x30, 0x18, 0x0, 0x0, 0x0, 0x96}; //#1
byte tc2[8] = {0x3B, 0x41, 0x2D, 0x18, 0x0, 0x0, 0x0, 0xD0}; //#2
byte tc3[8] = {0x3B, 0xF2, 0x32, 0x18, 0x0, 0x0, 0x0, 0x9D}; //#3
byte tc4[8] = {0x3B, 0x51, 0x2D, 0x18, 0x0, 0x0, 0x0, 0x8B}; //#4
byte tc5[8] = {0x3B, 0xC0, 0x27, 0x18, 0x0, 0x0, 0x0, 0xB0}; //#5
byte tc6[8] = {0x3B, 0xE0, 0x27, 0x18, 0x0, 0x0, 0x0, 0x06}; //#6
byte tc7[8] = {0x3B, 0x05, 0x2E, 0x18, 0x0, 0x0, 0x0, 0x37}; //#7
byte tc8[8] = {0x3B, 0xC4, 0x2F, 0x18, 0x0, 0x0, 0x0, 0x52}; //#8
byte tc9[8] = {0x3B, 0x07, 0x32, 0x18, 0x0, 0x0, 0x0, 0x04}; //#9
byte tc10[8] = {0x3B, 0xD3, 0x2B, 0x18, 0x0, 0x0, 0x0, 0x93}; //#10
byte tc11[8] = {0x3B, 0x47, 0x2B, 0x18, 0x0, 0x0, 0x0, 0xFE}; //#11
byte tc12[8] = {0x3B, 0xA2, 0x2B, 0x18, 0x0, 0x0, 0x0, 0x3C}; //#12
byte tc13[8] = {0x3B, 0x2F, 0x30, 0x18, 0x0, 0x0, 0x0, 0x90}; //#13
byte tc14[8] = {0x3B, 0xC7, 0x2F, 0x18, 0x0, 0x0, 0x0, 0x0B}; //#14
byte tc15[8] = {0x3B, 0x12, 0x2C, 0x18, 0x0, 0x0, 0x0, 0x6A}; //#15
byte tc16[8] = {0x3B, 0x6E, 0x6F, 0x18, 0x0, 0x0, 0x0, 0x28}; //#16
byte tc18[8] = {0x3B, 0x31, 0x76, 0x18, 0x0, 0x0, 0x0, 0xAD}; //#18
byte tc19[8] = {0x3B, 0x27, 0x32, 0x18, 0x0, 0x0, 0x0, 0xB2}; //#19
byte tc20[8] = {0x3B, 0xB2, 0x6C, 0x18, 0x0, 0x0, 0x0, 0xDF}; //#20
byte tc21[8] = {0x3B, 0x35, 0x76, 0x18, 0x0, 0x0, 0x0, 0x71}; //#21
byte tc22[8] = {0x3B, 0x19, 0x6C, 0x18, 0x0, 0x0, 0x0, 0x7B}; //#22
byte tc23[8] = {0x3B, 0x97, 0x76, 0x18, 0x0, 0x0, 0x0, 0x43}; //#23
//SD CARD
File logfile;

/*-(Declare Variables)-*/
time_t t;
int nextLoop;
int Fan_1;
int Fan_2;

/****** SETUP: RUNS ONCE ******/
void setup()
{
  Serial.begin(9600);
  Serial.println("Relays off to start");
  //(set pins as outputs )
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  //( Initialize Pins so relays are inactive at reset)
  digitalWrite(RELAY_1, RELAY_OFF);
  digitalWrite(RELAY_2, RELAY_OFF);
  Fan_1 = 0;
  Fan_2 = 0;

  Serial.println();
  Wire.begin();
  setSyncProvider(RTC.get);
  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT);
  pinMode(wakeUpPin, INPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  //define filename
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    Serial.println(filename);
    if (!SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }
  if (!logfile) {
    error("could not create file");
  }
  t = RTC.get();
    Serial.print("Logging to: ");
  Serial.println(filename);

logfile.println("date,hour,min,sec,tc-1,tc-2,tc-3,tc-4,tc-5,tc-6,tc-7,tc-8,tc-9,tc-10,tc-11,tc-12,tc-13,tc-14,tc-15,tc-16,tc-18,tc-19,tc-20,tc-21,tc-22,tc-23,dht-1h,dht-1t,dht-2h,dht-2t,Fan_1, Fan_2");
#if ECHO_TO_SERIAL
  Serial.println("date,hour,min,sec,tc-1,tc-2,tc-3,tc-4,tc-5,tc-6,tc-7,tc-8,tc-9,tc-10,tc-11,tc-12,tc-13,tc-14,tc-15,tc-16,tc-18,tc-19,tc-20,tc-21,tc-22,tc-23,dht-1h,dht-1t,dht-2h,dht-2t,Fan_1, Fan_2");
#endif

  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

  nextLoop = ((minute(RTC.get()) / resolution) + 1) * resolution;
  if (nextLoop >= 60) {
    nextLoop -= 60;
  }
  Serial.print(RTC.temperature());
  Serial.print(",");
  Serial.print(RTC.oscStopped());
  Serial.print(",");
  Serial.println(nextLoop);

  sensors.begin(); 
  dht1.begin();
  dht2.begin();
}

/****** LOOP: RUNS CONSTANTLY ******/
void loop()
{
  t = RTC.get();
  sensors.requestTemperatures(); // Send the command to get temperatures
  digitalWrite(greenLEDpin, HIGH);
  logfile.print(year(t), DEC);
  logfile.print("/");
  logfile.print(month(t), DEC);
  logfile.print("/");
  logfile.print(day(t), DEC);
  logfile.print(",");
  logfile.print(hour(t), DEC);
  logfile.print(",");
  logfile.print(minute(t), DEC);
  logfile.print(",");
  logfile.print(second(t), DEC);
  logfile.print(",");
  logfile.print(sensors.getTempC(tc1)); logfile.print(",");
  logfile.print(sensors.getTempC(tc2)); logfile.print(",");
  logfile.print(sensors.getTempC(tc3)); logfile.print(",");
  logfile.print(sensors.getTempC(tc4)); logfile.print(",");
  logfile.print(sensors.getTempC(tc5)); logfile.print(",");
  logfile.print(sensors.getTempC(tc6)); logfile.print(",");
  logfile.print(sensors.getTempC(tc7)); logfile.print(",");
  logfile.print(sensors.getTempC(tc8)); logfile.print(",");
  logfile.print(sensors.getTempC(tc9)); logfile.print(",");
  logfile.print(sensors.getTempC(tc10)); logfile.print(",");
  logfile.print(sensors.getTempC(tc11)); logfile.print(",");
  logfile.print(sensors.getTempC(tc12)); logfile.print(",");
  logfile.print(sensors.getTempC(tc13)); logfile.print(",");
  logfile.print(sensors.getTempC(tc14)); logfile.print(",");
  logfile.print(sensors.getTempC(tc15)); logfile.print(",");
  logfile.print(sensors.getTempC(tc16)); logfile.print(",");
  logfile.print(sensors.getTempC(tc18)); logfile.print(",");
  logfile.print(sensors.getTempC(tc19)); logfile.print(",");
  logfile.print(sensors.getTempC(tc20)); logfile.print(",");
  logfile.print(sensors.getTempC(tc21)); logfile.print(",");
  logfile.print(sensors.getTempC(tc22)); logfile.print(",");
  logfile.print(sensors.getTempC(tc23)); logfile.print(",");
  logfile.print(dht1.readHumidity()); logfile.print(",");
  logfile.print(dht1.readTemperature()); logfile.print(",");
  logfile.print(dht2.readHumidity()); logfile.print(",");
  logfile.print(dht2.readTemperature()); logfile.print(",");
  if ((sensors.getTempC(tc13)) >= 15) {
    digitalWrite(RELAY_1, RELAY_ON);
    Serial.println("Fan 1 is on");
    Fan_1 = 1;
  }
  else {
    digitalWrite(RELAY_1, RELAY_OFF);
    Serial.println("Fan 1 is off");
    Fan_1 = 0;
  }
  if ((sensors.getTempC(tc13)) >= 20) {
    digitalWrite(RELAY_2, RELAY_ON);
    Serial.println("Fan 2 is on");
    Fan_2 = 1;
  }
  else {
    digitalWrite(RELAY_2, RELAY_OFF);
    Serial.println("Fan 2 is off");
    Fan_2 = 0;
  }
  logfile.print(Fan_1, DEC);
  logfile.print(",");
  logfile.print(Fan_2, DEC);
  logfile.println();

#if ECHO_TO_SERIAL
  Serial.print(year(t), DEC);
  Serial.print("/");
  Serial.print(month(t), DEC);
  Serial.print("/");
  Serial.print(day(t), DEC);
  Serial.print(",");
  Serial.print(hour(t), DEC);
  Serial.print(",");
  Serial.print(minute(t), DEC);
  Serial.print(",");
  Serial.print(second(t), DEC);
  Serial.print(",");
  Serial.print(sensors.getTempC(tc1)); Serial.print(",");
  Serial.print(sensors.getTempC(tc2)); Serial.print(",");
  Serial.print(sensors.getTempC(tc3)); Serial.print(",");
  Serial.print(sensors.getTempC(tc4)); Serial.print(",");
  Serial.print(sensors.getTempC(tc5)); Serial.print(",");
  Serial.print(sensors.getTempC(tc6)); Serial.print(",");
  Serial.print(sensors.getTempC(tc7)); Serial.print(",");
  Serial.print(sensors.getTempC(tc8)); Serial.print(",");
  Serial.print(sensors.getTempC(tc9)); Serial.print(",");
  Serial.print(sensors.getTempC(tc10)); Serial.print(",");
  Serial.print(sensors.getTempC(tc11)); Serial.print(",");
  Serial.print(sensors.getTempC(tc12)); Serial.print(",");
  Serial.print(sensors.getTempC(tc13)); Serial.print(",");
  Serial.print(sensors.getTempC(tc14)); Serial.print(",");
  Serial.print(sensors.getTempC(tc15)); Serial.print(",");
  Serial.print(sensors.getTempC(tc16)); Serial.print(",");
  Serial.print(sensors.getTempC(tc18)); Serial.print(",");
  Serial.print(sensors.getTempC(tc19)); Serial.print(",");
  Serial.print(sensors.getTempC(tc20)); Serial.print(",");
  Serial.print(sensors.getTempC(tc21)); Serial.print(",");
  Serial.print(sensors.getTempC(tc22)); Serial.print(",");
  Serial.print(sensors.getTempC(tc23)); Serial.print(",");
  Serial.print(dht1.readHumidity()); Serial.print(",");
  Serial.print(dht1.readTemperature()); Serial.print(",");
  Serial.print(dht2.readHumidity()); Serial.print(",");
  Serial.print(dht2.readTemperature()); Serial.print(",");
  Serial.print(Fan_1, DEC);  Serial.print(",");
  Serial.print(Fan_2, DEC);
  Serial.println();
#endif //ECHO_TO_SERIAL

  digitalWrite(greenLEDpin, LOW);

  delay(1000);
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);

  delay(1000);
  goToSleep();
}
void goToSleep(void) {
  RTC.setAlarm(ALM2_MATCH_MINUTES, nextLoop, 1, 1);
  RTC.alarmInterrupt(ALARM_2, true);
  attachInterrupt(0, wakeUp, LOW);

  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  detachInterrupt(0);
  RTC.alarm(ALARM_2);
  nextLoop += resolution;
  if (nextLoop >= 60) {
    nextLoop -= 60;
  }
}
void wakeUp()
  // Handler for the pin interrupt.
void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);
  while (1);
}

