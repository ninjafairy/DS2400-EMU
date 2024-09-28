
//byte ESN[] = {1, 133, 100, 196, 27, 0, 0, 142};  // I have the dumb with bit significance index 0 holds the family code LSB
byte ESN[] = {142, 0, 0, 27, 196, 100, 133, 1};
//                                               // when changing the ESN place bytes L to R: Family code, S/N, CRC
void setup() {
  DDRB = DDRB | B00000000;        // B-PORT to all inputs 0x00 mask doesnt change anything but still required to function?
  PORTB = PORTB | B00000001;      // PB0 (pin 8) PULLUP resistor on.
  //DDRD = DDRD | B00000100;        // PD2 (pin 2) OUTPUT
  //PORTD = B00000000;              // D-PORT all pins LOW
  Serial.begin(9600);
  Serial.println("boot");
  unsigned long time1 = micros();
  unsigned long timearray[16];
  timearray[0] = micros();
  timearray[1] = micros();
  timearray[2] = micros();
  time1 = micros();
  // Serial.print("time1: "); Serial.println(time1);
  // Serial.print("array0: "); Serial.println(timearray[0]);
  // Serial.print("array1: "); Serial.println(timearray[1]);
  // Serial.print("array2: "); Serial.println(timearray[2]);
  unsigned long timea = micros();
  unsigned long timeb = micros();
  unsigned long timec = micros();
  Serial.print("timea: "); Serial.println(timea-time1);
  Serial.print("timeb: "); Serial.println(timeb-time1);
  Serial.print("timec: "); Serial.println(timec-time1);

}

void loop() {

// RESET PULSE DETECTION ----------------------------------------------------------------------------------------
  bool resetPulse = false;
  Serial.println("scanning...");
  while (resetPulse == false){        //scanning for the reset pulse
    if (!PINB & B00000001){           //when target pin goes LOW begin timing LOW pulse
      unsigned long time0 = micros();               //timestamp of detected LOW
      while (!PINB & B00000001){      //pause while pin is LOW
        delayMicroseconds(1); 
      }
      unsigned long time1 = micros(); //timestamp when pin goes HIGH
      if (time0 - time1 > 200){       //if pulse was longer than 200uS
        resetPulse = true;            //flag resetPulse true to break out of scanning loop
      }
    }
  }
  //Serial.println("reset pulse");
  delayMicroseconds(50);

// PRESENSE PULSE RESPONSE -------------------------------------------------------------------------------------

    DDRD = DDRD | B00000100;        //PB2 set to OUTPUT and defaults to LOW beginning presence pulse
    delayMicroseconds(150);
    DDRD = DDRD ^ B00000100;        //Rest PB0 to input high impedance so it doesnt hold high
    delayMicroseconds(150);


// COMMAND BYTE RECIEVING AND DECODING -------------------------------------------------------------------------
  byte romCommand;
  unsigned long decodeTime = micros();
  // for (int i = 0; i < 8; i++){        //index counter for building the command byte
  //   unsigned long time0;
  //   while (PINB & B00000001){                     //wait for the pin to get pulled LOW
  //     time0 = micros();               //set timestamp once LOW
  //   }                  
  //   while (!PINB & B00000001){                    //pause till pin goes HIGH again
  //     delayMicroseconds(1);
  //   }
  //    unsigned long time1 = micros() - time0;         //calculate pulse time (better than setting a second timestamp first?)           
  //   if (time1 < 30){                  //if it stayed LOW for less than 30uS its a 1
  //     bitWrite(romCommand, i, 1);     //set bit and index to 1
  //     delayMicroseconds(60-time1);   //get close to the 120uS cycle time
  //   }
  //   else {
  //     bitWrite(romCommand, i, 0);     //else its a 0 
  //   }
  // }
  // for (int i = 0; i < 8; i++){        //index counter for building the command byte
  //   unsigned long time0;
  //   while (PINB & B00000001){                     //wait for the pin to get pulled LOW
  //     time0 = micros();               //set timestamp once LOW
  //   }
  //   delayMicroseconds(20);
  //   if (!PINB & B00000001){
  //     bitWrite(romCommand, i, 1);
  //   }
  //   else if (PINB & B00000001){
  //     bitWrite(romCommand, i, 0);
  //     while (!PINB & B00000001){                    //pause till pin goes HIGH again
  //       delayMicroseconds(1);
  //     }
  //   }
  // }

unsigned long timestamp[16];
  for (int i = 0; i < 8; i++){        //index counter for building the command byte
    unsigned long time0;
    int pointer = i*2;
    while (PINB & B00000001){                     //wait for the pin to get pulled LOW
      delayMicroseconds(1);
    }
    timestamp[pointer] = micros();
    while (!PINB & B00000001){                     //wait for the pin to get pulled HIGH
      delayMicroseconds(1);
    }
    timestamp[pointer+1] = micros();
  }
  // unsigned long decodetime2 = micros() - decodeTime;
  // Serial.println(decodetime2);   // This is the time from the end of the 150us delay after the presense pulse to when the last bit in the command goes high
                                    // Currently ~630us and spot on there is ~57us of time from now to when the controller performs its first read


    // while (PINB & B00000001){                     //wait for the pin to get pulled LOW
    // }
    // unsigned long timebase = micros();
    // timestamp[0] = micros();
    // while (!PINB & B00000001){                     //wait for the pin to get pulled HIGH
    // }
    // timestamp[1] = micros();
    // while (PINB & B00000001){                     //wait for the pin to get pulled LOW
    // }
    // timestamp[2] = micros();
    // while (!PINB & B00000001){                     //wait for the pin to get pulled HIGH
    // }
    // timestamp[3] = micros();
    // while (PINB & B00000001){                     //wait for the pin to get pulled LOW
    // }
    // timestamp[4] = micros();
    // while (!PINB & B00000001){                     //wait for the pin to get pulled HIGH
    // }
    // timestamp[5] = micros();
    
    
  
  for (int i = 1; i < 16; i=i+2){
    //Serial.println(timestamp[i]-timestamp[i-1]);
    if ((timestamp[i]-timestamp[i-1])>50){
      bitWrite(romCommand, ((i+1)/2)-1, 0);
    }
    else {
      bitWrite(romCommand, ((i+1)/2)-1, 1);
    }
  }

  if (romCommand == 0x33){
    sendRom();
     Serial.println("\n0x33 Detected!!");
  }
  else if(romCommand == 0xF0){
    searchRom();
    Serial.println("\n0xF0 Detected!!");
  } 
  else if(romCommand == 0xE0){
    Serial.println("\n0xE0 Detected!!");
    sendRom();
    Serial.println("\n0xE0 Detected!!");
  } 

  Serial.print("Rom Command: "); Serial.println(romCommand, HEX);
}

