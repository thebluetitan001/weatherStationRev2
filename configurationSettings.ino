void configureRadio(){
 
  radio.begin();
  radio.setPALevel(RF24_PA_MAX); // Transmit Power (MAX,HIGH,LOW,MIN) //Decimal 76 is 0x4c in hexidecimal confirm that you have configured correctly on both sides
  radio.setChannel(76);
  radio.setDataRate( RF24_1MBPS ); //Transmit Speeed (250 Kbits)
  radio.enableDynamicPayloads();
  radio.openWritingPipe(pipes[0]);
  radio.powerUp();
  radio.printDetails();
  radio.stopListening();  // Disable Receiver
}
void enableBMP180(){

  if (!bme.begin()) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    //while (1);
  }
  
}
void enableAM2315(){
  
    // AM2315 sensor wiring - Connect RED to 5.0V, BLACK to Ground, WHITE to i2c clock to Analog 5, Connect YELLOW to i2c data to Analog 4
    //enables driver for am2315 temperature and humidty sensor
  if (! am2315.begin()) {
    Serial.println("AM2315 (Temp/Humidity) Sensor not found, check wiring & pullups!");
    //while (1);
  }

  
}
