#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#if ARDUINO >= 100
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include <Adafruit_FRAM_I2C.h>

class RingBuffer {
	
 public:
  RingBuffer(void);
  
  boolean begin(uint8_t addr = MB85RC_DEFAULT_ADDRESS);
  
  void format(uint16_t pageSize);
  
  void    write(uint8_t page[], size_t size);
  boolean read(uint8_t page[], size_t size);
  
  inline uint16_t getPageSize()   { return readUint16(bootSectorStart + pageSizeOffset);   }

  
  inline uint16_t getHead()  { return readUint16(bootSectorStart + headOffset);            }
  inline uint16_t getTail()  { return readUint16(bootSectorStart + tailOffset);            }
  inline boolean  isEmpty()  { return fram.read8(bootSectorStart + emptyMarkerOffset) > 0; }
  inline boolean  isFull()   { return !isEmpty() && getHead() == getTail();                }     

  void dump(Stream& stream);
  void dumpBootsector(Stream& stream);
  
  inline boolean isInitialized()  { return magicBytes == readUint32(bootSectorStart); }
  
  
  
 private:
 
  inline void setHead(uint16_t head)            {  writeUint16(bootSectorStart + headOffset,head);             }
  inline void setTail(uint16_t tail)            {  writeUint16(bootSectorStart + tailOffset,tail);             }
  inline void setEmpty(boolean empty)           {  fram.write8(bootSectorStart + emptyMarkerOffset,empty?0xFF:0x00); }

  uint16_t readUint16(uint16_t framAddr);
  void     writeUint16(uint16_t framAddr, uint16_t value);
  void     writePage(uint16_t framAddr, uint16_t pageSize, uint8_t page[], size_t size);
  void     readPage(uint16_t framAddr, uint16_t pageSize, uint8_t page[], size_t size);
  uint32_t readUint32(uint16_t framAddr);

  void printHex8(uint8_t *data, uint8_t length, Stream& stream);
  void printHex16(uint16_t *data, uint8_t length, Stream& stream);
  
 private:
 
  static const uint16_t bootSectorStart;
  static const int      bootSectorSize;
  static const uint32_t magicBytes;
  static const uint8_t  headOffset;
  static const uint8_t  tailOffset;
  static const uint8_t  pageSizeOffset;
  static const uint8_t  emptyMarkerOffset;

  static const uint8_t  startData;
 
  Adafruit_FRAM_I2C fram;
};

#endif
