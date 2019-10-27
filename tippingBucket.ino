//tipping bucket collector diamter 90mm
//volumne of bucket .7mm //amount im mm with every tip 0.1mm //connect one wire to ground
//connect 2nd wire to pin digital interrupt pin 2(phyiscally labelled ~3)

volatile long lastRiseTimeRain = 0;   
volatile long lastRiseTimeWind = 0;   

void rainISR() {

  //If more than 10 ms has elapsed since the last time pin 2 went high
  if ((millis() - lastRiseTimeRain) > 10)
  {

    tippingBucketTips++;

  }

  lastRiseTimeRain = millis();

}
