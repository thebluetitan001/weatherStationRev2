void transmit() {

  String transmitWeatherString = generateWeatherString();
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
