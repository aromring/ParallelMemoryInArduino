# ParallelMemoryInArduino

A small library handling a parallel flash memory chip attached to Arduino platform through shift registers. It is intended as a data collection device, single file at a time. Definitely, it is not a flash disk with functioning file system. In fact, there is no file system, but the library includes code for extending the chip's life via wear leveling. Think of an Arduino based measuring device you use to collect large amounts of data in the field, one time, after which it can offload the data to a computer via serial port.
