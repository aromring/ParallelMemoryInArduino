//
//  ParallelFlashMemory.cpp
//  ParallelFlashMemory
//
//  Created by Robert Fraczkiewicz on 7/17/16.
//
//  Copy this file into Arduino't libraries/ParallelFlashMemory folder
//

#include "ParallelFlashMemory.h"

void ParallelFlashMemory::begin (uint8_t lp, uint8_t cp, uint8_t ap, uint8_t wep, uint8_t oep, uint8_t cep, uint32_t ss, uint32_t ms, uint8_t mcpadd)
{
    latchPin=lp;
    clockPin=cp;
    addressPin=ap;
    wePin=wep;
    oePin=oep;
    cePin=cep;
    sectorSize=ss;
    memorySize=ms;
    mcp_I2C_Address=mcpadd;
    pinMode(cePin,OUTPUT);
    digitalWrite(cePin,HIGH);   // Immediately pull CE# pin high to preserve data
    pinMode(oePin,OUTPUT);
    pinMode(wePin,OUTPUT);
    digitalWrite(wePin,HIGH);
    
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(addressPin, OUTPUT);
    digitalWrite(latchPin, HIGH); //pull the latchPin to save the data
       
    mcp.begin(mcp_I2C_Address);
    
    LastSector=memorySize/sectorSize-1;
    EndLastSector=memorySize-1;
    StartLastSector=memorySize-sectorSize;
}

void ParallelFlashMemory::setAddress(uint32_t address)
{
    digitalWrite(latchPin, LOW); //ground latchPin and hold low for as long as you are transmitting
    shiftOut(addressPin, clockPin, MSBFIRST, (address >> 16) & 0x1);
    shiftOut(addressPin, clockPin, MSBFIRST, (address >> 8) & 0xFF);
    shiftOut(addressPin, clockPin, MSBFIRST, address & 0xFF);
    digitalWrite(latchPin, HIGH); //pull the latchPin to save the data
}

void ParallelFlashMemory::setDataAndAddress(uint8_t data,uint32_t address)
{
    mcp.allPinMode(OUTPUT);
    setAddress(address);
    digitalWrite(wePin,LOW);
    mcp.writeGPIO(data);
    delayMicroseconds(1);
    digitalWrite(wePin,HIGH);
}

uint8_t ParallelFlashMemory::readByteAtAddress(uint32_t address)
{
    uint8_t data;
    digitalWrite(wePin,HIGH);
    digitalWrite(cePin,LOW);
    digitalWrite(oePin,LOW);
    mcp.allPinMode(INPUT);
    setAddress(address);
    delayMicroseconds(1);
    data=mcp.readGPIO();
    digitalWrite(cePin,HIGH);
    return data;
}

void ParallelFlashMemory::chipErase()
{
    // Chip erase
    digitalWrite(cePin,LOW);
    digitalWrite(oePin,HIGH);
    setDataAndAddress(0xAA,0x5555);
    setDataAndAddress(0x55,0x2AAA);
    setDataAndAddress(0x80,0x5555);
    setDataAndAddress(0xAA,0x5555);
    setDataAndAddress(0x55,0x2AAA);
    setDataAndAddress(0x10,0x5555);
    delay(200);
    digitalWrite(cePin,HIGH);
}

// There are 32 4kB sectors, numbered from 0 to 31
void ParallelFlashMemory::sectorErase(uint32_t sectorNum)
{
    if(sectorNum>LastSector) return;
    // Sector erase
    digitalWrite(cePin,LOW);
    digitalWrite(oePin,HIGH);
    setDataAndAddress(0xAA,0x5555);
    setDataAndAddress(0x55,0x2AAA);
    setDataAndAddress(0x80,0x5555);
    setDataAndAddress(0xAA,0x5555);
    setDataAndAddress(0x55,0x2AAA);
    setDataAndAddress(0x30,sectorNum << 12);
    delay(50);
    digitalWrite(cePin,HIGH);
}

