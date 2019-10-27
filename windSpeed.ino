void getWindSpeed(){
  
  if(IsSampleRequired) { 
    // convert to mp/h using the formula V=P(2.25/T) 
    // V = P(2.25/2.5) = P * 0.9 
    windspeed = getKiloMeters(Rotations * 0.9); 
    Rotations = 0; // Reset count for next sample 
    
    IsSampleRequired = false; 

   } 
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
