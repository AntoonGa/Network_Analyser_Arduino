// Tested uning a 23LC1024 sRam chip in a 8 pin DIP package
// Test connections are...
// Chip UNO MEGA  NAME
//  1   9   9     SS    (Hardware SS Pin (10 or 53) needs to remain output no matter what other pin you may for SS)
//  2   12  50    MISO
//  3   NC  NC
//  4   GND GND   Vss
//  5   11  51    MOSI
//  6   13  52    SCK
//  7   +5V +5V   ~HOLD
//  8   +5V +5V   Vcc


#include <SPI.h>
#include <SpiRAM.h>

byte ramContents[256];
long int page = 0; // 256 byte pages
int totalErrors = 0;

SpiRAM sRam(53);     // Initialize the RAM

void setup(){
  randomSeed(0xff); // important that the same random numbers are produced from reset
  Serial.begin(115200);
  Serial.println("Ram Tests Begin.");
  
  SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void loop(){ 
    setContents();
    readContents();
    page += 256;
    if(page > 0x1ffff){
      Serial.print("One cycle complete ");
      Serial.print(totalErrors);
      Serial.println(" errors");
      totalErrors = 0;
      page = 0;
      delay(2000);
    }

}

void setContents(){
  for(int i =0; i<256; i++){ // write something to the memory
  ramContents[i] = random(0xff);
  ramWrite((long)i, ramContents[i]);
  }
}

void readContents(){
  byte value;
  boolean fault = false;
  for(int i =0; i<256; i++){ // read something from the memory
  value = ramRead((long)i); // read a byte
  if(ramContents[i] != value){
    fault = true;
    printReading((long)i, value, ramContents[i]);
    totalErrors++;
    }
  }
  if(!fault){
    Serial.print("verified cycle page ");
    Serial.println(page,HEX);
  }
}

byte ramRead(long add){ 
  add += page;
  return sRam.readByte(add);
}

void ramWrite(long add, byte val){ 
  add += page;
  sRam.writeByte(add, val);
}

void printReading(long add, int got, int should){
  Serial.print("At address ");
  Serial.print(add + page, HEX);
  Serial.print("  -  Read ");
  Serial.print(got, HEX);
  Serial.print("  -  should be ");
  Serial.println(should, HEX);
}

