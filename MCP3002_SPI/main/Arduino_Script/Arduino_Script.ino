#include <SPI.h>
// SRAM Instructions
#define RDMR  5
#define WRMR  1
#define WRITE 2
#define READ  3
// SRAM modes
#define BYTE_MODE       0x00
#define PAGE_MODE       0x80
#define SEQUENTIAL_MODE 0x40


byte TransfertMode;                             //(reading only, do not assign value)
int bps=1000000;                                //in BITS/s

const int ADCpin = 8;                          //pins.
const int RAMpin = 9;

int BufferSize=8192; //Number of samples, adjust for speed vs precision
int ControlDelay=300;//Control delay, set to get correct spectral resolution/width (see on matlab for values)

byte conversionMSB,conversionLSB,conversionMSB2,conversionLSB2 ; //MCP3002/SRAM data bytes
byte byte1=0b01100000; //MCP3002 config byte
byte byte2=0b00000000; //MCP3002 don'tcare byte
byte maskByte=0b00000011; //MCP3002: only the last 2bites of the 1srt byte are actual datas.

//syntax control: do not change
int counter;                                  
unsigned long address;
unsigned long stamp;
unsigned long time;

int digitalValue;

void setup()
{
 // wait for capa to fully load
 delay(2000);
 // Initialize serial port for com to host
 Serial.begin (1000000);
 Serial.flush();

 // Set the slave select pin as an output
 pinMode (ADCpin, OUTPUT);
 pinMode (RAMpin, OUTPUT);
 // Initialize SPI
 SPI.begin();
 SPI.setClockDivider(SPI_CLOCK_DIV2); //16/X Mhz clock, div2= max speed
 SPI.setBitOrder (MSBFIRST);          

 //timing optimization stuff, add more at will.
 ADCSRA=0; // turn off arduino's ADC
 

  /// /// /// /// /// Set sequential_mode/// /// /// /// /// ///
  digitalWrite(RAMpin, LOW);
     SPI.transfer(WRMR);
     SPI.transfer(SEQUENTIAL_MODE);
 digitalWrite(RAMpin, HIGH);

 
  digitalWrite(RAMpin, LOW);
     SPI.transfer(RDMR);
     TransfertMode=SPI.transfer(0x00);
 digitalWrite(RAMpin, HIGH);
 digitalWrite(ADCpin, HIGH);
   /// /// /// /// /// /// /// /// /// /// ///

}

void loop(){
counter=0;
address=0;
    /// /// /// /// AQUISITION AND SRAM TRANSFER /// /// /// /// 
//GET adc bytes, write to SRAM, time stamps are taken INSIDE while loop for higher precision
  while(counter <BufferSize){
           if (counter==0){stamp=micros();} //timeStamp 1
            counter++;
//GET 2 ADC BYTES
  digitalWrite(ADCpin, LOW);
     conversionMSB = SPI.transfer (byte1);
     conversionLSB = SPI.transfer (byte2);
  digitalWrite(ADCpin, HIGH);

// write 2 bytes in SRAM
  digitalWrite(RAMpin, LOW);
    SPI.transfer(WRITE);
    SPI.transfer((uint8_t)(address >> 16)&0xFF);
    SPI.transfer((uint8_t)(address >> 8)&0xFF);
    SPI.transfer((uint8_t)address);
    SPI.transfer(conversionMSB);
    SPI.transfer(conversionLSB);
  digitalWrite(RAMpin, HIGH);
address= address+2;
delayMicroseconds(ControlDelay);        // pauses. USED TO CONTROL SPECTRAL WIDTH vs RESOLUTION  
      if (counter==BufferSize){time=micros()-stamp;} //timeStamp2, better precision if taken here!

}

   /// /// /// /// /// SRAM TO PC TRANSFER/// /// /// /// /// ///
counter=0;
address=0;
//set SRAM readmode/address
digitalWrite(RAMpin, LOW);
  SPI.transfer(READ);
  SPI.transfer((uint8_t)(address >> 16)&0xFF);
  SPI.transfer((uint8_t)(address >> 8)&0xFF);
  SPI.transfer((uint8_t)address);
//read the whole SRAM data
  while(counter<BufferSize){
  counter++;
// read 2 bytes from SRAM
  conversionMSB2=SPI.transfer(0x00);
  conversionLSB2=SPI.transfer(0x00);       
// convert and send bytes
  conversionMSB2 &= maskByte;                               //keep only meaningful bits of byte
  digitalValue= (conversionMSB2 << 8) |  conversionLSB2;   //mix and match 2bytes
  Serial.println(digitalValue);
  }
digitalWrite(RAMpin, HIGH);

Serial.print(time); //timestamp
Serial.print("\t"); //buffer Terminator (for Processing)
}


// MCP3002 - 05/06/2016 V4
// Wiring ADC/SRAM
//    Pin 9 to CS SRAM
//    Pin 8 to CS ADC
//    Pin 11 to Din
//    Pin 12 to Dout
//    Pin 13 to CLK
//    Vdd/Vref: 5V arduino
//    Vss:      GND arduino
//    CH0: 10kOhm--Positive_BNC_waveform
//    GND_BNC_waveform: GND arduino
//
// Communication with the device starts when /CS line brought low.
// The first '1' bit transmitted is the start bit.
// The next 3 bits define
//    Single Ended Input / Differential Input
//    Ch0 / Ch1 if single ended input or polarity if differential
//    MSBF / LSBF (Most/Least Significan Bit First)
// The returned value is ten bits long so if the bit pattern transmitted
// is placed correctly, the two most significant bits will be sent back
// with the first transfer. the rest on the second transfer.