void sendRom(void) {
  for (int bite = 0; bite < 8  ; bite++){
    for (int bitt = 0; bitt < 8; bitt++){
      while(PINB & B00000001){
        delayMicroseconds(1);
      }
      while(!PINB & B00000001){
        delayMicroseconds(1);
      }
      if (bitRead(ESN[bite], bitt)){
        DDRD = DDRD | B00000100;
      }
      delayMicroseconds(60);
      DDRD = DDRD ^ B00000100;
    }
  }
}

void searchRom(void){
// Serial.print("byte 0: "); Serial.println(ESN[0], BIN);
// Serial.print("byte 1: "); Serial.println(ESN[1], BIN);
// Serial.print("byte 2: "); Serial.println(ESN[2], BIN);


  for (int i=7; i>=0; i--){
    for (byte mask = 00000001; mask>0; mask <<= 1){
      while (PINB & B00000001){                     //wait for the pin to get pulled LOW 
      }
      if(ESN[i] & mask){
        //Serial.print(1);
        //Leave line HIGH for a 1
        delayMicroseconds(10);
      }
      else{
        //Serial.print(0);
        DDRD = DDRD | B00000100;        //PB2 set to OUTPUT and defaults to LOW beginning presence pulse
        delayMicroseconds(35);
        DDRD = DDRD ^ B00000100;        //Rest PB0 to input high impedance so it doesnt hold high
        delayMicroseconds(5);
      }

      while (PINB & B00000001){                     //wait for the pin to get pulled LOW 2nd bit read, send complement
      }
      if(ESN[i] & mask){
          DDRD = DDRD | B00000100;        //PB2 set to OUTPUT and defaults to LOW beginning presence pulse
          delayMicroseconds(35);
          DDRD = DDRD ^ B00000100;        //Rest PB0 to input high impedance so it doesnt hold high
          delayMicroseconds(5);
      }
      else{
          delayMicroseconds(10);
      }
      while (PINB & B00000001){                     //wait for the pin to get pulled LOW master transmit catch
      }
      while (!PINB & B00000001){                     //wait for the pin to get let HIGH absorb the low pulse
      }
      
    }
  }



}
