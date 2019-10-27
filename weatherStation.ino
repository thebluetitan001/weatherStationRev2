#include <avr/wdt.h>
#include <RF24.h>
#include <SPI.h>
#include <TimerOne.h>
#include <math.h>
#include <Adafruit_AM2315.h>
#include <Adafruit_BMP280.h>

#define RADIO_CE_PIN 8 //RF24 chip enable pin
#define RADIO_CS_PIN 9 //RF24 chip select pin

//defines settings for RF24ghz Transceiver
RF24 radio = RF24(RADIO_CE_PIN, RADIO_CS_PIN); //Define Radio (CE-PIN,CSN-PIN)
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

#define WindSensorPin (2) //pin for WindSpeedSensor

#define BMP_SCK 13 //defines settings for BMP280 Barometric Sensor
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10
#define TippingBucketPin (3)//pin for Tipping Bucket Sensor

//time between transmissions
const int tranmissionDelayTime = 2500;
bool requestString = true;
String transmitWeatherString;

int sizeWeather;  //used to define max buffer size of wireless transmission

volatile float windspeed; //wind speed in km
volatile bool IsSampleRequired; // this is set true every 2.5s. Get wind speed

volatile unsigned long Rotations; // cup rotation counter used in interrupt routine
volatile unsigned long ContactBounceTime; // Timer to avoid contact bounce in isr

volatile unsigned int TimerCount; // used to determine 2.5sec timer count

volatile int tippingBucketTips = 0;   //number of times the buckets has tipped

Adafruit_BMP280 bme;

// AM2315 sensor wiring
// Connect RED to 5.0V
// Connect BLACK to Ground
// Connect WHITE to i2c clock to Analog 5
// Connect YELLOW to i2c data to Analog 4

Adafruit_AM2315 am2315;

//wind direction in degrees

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

  lastWindVaneTime = micros();

  //enables driver for am2315 temperature and humidty sensor
  if (! am2315.begin()) {
    Serial.println("AM2315 (Temp/Humidity) Sensor not found, check wiring & pullups!");
    //while (1);
  }


  // delay(500);
  if (!bme.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    //while (1);
  }

  //Serial.println("Beginning 2.4ghz Radio transmitter configuration.... ");

  radio.begin();
  radio.setPALevel(RF24_PA_MAX); // Transmit Power (MAX,HIGH,LOW,MIN)
  //Decimal 76 is 0x4c in hexidecimal confirm that you have configured correctly on both sides
  radio.setChannel(76);
  radio.setDataRate( RF24_1MBPS ); //Transmit Speeed (250 Kbits)

  radio.enableDynamicPayloads();
  radio.openWritingPipe(pipes[0]);
  radio.powerUp();
  radio.printDetails();
  // Disable Receiver
  radio.stopListening();

  TimerCount = 0;
  Rotations = 0; // Set Rotations to 0 ready for calculations

  //configure interrupt for tipping bucket scale

  pinMode(TippingBucketPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TippingBucketPin), rainISR, FALLING);
  tippingBucketTips = 0;

  //configure interrupt for wind speed
  pinMode(WindSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), windISR, FALLING);

  Timer1.initialize(500000);// Timer interrupt every 2.5 seconds
  Timer1.attachInterrupt(windISR_timer);

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

void transmit() {

  transmitWeatherString = generateWeatherString();
  sizeWeather = transmitWeatherString.length() + 1;
  char s[int(sizeWeather)];
  transmitWeatherString.toCharArray(s, transmitWeatherString.length() + 1);
  PCMSK2 &= ~(1 << PCINT23); //disabled weather vane interrupt
  radio.write(&s, sizeof(s)); //transmission
  Serial.println(s);
  PCMSK2 |= (1 << PCINT23);//re-enable weather vane interrupt

}

String generateWeatherString() {

  String weather = "";
  weather.concat(am2315.readTemperature());//temperature
  weather.concat(",");
  weather.concat(am2315.readHumidity());//humidity
  weather.concat(",");
  weather.concat(tippingBucketTips);//tipping bucket tips
  weather.concat(",");
  weather.concat(String(convertDirectionToString(windVaneDirection)));//winddirection
  weather.concat(",");
  weather.concat(windspeed);//windspeed
  weather.concat(",");
  weather.concat(int(bme.readPressure() / 100));//pressure

  return weather;

}
