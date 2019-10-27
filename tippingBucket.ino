void rainISR() {

  //If more than 10 ms has elapsed since the last time pin 2 went high
  if ((millis() - lastRiseTimeRain) > 10)
  {

    tippingBucketTips++;

  }

  lastRiseTimeRain = millis();

}
