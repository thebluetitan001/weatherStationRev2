#include <RF24.h>
#include <SPI.h>
#include <TimerOne.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_AM2315.h>
#include <Adafruit_BMP280.h>

//time between transmissions
const int tranmissionDelayTime = 5000;
bool requestString = true;
String weather;

//defines settings for RF24ghz Transceiver
//RF24 radio(8, 9); //Define Radio (CE-PIN,CSN-PIN)
#define RADIO_CE_PIN 8
#define RADIO_CS_PIN 9
RF24 radio = RF24(RADIO_CE_PIN, RADIO_CS_PIN);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//used to define max buffer size of wireless transmission
int sizeWeather;


#define WindSensorPin (2) //define pin for WindSpeedSensor
//wind speed in km
volatile float windspeed;
volatile bool IsSampleRequired; // this is set true every 2.5s. Get wind speed 
volatile unsigned int TimerCount; // used to determine 2.5sec timer count 
volatile unsigned long Rotations; // cup rotation counter used in interrupt routine 
volatile unsigned long ContactBounceTime; // Timer to avoid contact bounce in isr 
//float WindSpeed;

//tipping bucket collector diamter 90mm
//volumne of bucket .7mm
//amount im mm with every tip 0.1mm
//connect one wire to ground
//connect 2nd wire to pin digital interrupt pin 2(phyiscally labelled ~3)
#define TippingBucketPin (3)//define pin for Tipping Bucket Sensor

//number of times the buckets has tipped
volatile int tippingBucketTips = 0;
volatile long lastRiseTimeRain = 0;
volatile long lastRiseTimeWind = 0;

//defines settings for BMP280 Barometric Sensor
Adafruit_BMP280 bme;
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

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

//reset function
//void(* resetFunc) (void) = 0;//declare reset function at address 0

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
  }else{
   Serial.println("AM2315 (Temp/Humidity) sensor status OK... continue");
   delay(1000);
  }


  // delay(500);
  if (!bme.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    //while (1);
  }else{
   Serial.println("BM180 (Barometric Pressure) sensor status OK... continue");
   delay(1000);
}

  /*
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
     //Serial.println(F("SSD1306 allocation failed"));
     //for(;;); // Don't proceed, loop forever
    }
  */
  
  //Serial.println("Beginning 2.4ghz Radio transmitter configuration.... ");

  radio.begin();
  delay(1000);
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

  IsSampleRequired = false; 
  
  TimerCount = 0; 
  Rotations = 0; // Set Rotations to 0 ready for calculations 

  //configure interrupt for tipping bucket scale

  pinMode(TippingBucketPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TippingBucketPin), rainISR, FALLING);

  //configure interrupt for wind speed
  pinMode(WindSensorPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), windISR, FALLING); 

  Timer1.initialize(500000);// Timer interrupt every 2.5 seconds 
  Timer1.attachInterrupt(windISR_timer);
}

void loop() {

    if(IsSampleRequired) { 
    // convert to mp/h using the formula V=P(2.25/T) 
    // V = P(2.25/2.5) = P * 0.9 
    windspeed = getKiloMeters(Rotations * 0.9); 
    Rotations = 0; // Reset count for next sample 
    
    IsSampleRequired = false; 

    } 

  delay(tranmissionDelayTime);
  //windSpeedCurrent = rotations * resolution;

  if(requestString){
  PCMSK2 &= ~(1 << PCINT23);
  for (int i = 0; i < sizeof(wVD); i++) {
    // Serial.println(wVD[i]);
    windVaneDirection.concat(wVD[i]);
  }

  if (windVaneDirection.length() == 4) {
    generateWeatherString();
  }
  PCMSK2 |= (1 << PCINT23);
  }
  RESETCOUNTER = false;
  count = 0;

}


ISR(WDT_vect)
{


}

ISR(PCINT2_vect) {

  //grabs stream of data coming from wind vane, and stores the final 4 bits in a string for transmission

  newWindVaneTime = micros();
  windVaneCompare = newWindVaneTime - lastWindVaneTime;

  //finds the start bit in the datastream, times appears to be around 3000ms
  if ((windVaneCompare ) < 4000 and (windVaneCompare) > 2000) {
    count = 0;
    arrayCounter = 0;
    windVaneDirection = "";

  }

  if (RESETCOUNTER == false) {
    if (count < windVaneSampleSize) {

      //modulus function differentiates between the rise and fall of the pin to know when to start reading time of the bit

      if ((count % 2) == 1) {
        if ((windVaneCompare) > 700) {

          //greater than 700ms is a 1, below is a zero
          //stores the final 4 bits in the array
          //final 4 bits contains the directional data

          if (arrayCounter > 3 and arrayCounter < 8) {

            //windVaneDirection.concat(1);
            wVD[arrayCounter - 4] = '1';

          }
          arrayCounter ++;
        } else {
          if (arrayCounter > 3 and arrayCounter < 8) {

            //windVaneDirection.concat(0);
            wVD[arrayCounter - 4] = '0';

          }
          arrayCounter ++;
          if (arrayCounter > 8) {
            RESETCOUNTER == true;
          }
        }

      }
      count++;
    }
    lastWindVaneTime = newWindVaneTime;
  }


}

void rainISR() {

  //If more than 10 ms has elapsed since the last time pin 2 went high
  if ((millis() - lastRiseTimeRain) > 10)
  {

    tippingBucketTips++;

  }

  lastRiseTimeRain = millis();

}


void generateWeatherString() {
  weather = "Temperature ";
  weather = "";
  weather.concat(am2315.readTemperature());
  weather.concat(",");
  //weather.concat(",Humidity ");
  weather.concat(am2315.readHumidity());
  weather.concat(",");
  //weather.concat(",Rain Tips ");
  weather.concat(tippingBucketTips);
  weather.concat(",");
  //weather.concat(",Wind Direct ");
  weather.concat(windVaneDirection);
  weather.concat(",");
  //weather.concat(int(windVaneDirection,HEX));
  //weather.concat(",Wind Speed ");
  weather.concat(windspeed);
  weather.concat(",");
  //weather.concat("Kph ,Pressure ");
  weather.concat(int(bme.readPressure() / 100));

  //weather = "Hello World";
  sizeWeather = weather.length() + 1;

  char s[int(sizeWeather)];

  weather.toCharArray(s, weather.length() + 1);

  PCMSK2 &= ~(1 << PCINT23);
  radio.write(&s, sizeof(s));
 
  Serial.println(s);
  
  PCMSK2 != (1 << PCINT23);

}

void windISR_timer(){
  
  TimerCount++; 
  
  if(TimerCount == 6){ 
    IsSampleRequired = true; 
    TimerCount = 0; 
    } 
} 

// This is the function that the interrupt calls to increment the rotation count 
void windISR() { 
  
  if((millis() - ContactBounceTime) > 20 ) { // debounce the switch contact. 
    Rotations++; 
    ContactBounceTime = millis(); 
  } 
} 

// Convert MPH to KPH 
float getKiloMeters(float speed) { 
  return speed * 0.621371; 
} 

