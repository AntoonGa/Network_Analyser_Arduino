
#include <SPI.h>
// ADC config (do not touch)
#define CONVST 8
#define RD 10
// SRAM Instructions (do not touch)
#define RDMR  5
#define WRMR  1
#define WRITE 2
#define READ  3
// SRAM modes (do not touch)
#define BYTE_MODE       0x00
#define PAGE_MODE       0x80
#define SEQUENTIAL_MODE 0x40
const int RAMpin = 9;

//ADC SETUP BYTES (set adc mode, read datasheet first. if you change mode: dont forget to change digitalvalue conversion !)
byte ADCbyte1=B10000000;                         //-+5V. CH0/GRD. NO_sleep/nap
byte ADCbyte2=B00000000;                         // filler byte
//Data BYTES (do not touch)
byte conversionMSB=0; byte conversionLSB=0;      //Storage for ADC-to-SRAM result high and low byte
byte conversionMSB2=0; byte conversionLSB2=0;    //Storage for SRAM-to-PC result high and low byte
byte TransfertMode;                              //(reading only, do not assign value)
//SETUP constants (do not touch)
int address=0;
float digitalValue;
unsigned long stamp;
unsigned long time;



//SETUP options: These two values set the frequency witdh/sampling rate etc.
const int BufferSize=16384;                          //number of sample :buffer size /!\ cannot go above 16384=2^14
int microsecPause=87;                                //Sampling PERIOD in us (not exact). Min=87@12kHz. !!!


void setup() {
  //Serial setup
  Serial.begin(1000000); //set at maximum speed. Does not influence the sampling rate. Must be same value as Processing.
  Serial.flush();
  //SPI setup
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2); //Set for 8Mhz SPI (can be faster if using Due)
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  //PIN Setup
  pinMode(RAMpin, OUTPUT);  //SlaveSelect (SS) SRAM
  pinMode(CONVST, OUTPUT);  //SS ADC
  pinMode(RD, OUTPUT);      //SS ADC (need both)
  
  //timing optimization stuff, add more at will.
  ADCSRA=0; // turn off arduino's ADC
  microsecPause=microsecPause-86;                   //manual calibration of the period. The number is approx the time needed to run the CODE (without SPI transfer)
  
  digitalWrite(CONVST, LOW);  //Set CONVST HIGH by default
  digitalWrite(RD, HIGH);  //Set and keep RD (adc SS) high
  
  /// /// /// /// /// Set SRAM sequential_mode/// /// /// /// /// ///
  digitalWrite(RAMpin, LOW);
   SPI.transfer(WRMR);
   SPI.transfer(SEQUENTIAL_MODE);
   digitalWrite(RAMpin, HIGH);
  
   
  digitalWrite(RAMpin, LOW);
   SPI.transfer(RDMR);
   TransfertMode=SPI.transfer(0x00);
   digitalWrite(RAMpin, HIGH);
   /// /// /// /// /// /// /// /// /// /// ///

} //end setup


void loop() {
  //record  ADC samples and transfer each of them in SRAM
  for (int i=0;i<BufferSize;i++){
  if (i==0){time=micros();}  //initial time
  ///////////////ADC GET BYTES/////////////////////////////////    
  ///////////////SLAVE SELECT: ADC///////////////
  digitalWrite(RAMpin, HIGH);
  digitalWrite(CONVST, HIGH); 
  digitalWrite(RD, LOW); 
  ///////////////GET ADC BYTE/////////////////////////
  conversionMSB=SPI.transfer(ADCbyte1);      // Single ended input at channel 0, 0V to 5V range (See LTC185x datasheet p15-16)
  conversionLSB=SPI.transfer(ADCbyte2);      // Filler
  delayMicroseconds(4);  //This delay is necessary to meet Tacq time (see datasheet p5). Change to 3us if voltage reads low
  //Trigger a conversion with a fast pulse
  noInterrupts();
  #if defined (__SAM3X8E__) // Arduino Due compatible
  REG_PIOC_SODR |= (0x01 << 22);
  REG_PIOC_CODR |= (0x01 << 22);
  #else
  PORTB |= 0x01;
  PORTB &= ~0x01;
  #endif
  interrupts();
  //Wait for conversion to be finished
  delayMicroseconds(4);
  /////////////////////////////////////////////////////
  
  
  /////////////////SRAM WRITE 2 BYTES/////////////////////////////////    
  ///////////////SLAVE SELECT: SRAM///////////////
  digitalWrite(RAMpin, LOW);
  digitalWrite(CONVST, LOW); 
  digitalWrite(RD, HIGH); 
  /////////////////TRANSFER TO SRAM//////////////////////////
  SPI.transfer(WRITE);
  SPI.transfer((uint8_t)(address >> 16)&0xFF);
  SPI.transfer((uint8_t)(address >> 8)&0xFF);
  SPI.transfer((uint8_t)address);
  SPI.transfer(conversionMSB);
  SPI.transfer(conversionLSB);
  ///////////////SLAVE SELECT: NO SLAVE///////////////
  digitalWrite(RAMpin, HIGH);
  digitalWrite(CONVST, LOW); 
  digitalWrite(RD, HIGH); 
  /////////////////////////////////////////////////////
  
  address= address+2;                      //increment address by two (2 bytes) for next writing
  delayMicroseconds(microsecPause);        // pauses. USED TO CONTROL SPECTRAL WIDTH vs RESOLUTION  
  if (i==BufferSize-1){stamp=micros()-time;} //total aquisition time!
  }
  ///////////////SRAM TO PC TRANSFER
  address=0; //set address to read to first address
  ///////////////SLAVE SELECT: SRAM///////////////
  digitalWrite(RAMpin, LOW);
  digitalWrite(CONVST, LOW); 
  digitalWrite(RD, HIGH); 
  /////////////////SET SRAM TO READ mode//////////////////////////
  SPI.transfer(READ);
  SPI.transfer((uint8_t)(address >> 16)&0xFF);
  SPI.transfer((uint8_t)(address >> 8)&0xFF);
  SPI.transfer((uint8_t)address);
  ////////////////READ SRAM////////////////////////////////
  Serial.flush();
  for (int i=0;i<BufferSize;i++){
  // read 2 bytes from SRAM, adress is incremented automatically 
  conversionMSB2=SPI.transfer(0x00);
  conversionLSB2=SPI.transfer(0x00); 
  digitalValue= (conversionMSB2 << 8) |  conversionLSB2;   //mix and match 2bytes
  if (digitalValue > 65534/2 ) {digitalValue=digitalValue-(65534);} //+-5V bipolar ADC has strange conversion system. See dataSheet
  Serial.println(digitalValue);  
  }
  ///////////////SLAVE SELECT: NO SLAVE///////////////
  digitalWrite(RAMpin, HIGH);
  digitalWrite(CONVST, LOW); 
  digitalWrite(RD, HIGH); 
  /////////////////////////////////////////////////////  
   Serial.print(stamp);
   Serial.println("\t");
}
  
// V2.0
// MAYHEW labs. LTC1859 + pcb 23lc1024 SRAM
// Wiring: plug the two shield making sure: Mayhews shield is between the SRAM shield and Arduino.
//    Pin 09 to /CS 23LC1024
//    Pin 11 to Din
//    Pin 12 to Dout
//    Pin 13 to CLK
//    Vdd/Vref: 5V arduino
//    Vss:      GND arduino
//    CH0: (10kOhm-)-Positive_BNC_waveform
//    GND_BNC_waveform: GND arduino
//	  See: picture.
      

  
