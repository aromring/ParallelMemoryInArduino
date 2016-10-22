// An illustrative example on how to use the 
// ParallelFlashMemory library. This short program
// reads the address of the begining of the last
// written file - a 20 rows X 4 columns table of
// random floats, then proceeds to generating a
// new random table of the same size writing it
// into a new sequential file.
// By Robert Fraczkiewicz, 10/21/2016
// All chips are powered by 5V and use 5V logic!

#include <Wire.h>
#include "Adafruit_MCP23008_RF.h"
#include "ParallelFlashMemory.h"

// Only 6 Arduino pins + 2 I2C pins are needed to connect everything 
const int latchPin = 8;//Pin connected to ST_CP/RCLK of 74HC595
const int clockPin = 7;//Pin connected to SH_CP/SRCLK of 74HC595
const int addressPin = 6; //Pin connected to DS/SER of 74HC595
const int wePin = 5; // Pin connected to WE# of SST39SF010A
const int oePin = 4; // Pin connected to OE# of SST39SF010A
const int cePin = 3; // Pin connected to CE# of SST39SF010A


void setup ()
{
  uint32_t nextItem;
  float out[4];
  
  Serial.begin(57600);  // start serial for output
  
  /* Initialize Parallel Flash Memory object
   * Arguments are, in order:
   * lp = Pin connected to ST_CP/RCLK of 74HC595
   * cp = Pin connected to SH_CP/SRCLK of 74HC595
   * ap = Pin connected to DS/SER of 74HC595
   * wep = Pin connected to WE# of SST39SF010A
   * oep = Pin connected to OE# of SST39SF010A
   * cep = Pin connected to CE# of SST39SF010A
   * ss = Size of erasable sector in bytes
   * ms = Total size of memory in bytes
   * mcpadd = I2C address of the I/O extender MCP23008
   */
  ParallelFlashMemory pfm;
  pfm.begin(latchPin,clockPin,addressPin,wePin,oePin,cePin,4096,131072,1);

  // Read the address of the last file
  nextItem=pfm.readLastAddress();
  
  if(Serial) {
    Serial.print(F("Last saved data at address: "));
    Serial.println(nextItem,HEX);
    Serial.println(F("Col1\tCol2\tCol3\tCol4"));
  }
  
  // Write the contents of the last saved file
  memset(out,0,4*sizeof(float));
  nextItem=pfm.readFloatArrayAtAddress(out,4,nextItem);
  while(0xFFFFFFFF!=pfm.floatToLong(out[0])) {
    if(Serial) {
      Serial.print(out[0],4);
      Serial.print("\t");
      Serial.print(out[1],4);
      Serial.print("\t");
      Serial.print(out[2],4);
      Serial.print("\t");
      Serial.println(out[3],4);
    }
    nextItem=pfm.readFloatArrayAtAddress(out,4,nextItem);
  }
  
  // Generate new file
  // Write begin address for the new file
  pfm.saveAddress(nextItem);
  int i,j;
  // Write 20 rows of 4 random floats
  for(i=0;i<20;++i) {
    for(j=0;j<4;++j) out[j]=rand()/1000.0;
    nextItem=pfm.writeFloatArrayAtAddress(out,4,nextItem);
  }
  // Write the end-of-file long
  pfm.writeLongAtAddress(0xFFFFFFFF,nextItem); // 0xFFFFFFFF indicates the end of file
}

void loop() {}

