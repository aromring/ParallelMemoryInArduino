# ParallelMemoryInArduino

This is a small library handling a parallel flash memory chip, SST39SF010A from SST, 128kB, attached to Arduino platform through shift registers and I/O expander. It is intended as a data collection device, single file at a time. Definitely, it is not a flash disk with functioning file system. In fact, there is no file system and subsequent batches of data are written sequentially. The library includes code for extending the chip's life via wear leveling. Think of an Arduino based measuring device you use to collect large amounts of data in the field, one at a time, after which it can offload the data to a computer via serial port.

Since this hardware implementation uses the MCP23008 I/O expander from Adafruit [https://www.adafruit.com/products/593] you  will have to download and install to your Arduino folder the associated library, Adafruit_MCP23008_RF, which is here: https://github.com/aromring/Adafruit_MCP23008_RF. 

This repository also inlcudes an Arduino sketch "Memory_SST39SF010A.ino" illustrating the library use. In short, the sketch reads previously written data consisting of an 20x4 array of random floats, generates and saves on the chip a new such array, and stops. The numbers are written to the Serial Monitor. The sketch has been tested with ATmega328 and ATmega2560.

