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

String convertDirectionToString(String binary) {

  char s1[binary.length()];
  binary.toCharArray(s1, binary.length() + 1);
  int value = 0;
  for (int i = 0; i < strlen(s1); i++) // for every character in the string  strlen(s) returns the length of a char array
  {
    value *= 2; // double the result so far
    if (s1[i] == '1') value++;  //add 1 if needed
  }
  //Serial.println(value);

  /* '''
    0    0    0    0    N
    0    0    0    1    NNE
    0    0    1    0    NE
    0    0    1    1    ENE
    0    1    0    0    E
    0    1    0    1    ESE
    0    1    1    0    SE
    0    1    1    1    SSE
    1    0    0    0    S
    1    0    0    1    SSW
    1    0    1    0    SW
    1    0    1    1    WSW
    1    1    0    0    W
    1    1    0    1    WNW
    1    1    1    0    NW
    1    1    1    1    NNW
    '''*/
  switch (value) {
    case 0:
      return "N";
    case 1:
      return "NNE";
    case 2:
      return "NE";
    case 3:
      return "ENE";
    case 4:
      return "E";
    case 5:
      return "ESE";
    case 6:
      return "SE";
    case 7:
      return "SSE";
    case 8:
      return "S";
    case 9:
      return "SSW";
    case 10:
      return "SW";
    case 11:
      return "WSW";
    case 12:
      return "W";
    case 13:
      return "WNW";
    case 14:
      return "NW";
    case 15:
      return "NNW";
    default:
      return binary;
  }

}
