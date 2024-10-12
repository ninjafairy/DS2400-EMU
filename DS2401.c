#define F_CPU 8000000UL											//Internal 8MHz Ocilator, /1 prescale = 8MHz clkCPU
#include "avr/io.h"
#include "util/delay.h"
#define PB0pin 1												//PB0 = Pin D8 for a nano  = B0000001
#define PB0m 0													// no shift needed for PB0 mask

uint8_t ESN[] = {142, 0, 0, 27, 196, 100, 133, 1};				//64bit ROM contents
	
void waitForLow(void){
	while (PINB & PB0){
	}										//pause while PB0 is HIGH
}

void waitForHigh(void){
	while (!PINB & PB0){
	}										//pause while PB0 is LOW
}

void pullLowForZero(void){
	DDRB |= 1 << PB0m;											//PB0 set to OUTPUT and defaults to LOW beginning presence pulse
	_delay_us(35);
	DDRB &= ~(1 << PB0m);										//Reset PB0 to input high impedance so it doesnt hold high and can be used as input again
	_delay_us(5);
}

void searchRom(void){
	for (int i=7; i>=0; i--){									//Index for ESN[] bytes
		for (uint8_t mask = 1; mask>0; mask <<= 1){             //Index mask for bits out of selected ESN[] byte
			waitForLow();                                       //Wait for read pulse from master                             DS2401 Tx BitN
			if(ESN[i] & mask){                                  //If current bit is a one
				_delay_us(10);									//Do not pull low for Zero Wait for read pulse to pass
			}                                     
			else{                                               //It current bit wasnt a one
				pullLowForZero();
			}
			waitForLow();                                       //Wait for read pulse from master                             DS2401 Tx !BitN
			if(ESN[i] & mask){                                  //If current bit is a one
				pullLowForZero();								//Send a Zero since were in the complement stage
			}                                  
			else{
				_delay_us(10);									//Do not pull low for Zero Wait for read pulse to pass
			}                                     
			waitForLow();                                       //Absorbing the Master Tx BitN                                Master Tx BitN device select
			waitForHigh();                                      //Incompatible with multiple 1-wire units now
		}
	}
}
	
int main(void){
	//CLKMSR = 0;												//Defaults to 0 for internal 8mhz occilator?
	CLKPSR = 0;													//Set /1 prescaler for max CPU speed on internal oscillator, Source clock = 8Mhz clkCPU = 8Mhz
	TCCR0B = 0x2;												//Set /8 prescaler timer0 now counts 1us per tick            Timer0 clock = 1Mhz
		
    while (1) 
    {
// RESET PULSE DETECTION -- Reset Pulse nominal 480us will overflow 8-bit counter, need to use 16-bit -----------  Uses PB0 as input
		while (1){                                              //scanning for the reset pulse
			if (!PINB & PB0){                                   //when target pin goes LOW begin timing LOW pulse
				TCNT0 = 0;                                      //reset Timer0 to 0, 8Mhz Attiny with /8 prescale will be 1us per tick
				uint16_t time0 = TCNT0;                         //timestamp of detected LOW, TCNT0 atomic reads the high/low bytes of the timer, 65ms overflow
				waitForHigh();
				if (TCNT0 - time0 > 200){                       //if pulse was longer than 200uS 8mhz /8 prescale 1 tick = 1us nominal ~480us
					break;
				}
			}
		}
		_delay_us(50);                                          //Delay between end of reset pulse and presence pulse

// PRESENSE PULSE RESPONSE -------------------------------------------------------------------------------------  Uses PB0 as output
		DDRB |= 1 << PB0m;                                      //PB0 set to OUTPUT and defaults to LOW beginning presence pulse
		_delay_us(150);
		DDRB &= ~(1 << PB0m);                                   //Reset PB0 to input high impedance so it doesnt hold high and can be used as input again
		_delay_us(150);

// COMMAND BYTE READ -------------------------------------------------------------------------------------------  Uses PB0 as input
		uint8_t romCommand = 0;
		for (int i = 0; i < 8; i++){                            //index counter for building the command byte
			waitForLow();
			TCNT0 = 0;
			waitForHigh();
			if (TCNT0L < 40){                                   //1us per tick, Master holds low for 65us for a 0, so less than 40us is going to be a 1 (10us nominal)
				romCommand |= 1 << i;                           //Below threshold, set i index bit of command byte
			}
		}

// COMMAND SWITCH ----------------------------------------------------------------------------------------------
		if(romCommand == 0xF0){                                 //0xF0 SearchRom,  0X33 ReadRom, 0x0F legacy ReadRom for the 2400
			searchRom();
		}
	}
// END OF LOOP -------------------------------------------------------------------------------------------------
}


