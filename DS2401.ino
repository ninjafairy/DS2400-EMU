// DS2400 Emulator by Squoril 9/28/2024
byte ESN[] = {142, 0, 0, 27, 196, 100, 133, 1};   //64bit ROM contents
void setup() {
  DDRB = DDRB | B00000000;        // B-PORT to all inputs 0x00 mask doesnt change anything but still required to function?
  PORTB = PORTB | B00000001;      // PB0 (pin 8) PULLUP resistor on.
}

void loop() {

// RESET PULSE DETECTION ----------------------------------------------------------------------------------------  Uses PB0 as input
  while (true){                       //scanning for the reset pulse
    if (!PINB & B00000001){           //when target pin goes LOW begin timing LOW pulse
      unsigned long time0 = micros(); //timestamp of detected LOW
      waitForHigh();
      if (time0 - micros() > 200){       //if pulse was longer than 200uS
        break;}
    }
  }
  delayMicroseconds(50);    // can this be removed?

// PRESENSE PULSE RESPONSE -------------------------------------------------------------------------------------  Uses PB2 as output

    DDRD = DDRD | B00000100;        //PD2 set to OUTPUT and defaults to LOW beginning presence pulse
    delayMicroseconds(150);
    DDRD = DDRD ^ B00000100;        //Reset PD2 to input high impedance so it doesnt hold high
    delayMicroseconds(150);

// COMMAND BYTE RECIEVING --------------------------------------------------------------------------------------  Uses PB0 as input
  byte romCommand;
  unsigned long decodeTime = micros();
  unsigned long timestamp[16];
  for (int i = 0; i < 8; i++){                    //index counter for building the command byte
    unsigned long time0;
    int pointer = i*2;
    waitForLow();
    timestamp[pointer] = micros();
    waitForHigh();
    timestamp[pointer+1] = micros();
  }

// COMMAND BYTE DECODING ---------------------------------------------------------------------------------------
  for (int i = 1; i < 16; i=i+2){             //start and end timestamps and step forward by 2 since 16 timestamps for 8 pulses starts and stops
    if ((timestamp[i]-timestamp[i-1])>50){    //i is end timestamp, subtract i-1 timestamp to get pulse width 
      bitWrite(romCommand, ((i-1)/2), 0);}
    else {
      bitWrite(romCommand, ((i-1)/2), 1);}
  }

// COMMAND SWITCH ----------------------------------------------------------------------------------------------
  if(romCommand == 0xF0){
    searchRom();
  } 
}


void waitForLow(void){
  while (PINB & B00000001){}                   //pause while PB0 is HIGH
}

void waitForHigh(void){
  while (!PINB & B00000001){}                  //pause while PB0 is LOW
}

void pullLowForZero(void){
  DDRD = DDRD | B00000100;        //PB2 set to OUTPUT and defaults to LOW beginning Bit = 0 pulse
  delayMicroseconds(35);
  DDRD = DDRD ^ B00000100;        //Reset PB2 to input high impedance so it doesnt hold high but is allowed to be pulled up
  delayMicroseconds(5);           //safety delay might be able to remove
}

void searchRom(void){

  for (int i=7; i>=0; i--){                                 //Index for ESN[] bytes
    for (byte mask = 00000001; mask>0; mask <<= 1){         //Index mask for bits out of selected ESN[] byte
      waitForLow();                                         //Wait for read pulse from master                             DS2401 Tx BitN
      if(ESN[i] & mask){                                    //If current bit is a one
        delayMicroseconds(10);}                             //Do not pull low for Zero Wait for read pulse to pass
      else{                                                 //It current bit wasnt a one
        pullLowForZero();}
      waitForLow();                                         //Wait for read pulse from master                             DS2401 Tx !BitN
      if(ESN[i] & mask){                                    //If current bit is a one
        pullLowForZero();}                                  //Send a Zero since were in the complement stage
      else{
        delayMicroseconds(10);}                             //Do not pull low for Zero Wait for read pulse to pass
      waitForLow();                                         //Absorbing the Master Tx BitN                                Master Tx BitN
      waitForHigh();                                        //Incompatible with multiple 1-wire units now
    }
  }
}
