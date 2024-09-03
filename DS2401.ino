
byte ESN[] = {1, 133, 100, 196, 27, 0, 0, 142};

void setup() {
  DDRB = DDRB | B00000000;        // B-PORT to all inputs 0x00 mask doesnt change anything but still required to function?
  PORTB = PORTB | B00000001;      // PB0 (pin 8) PULLUP resistor on.
  DDRD = DDRD | B00000100;        // PD2 (pin 2) OUTPUT
  PORTD = B00000000;              // D-PORT all pins LOW
}

void loop() {

// RESET PULSE DETECTION ----------------------------------------------------------------------------------------
  bool resetPulse = false;
  while (resetPulse == false){        //scanning for the reset pulse
    if (!PINB & B00000001){           //when target pin goes LOW begin timing LOW pulse
      unsigned long time0 = micros();               //timestamp of detected LOW
      while (!PINB & B00000001){      //pause while pin is LOW
        delayMicroseconds(1); 
      }
      unsigned long time1 = micros();               //timestamp when pin goes HIGH
      if (time0 - time1 > 200){       //if pulse was longer than 200uS
        resetPulse = true;            //flag resetPulse true to break out of scanning loop
      }
    }
  }

// COMMAND BYTE RECIEVING AND DECODING -------------------------------------------------------------------------
  byte romCommand;
  for (int i = 0; i < 8; i++){        //index counter for building the command byte
    unsigned long time0;
    while (PINB & B00000001){                     //wait for the pin to get pulled LOW
      time0 = micros();               //set timestamp once LOW
    }                  
    while (!PINB & B00000001){                    //pause till pin goes HIGH again
      delayMicroseconds(1);
    }
     unsigned long time1 = micros() - time0;         //calculate pulse time (better than setting a second timestamp first?)           
    if (time1 < 30){                  //if it stayed LOW for less than 30uS its a 1
      bitWrite(romCommand, i, 1);     //set bit and index to 1
      delayMicroseconds(100-time1);   //get close to the 120uS cycle time
    }
    else {
      bitWrite(romCommand, i, 0);     //else its a 0 
    }
  }

  if (romCommand == 0x33){
    sendRom();
    // Serial.println("\n0x33 Detected!!");
  }
  else if(romCommand == 0x0F){
    sendRom();
    // Serial.println("\n0x0F Detected!!");
  } 
}

void sendRom(void) {
  for (int bite = 0; bite < 8  ; bite++){
    for (int bitt = 0; bitt < 8; bitt++){
      while(PINB){
        delayMicroseconds(1);
      }
      while(!PINB){
        delayMicroseconds(1);
      }
      if (bitRead(ESN[bite], bitt)){
        PORTD = B00000100;
      }
      delayMicroseconds(60);
      PORTD = B00000000;
    }
  }
}

void printBin(byte aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}
