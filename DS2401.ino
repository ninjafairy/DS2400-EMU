// DS2400 Emulator by Squoril 9/28/2024
#define PB0 1   //PB0 = Pin D8 for a nano  = B0000001
#define PD2 3   //PD2 = Pin D2 for a nano  = B0000100
#define PB0m 0  // no shift needed for PB0 mask
#define PD2m 2  // shift the 1 (001) twice to get (100) for masking
#define 	F_CPU   16000000UL

unsigned char ESN[] = {142, 0, 0, 27, 196, 100, 133, 1};   //64bit ROM contents

void setup() {
  DDRB |= 0 << PB0;        // B-PORT to all inputs 0x00 mask doesnt change anything but still required to function?
    PORTB |= (1 << PORTB5);  // Set pin 5 high
  _delay_ms(250);              // Wait for a second
  PORTB &= ~(1 << PORTB5); // Set pin 5 low
  _delay_ms(250);              // Wait for a second
    PORTB |= (1 << PORTB5);  // Set pin 5 high
  _delay_ms(250);              // Wait for a second
  PORTB &= ~(1 << PORTB5); // Set pin 5 low
  _delay_ms(250);              // Wait for a second
}

void loop() {

// RESET PULSE DETECTION ----------------------------------------------------------------------------------------  Uses PB0 as input
		while (1){                       //scanning for the reset pulse
			if (!PINB & PB0){           //when target pin goes LOW begin timing LOW pulse
				uint8_t time0 = TCNT1; //timestamp of detected LOW
				waitForHigh();
				if (time0 - TCNT1 > 400){       //if pulse was longer than 200uS set to 400 for 16mhz /8 prescale
				break;}
			}
		}
  _delay_us(50);    //Delay between end of reset pulse and presence pulse

// PRESENSE PULSE RESPONSE -------------------------------------------------------------------------------------  Uses PB0 as output

    DDRB |= 1 << PB0m;          //PB0 set to OUTPUT and defaults to LOW beginning presence pulse
    _delay_us(150);
    DDRB &= ~(1 << PB0m);       //Reset PB0 to input high impedance so it doesnt hold high and can be used as input again
    _delay_us(150);

// COMMAND BYTE RECIEVING --------------------------------------------------------------------------------------  Uses PB0 as input

  	uint8_t romCommand = 0;
		TCNT1 = 0;
		uint8_t timestamp[16];
		for (int i = 0; i < 8; i++){                    //index counter for building the command byte
			int pointer = i*2;
			waitForLow();
			timestamp[pointer] = TCNT1;
			waitForHigh();
			timestamp[pointer+1] = TCNT1;
		}

// COMMAND BYTE DECODING ---------------------------------------------------------------------------------------

  		for (int i = 1; i < 16; i=i+2){             //start and end timestamps and step forward by 2 since 16 timestamps for 8 pulses starts and stops
			if ((timestamp[i]-timestamp[i-1])>10){    //i is end timestamp, subtract i-1 timestamp to get pulse width. double to 100 for 16mhz clock /8 prescale
			}
			else {
			romCommand |= 1 << ((i-1)/2);
			}
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
  _delay_us(35);
  DDRB &= ~(1 << PB0m);         //Reset PB0 to input high impedance so it doesnt hold high and can be used as input again
  _delay_us(5);
}

void searchRom(void){

  for (int i=7; i>=0; i--){                                 //Index for ESN[] bytes
    //for (byte mask = 0b00000001; mask>0; mask <<= 1){         //Index mask for bits out of selected ESN[] byte

    for (uint8_t j = 0; j < 8; ++j) {  // Loop through the 8 bits of the selected ESN[] byte
      uint8_t mask = 1 << j;

      waitForLow();                                         //Wait for read pulse from master                             DS2401 Tx BitN
      if(ESN[i] & mask){                                    //If current bit is a one
        _delay_us(10);}                             //Do not pull low for Zero Wait for read pulse to pass
      else{                                                 //It current bit wasnt a one
        pullLowForZero();}
      waitForLow();                                         //Wait for read pulse from master                             DS2401 Tx !BitN
      if(ESN[i] & mask){                                    //If current bit is a one
        pullLowForZero();}                                  //Send a Zero since were in the complement stage
      else{
        _delay_us(10);}                             //Do not pull low for Zero Wait for read pulse to pass
      waitForLow();                                         //Absorbing the Master Tx BitN                                Master Tx BitN
      waitForHigh();                                        //Incompatible with multiple 1-wire units now
    }
  }
}
