//
//  ParallelFlashMemory.h
//  ParallelFlashMemory
//
//  Created by Robert Fraczkiewicz on 7/17/16.
//
//  Copy this file into Arduino't libraries/ParallelFlashMemory folder
//
//

#ifndef ParallelFlashMemory_h
#define ParallelFlashMemory_h

#include <Arduino.h>
#include "../Adafruit_MCP23008_RF/Adafruit_MCP23008_RF.h"

#define SIZE_OF_LONG    4   // Valid for Atmega328

class ParallelFlashMemory {
private:
    // Data members
    uint8_t latchPin;   //Pin connected to ST_CP/RCLK of 74HC595
    uint8_t clockPin;   //Pin connected to SH_CP/SRCLK of 74HC595
    uint8_t addressPin; //Pin connected to DS of 74HC595
    uint8_t wePin;  // Pin connected to WE# of SST39SF010A
    uint8_t oePin;  // Pin connected to OE# of SST39SF010A
    uint8_t cePin;  // Pin connected to CE# of SST39SF010A
    uint8_t mcp_I2C_Address;    // I2C address of the I/O extender
    uint32_t sectorSize;    // Size of erasable sector in bytes
    uint32_t memorySize;    // Total size of memory in bytes
    uint32_t LastSector;    // Index of the last sector starting from 0
    uint32_t StartLastSector;   // Memory address of the first byte in the last sector counting from 0
    uint32_t EndLastSector;    // Memory address of the last byte in the last sector counting from 0
    Adafruit_MCP23008_RF mcp;  // Object for an I/O extender over I2C
    // Functions
    void setAddress(uint32_t address);
    void setDataAndAddress(uint8_t data,uint32_t address);
    void writeByteAtAddress(uint8_t data,uint32_t address);
    uint8_t readByteAtAddress(uint32_t address);
public:
    // Setup
    void begin(uint8_t lp, uint8_t cp, uint8_t ap, uint8_t wep, uint8_t oep, uint8_t cep, uint32_t ss, uint32_t ms,  uint8_t mcpadd);
    // Erase
    void chipErase();
    void sectorErase(uint32_t sectorNum);
    // Info
    void getSoftwareAndDeviceID(uint8_t &sID,uint8_t &dID);
    inline uint32_t getStartLastSector() {return StartLastSector;}
    // Bitwise (!) conversion
    float longToFloat(uint32_t l);
    uint32_t floatToLong(float f);
    // Read & write
    uint32_t writeLongAtAddress(uint32_t l,uint32_t address);
    uint32_t readLongAtAddress(uint32_t address);
    uint32_t writeFloatAtAddress(float f,uint32_t address);
    uint32_t writeFloatArrayAtAddress(float *a,uint32_t n,uint32_t address);
    float readFloatAtAddress(uint32_t address);
    uint32_t readFloatArrayAtAddress(float *a,uint32_t n,uint32_t address);
    void saveAddress(uint32_t address);
    uint32_t readLastAddress();
    // Debug
    void printByteAndDistributedAddress(uint8_t data,uint32_t address);
    void dumpSector(uint32_t sectorNum)
    ;
};

#endif /* ParallelFlashMemory_h */
