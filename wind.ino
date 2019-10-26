#include <TimerOne.h>
#include <math.h>

#define WindSensorPin (3)

volatile bool IsSampleRequired; // this is set true every 2.5s. Get wind speed 
volatile unsigned int TimerCount; // used to determine 2.5sec timer count 
volatile unsigned long Rotations; // cup rotation counter used in interrupt routine 
volatile unsigned long ContactBounceTime; // Timer to avoid contact bounce in isr 
volatile unsigned int Rotations2;
float WindSpeed;

void setup() {
  
  IsSampleRequired = false; 
  
  TimerCount = 0; 
  Rotations = 0; // Set Rotations to 0 ready for calculations 
  
  Serial.begin(9600); 
  
  pinMode(WindSensorPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING); 
  
  // Setup the timer interupt 
  Timer1.initialize(500000);// Timer interrupt every 2.5 seconds 
  Timer1.attachInterrupt(isr_timer); 

}

void loop() {
  
  if(IsSampleRequired) { 
    // convert to mp/h using the formula V=P(2.25/T) 
    // V = P(2.25/2.5) = P * 0.9 
    WindSpeed = Rotations * 0.9; 
    Rotations = 0; // Reset count for next sample 
    
    IsSampleRequired = false; 
    
    Serial.println("Speed (MPH)\tKnots\tKPH\tDirection\tStrength");  
    Serial.print(WindSpeed); Serial.print("\t\t"); 
    Serial.print(getKnots(WindSpeed)); Serial.print("\t"); 
    Serial.print(getKiloMeters(WindSpeed)); Serial.print("\t");
    Serial.println(); 
} 

}

// isr handler for timer interrupt 
void isr_timer(){
  
  TimerCount++; 
  
  if(TimerCount == 6){ 
    IsSampleRequired = true; 
    TimerCount = 0; 
    } 
} 

// This is the function that the interrupt calls to increment the rotation count 
void isr_rotation() { 
  
  if((millis() - ContactBounceTime) > 20 ) { // debounce the switch contact. 
    Rotations++; 
    ContactBounceTime = millis(); 
  } 
} 

// Convert MPH to KPH 
float getKiloMeters(float speed) { 
  return speed * 0.621371; 
} 

// Convert MPH to Knots 
float getKnots(float speed) { 
  return speed * 0.868976; 
} 