void ParallelFlashMemory::getSoftwareAndDeviceID(uint8_t &sID,uint8_t &dID)
{
    // Read software ID
    digitalWrite(cePin,LOW);
    digitalWrite(oePin,HIGH);
    digitalWrite(wePin,HIGH);
    setDataAndAddress(0xAA,0x5555);
    setDataAndAddress(0x55,0x2AAA);
    setDataAndAddress(0x90,0x5555);
    digitalWrite(cePin,HIGH);
    delayMicroseconds(20);
    digitalWrite(cePin,LOW);
    digitalWrite(oePin,LOW);
    sID = readByteAtAddress(0x0);
    dID = readByteAtAddress(0x1);
    digitalWrite(cePin,HIGH);
    delayMicroseconds(1);
    // Exit Software ID
    digitalWrite(wePin,HIGH);
    digitalWrite(cePin,LOW);
    digitalWrite(oePin,HIGH);
    setDataAndAddress(0xAA,0x5555);
    setDataAndAddress(0x55,0x2AAA);
    setDataAndAddress(0xF0,0x5555);
    digitalWrite(cePin,HIGH);
}

// WARNING: this fucntion assumes that the appropriate sector is already erased!
void ParallelFlashMemory::writeByteAtAddress(uint8_t data,uint32_t address)
{
    // Write one byte
    digitalWrite(cePin,LOW);
    digitalWrite(oePin,HIGH);
    setDataAndAddress(0xAA,0x5555);
    setDataAndAddress(0x55,0x2AAA);
    setDataAndAddress(0xA0,0x5555);
    setDataAndAddress(data,address);
    digitalWrite(cePin,HIGH);
    delayMicroseconds(20);
}

// Save the begin address of the last written file. The address is saved
// in the specially reserved "directory" area: the last 4096-byte sector of the
// flash drive (0x1F000-0x1FFFF) using the log-based system. In this system,
// subsequent addresses are appended one after another until they fill the
// entire sector. When that happens, the sector is erased and the next address
// is written at the first sector's byte (= 0x1F000).
// The address itself is composed of 3 bytes: first is always either 0x0 or 0x1,
// depending on whether it points to sectors 0x0-0xF or to sectors 0xF-0x1E. The
// next two bytes can assume any value from 0x0 to 0xFF. Thus, it is easy to search
// for the last address by starting at 0x0 every 3 bytes and stopping at the first
// empty memory cell (which is indicated by the default 0xFF value).
void ParallelFlashMemory::saveAddress(uint32_t address)
{
    uint32_t i;
    uint8_t *addrPtr,data0,data1,data2;
    
    bool sector_full=true;
    // Start search for the next empty address
    for(i=StartLastSector+2;i<EndLastSector;i+=3) {
        data0=readByteAtAddress(i-2);
        data1=readByteAtAddress(i-1);
        data2=readByteAtAddress(i);
        if(0xFF==data2 && 0xFF==data1 && 0xFF==data0) {
            // Remember: ATmega328 stores integers LSB first!
            addrPtr=reinterpret_cast<uint8_t *>(&address);
            data0=*(addrPtr++);
            data1=*(addrPtr++);
            data2=*addrPtr;
            writeByteAtAddress(data0,i-2);
            writeByteAtAddress(data1,i-1);
            writeByteAtAddress(data2,i);
            sector_full=false;
            break;
        }
    }
    if(sector_full) {
        sectorErase(LastSector);
        // Remember: ATmega328 stores integers LSB first!
        addrPtr=reinterpret_cast<uint8_t *>(&address);
        data0=*(addrPtr++);
        data1=*(addrPtr++);
        data2=*addrPtr;
        writeByteAtAddress(*(addrPtr++),StartLastSector);
        writeByteAtAddress(*(addrPtr++),StartLastSector+1);
        writeByteAtAddress(*addrPtr,StartLastSector+2);
    }
}

// Read the begin address of the last written file. The addresses are saved
// in the specially reserved "directory" area: the last 4096-byte sector of the
// flash drive (0x1F000-0x1FFFF) using the log-based system.
// The address itself is composed of 3 bytes: first is always either 0x0 or 0x1,
// depending on whether it points to sectors 0x0-0xF or to sectors 0xF-0x1E. The
// next two bytes can assume any value from 0x0 to 0xFF. Thus, it is easy to search
// for the last address by starting at 0x0 every 3 bytes and stopping at the first
// empty memory cell (which is indicated by the default 0xFF value).
uint32_t ParallelFlashMemory::readLastAddress()
{
    uint32_t address=0x0;
    uint32_t i;
    uint8_t *addrPtr,data0,data1,data2,data0old=0x0,data1old=0x0,data2old=0x0;
    
    // Start search for the next empty address
    for(i=StartLastSector+2;i<EndLastSector;i+=3) {
        data0=readByteAtAddress(i-2);
        data1=readByteAtAddress(i-1);
        data2=readByteAtAddress(i);
        if(0xFF==data2 && 0xFF==data1 && 0xFF==data0) {
            break;
        }
        data0old=data0;
        data1old=data1;
        data2old=data2;
    }
    // Remember: ATmega328 stores integers LSB first!
    addrPtr=reinterpret_cast<uint8_t *>(&address);
    *(addrPtr++)=data0old;
    *(addrPtr++)=data1old;
    *addrPtr=data2old;
    return address;
}

