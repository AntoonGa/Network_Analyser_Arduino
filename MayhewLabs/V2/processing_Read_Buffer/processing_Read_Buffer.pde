//mind over matter!
//This program is just here to create the buffer files 1,2...,9.txt

import processing.serial.*; 
Serial myPort;


int bps=1000000; //in BITS/s must be the same as arduino

double time=0;
int tic=0;    //used to change the name of the output FILE.TXT so outside program can read.
String filename="1.txt";  
String Temp_buffer="init";

PrintWriter output;                     
 
 
void setup() {
  //printArray(Serial.list());
  myPort= new Serial(this, Serial.list()[0], bps); //in BITS/s
  myPort.bufferUntil('\t');    //horizontal tab terminator calls the serialEvent
  myPort.clear();              // Clears Processing buffer.

}
 
void draw() {}

//Event: Processing buffer is read when Terminator (\t: newline) is sent from arduino Serial.
void serialEvent(Serial myPort) {          
  tic++;                                        //filename goes from 1.txt to 9.txt  
  if (tic==10) {tic = 1;}
  filename = String.valueOf(tic) + ".txt";
  output = createWriter(filename); 

 
  while (myPort.available() > 0) {
    Temp_buffer=myPort.readString();
     if (Temp_buffer != null) {
 
          output.println(Temp_buffer);
     }
     
        output.flush();                         // Writes the remaining data to the file
        output.close();                         // Finishes the file 
        myPort.clear();                         // Clears Processing buffer.
        
        time=millis()-time;
        print(filename);
        print(" : ");
        print(time);
        println(" ms");
        time=millis();

      }
 

}