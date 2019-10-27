#include <avr/wdt.h>
#include <RF24.h>
#include <SPI.h>
#include <TimerOne.h>
#include <math.h>
#include <Adafruit_AM2315.h>
#include <Adafruit_BMP280.h>

#define RADIO_CE_PIN 8 //RF24 chip enable pin
#define RADIO_CS_PIN 9 //RF24 chip select pin
#define WindSensorPin (2) //pin for WindSpeedSensor
#define BMP_SCK 13 //defines settings for BMP280 Barometric Sensor
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10
#define TippingBucketPin (3)//pin for Tipping Bucket Sensor

//defines settings for RF24ghz Transceiver
RF24 radio = RF24(RADIO_CE_PIN, RADIO_CS_PIN); //Define Radio (CE-PIN,CSN-PIN)
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

Adafruit_BMP280 bme;
Adafruit_AM2315 am2315;

//time between transmissions
const int tranmissionDelayTime = 2500;
bool requestString = true;

int sizeWeather;  //used to define max buffer size of wireless transmission

volatile float windspeed; //wind speed in km
volatile bool IsSampleRequired; // this is set true every 2.5s. Get wind speed
volatile unsigned long Rotations = 0; // cup rotation counter used in interrupt routine
volatile unsigned long ContactBounceTime; // Timer to avoid contact bounce in isr
volatile unsigned int TimerCount = 0; // used to determine 2.5sec timer count
volatile int tippingBucketTips = 0;   //number of times the buckets has tipped

const int windVaneSampleSize = 16;
long newWindVaneTime;
long lastWindVaneTime = 0;
long windVaneCompare;
String windVaneDirection = "";
volatile char wVD[4];
int count;
int arrayCounter;
bool RESETCOUNTER;

void setup() {

  Serial.begin(9600);

  PCICR |= (1 << PCIE2);
  //Enables Interrupt on Pin 9
  pinMode(7, INPUT_PULLUP);
  PCMSK2 |= (1 << PCINT23);

  //configure interrupt for tipping bucket scale
  pinMode(TippingBucketPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TippingBucketPin), rainISR, FALLING);
  tippingBucketTips = 0;

  //configure interrupt for wind speed
  pinMode(WindSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), windISR, FALLING);

  Timer1.initialize(500000);// Timer interrupt every 2.5 seconds
  Timer1.attachInterrupt(windISR_timer);

  lastWindVaneTime = micros();

  enableBMP180();
  enableAM2315();
  
  Serial.println("Beginning 2.4ghz Radio transmitter configuration.... ");
  configureRadio();

  Serial.println("Startup complete, transmission will begin shortly...");

}

void loop() {

  delay(tranmissionDelayTime);

  getWindSpeed();
  getWindDirection();

  if (windVaneDirection.length() == 4) {
    transmit();

  }

  RESETCOUNTER = false;
  count = 0;

}
