unsigned long time0 = micros();
bool resetPulse = false;
bool ox33 = false;
byte family = 1;                                      // value of 1 indicates an ESN
byte serialNumber[] = {133, 100, 196, 27, 0, 0,};     // S/N value
byte crc = 142;                                       // crc, if you change the S/N must recompute 
byte ESN[] = {1, 133, 100, 196, 27, 0, 0, 142};
//byte ESN[] = {255,255,255,255,0,0,0,0};
byte famCode = 0b10100111;
unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(8, INPUT_PULLUP);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  Serial.begin(9600);
  time1 = micros();
  Serial.print("Boot time: ");
  Serial.println(time1-time0);
  Serial.print("famCode: ");
  Serial.println(famCode, BIN);
}

void loop() {
  // put your main code here, to run repeatedly:

// RESET PULSE DETECTION
  while (resetPulse == false){      //scanning for the reset pulse
    if (!PINB){                     //when target pin goes low
      time0 = micros();             //reset timer
      while (!PINB){                //pause while pin is LOW
        delayMicroseconds(1);
      }
      time1 = micros();             //second timestamp
      if (time0 - time1 > 200){     //if pulse was longer than 200uS
        resetPulse = true;          //flag resetPulse true to break out of scanning loop
      }
    }
  }
resetPulse = false;                 //reset flag for next round.

//COMMAND BYTE RECIEVING AND DECODING
for (int i = 7; i >= 0; i--){       //index counter for building the command byte

  while (PINB){                     //wait for the pin to get pulled LOW
    time1 = micros();               //set timestamp once LOW
  }                  
  while (!PINB){                    //pause till pin goes HIGH again
    delayMicroseconds(1);
  }
  time2 = micros() - time1;         //calculate pulse time (better than setting a second timestamp first?)           
  if (time2 < 30){                  //if it stayed LOW for less than 30uS its a 1
    bitWrite(famCode, i, 1);        //set bit and index to 1
    delayMicroseconds(100-time2);   //get close to the 120uS cycle time
  }
  else {
    bitWrite(famCode, i, 0);        //else its a 0 
  }
  
}
//Serial.print("\n"); printBin(famCode);
if (famCode == 0b11001100){
 // Serial.println("\n0x33 Detected!!");
  ox33 = true;
}


  if (0x33){
    // for (int bite = 0; bite < 8; bite++){
    //   //Serial.print(ESN[bite], HEX);
    //   //printBin(ESN[bite]);
    //   for (int bitt = 7; bitt >= 0; bitt--){
    //     Serial.print(bitRead(ESN[bite], bitt));
    //   }
    // }
    for (int bite = 0; bite < 8  ; bite++){
      for (int bitt = 0; bitt < 8; bitt++){
          while(PINB){
            delayMicroseconds(1);
          }
          while(!PINB){
            delayMicroseconds(1);
          }
          if (bitRead(ESN[bite], bitt)){
          digitalWrite(2, HIGH);
          //Serial.println("1");
          }
          delayMicroseconds(60);
          digitalWrite(2, LOW);


      }
    }
  }
  Serial.println("\n0x33 Detected!!");
}

void printBin(byte aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}
