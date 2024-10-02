// DS2400 Emulator by Squoril 9/28/2024
#define PB0 1   //PB0 = Pin D8 for a nano  = B0000001
#define PD2 3   //PD2 = Pin D2 for a nano  = B0000100
#define PB0m 0  // no shift needed for PB0 mask
#define PD2m 2  // shift the 1 (001) twice to get (100) for masking

byte ESN[] = {142, 0, 0, 27, 196, 100, 133, 1};   //64bit ROM contents

void setup() {
  DDRB |= 0 << PB0;        // B-PORT to all inputs 0x00 mask doesnt change anything but still required to function?
}

void loop() {

// RESET PULSE DETECTION ----------------------------------------------------------------------------------------  Uses PB0 as input
  while (true){                       //scanning for the reset pulse
    if (!PINB & PB0){           //when target pin goes LOW begin timing LOW pulse
      unsigned long time0 = micros(); //timestamp of detected LOW
      waitForHigh();
      if (time0 - micros() > 200){       //if pulse was longer than 200uS
        break;}
    }
  }
  delayMicroseconds(50);    //Delay between end of reset pulse and presence pulse

// PRESENSE PULSE RESPONSE -------------------------------------------------------------------------------------  Uses PB0 as output

    DDRB |= 1 << PB0m;          //PB0 set to OUTPUT and defaults to LOW beginning presence pulse
    delayMicroseconds(150);
    DDRB &= ~(1 << PB0m);       //Reset PB0 to input high impedance so it doesnt hold high and can be used as input again
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
// END OF LOOP -------------------------------------------------------------------------------------------------


void waitForLow(void){
  while (PINB & PB0){}                   //pause while PB0 is HIGH
}

void waitForHigh(void){
  while (!PINB & PB0){}                  //pause while PB0 is LOW
}

void pullLowForZero(void){
  DDRB |= 1 << PB0m;            //PB0 set to OUTPUT and defaults to LOW beginning presence pulse
  delayMicroseconds(35);
  DDRB &= ~(1 << PB0m);         //Reset PB0 to input high impedance so it doesnt hold high and can be used as input again
  delayMicroseconds(5);
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