// Save the value of a long (4 bytes on ATmega328) to memory chip and
// return the address for writing the next, adjacent long. The latter,
// however, is "smart" in the sense of: 1) detecting whether the address
// is the begining of a new sector and erasing this sector if yes; 2) wrapping
// the address back to 0x0 if the upper memory limit has been reached
uint32_t ParallelFlashMemory::writeLongAtAddress(uint32_t l,uint32_t address)
{
    uint32_t *nv=&l,newAddress,currentSector,newSector;
    currentSector=address/sectorSize;
    newAddress=(address+SIZE_OF_LONG)%StartLastSector;
    newSector=newAddress/sectorSize;
    if(0==address%sectorSize) sectorErase(currentSector);
    else if(currentSector!=newSector && newAddress%sectorSize>0) sectorErase(newSector);
    uint8_t x,i;
    for(i=0;i<SIZE_OF_LONG;++i) {
        x=(*nv) >> 8*i;
        writeByteAtAddress(x,(address+i)%StartLastSector);
    }
    return newAddress;
}

// Read saved value of a long from memory chip
uint32_t ParallelFlashMemory::readLongAtAddress(uint32_t address)
{
    uint32_t vn=0;
    int i;
    uint8_t x;
    for(i=SIZE_OF_LONG-1;i>=0;--i) {
        x=readByteAtAddress((address+i)%StartLastSector);
        vn+=x;
        if(i>0) vn=(vn<<8);
    }
    return vn;
}

// Convert 4-byte long to 4-byte float in bitwise manner
float ParallelFlashMemory::longToFloat(uint32_t l)
{
    float *pf;
    pf=reinterpret_cast<float *>(&l);
    return *pf;
    
}
// Convert 4-byte float to 4-byte long in bitwise manner
uint32_t ParallelFlashMemory::floatToLong(float f)
{
    uint32_t *nv;
    nv=reinterpret_cast<uint32_t *>(&f);
    return *nv;
}

// Save the value of a float (4 bytes on ATmega328) to memory chip
uint32_t ParallelFlashMemory::writeFloatAtAddress(float f,uint32_t address)
{
    return writeLongAtAddress(floatToLong(f),address);
}
uint32_t ParallelFlashMemory::writeFloatArrayAtAddress(float *a,uint32_t n,uint32_t address)
{
    if(n<=0) return address;
    uint32_t j;
    for(j=0;j<n;++j) {
        address=writeFloatAtAddress(a[j],address);
    }
    return address;
}

// Read saved value of a float from memory chip
float ParallelFlashMemory::readFloatAtAddress(uint32_t address)
{
    return longToFloat(readLongAtAddress(address));
}

uint32_t ParallelFlashMemory::readFloatArrayAtAddress(float *a,uint32_t n,uint32_t address)
{
    if(n<=0) return address;
    uint32_t j;
    for(j=0;j<n;++j) {
        a[j]=readFloatAtAddress(address);
        address=(address+SIZE_OF_LONG)%StartLastSector;
    }
    return address;
}


void ParallelFlashMemory::printByteAndDistributedAddress(uint8_t data,uint32_t address)
{
    Serial.print("Data: ");
    Serial.print(data,BIN);
    Serial.print("\tA0-A7: ");
    Serial.print(address & 0xFF,BIN);
    Serial.print("\tA8-A15: ");
    Serial.print((address >> 8) & 0xFF, BIN);
    Serial.print("\tA16: ");
    Serial.println((address >> 16) & 0x1,BIN);
}

void ParallelFlashMemory::dumpSector(uint32_t sectorNum)
{
    if(sectorNum>LastSector) return;
    uint32_t addr;
    uint32_t sectorBeginAddress=sectorSize*sectorNum,
            sectorEndAddress=sectorBeginAddress+sectorSize;
    Serial.print("Contents of sector ");
    Serial.println(sectorNum);
    Serial.println("Address\tByte");
    for(addr=sectorBeginAddress;addr<=sectorEndAddress;++addr) {
        Serial.print(addr,HEX);
        Serial.print("\t");
        Serial.println(readByteAtAddress(addr),HEX);
   }
}