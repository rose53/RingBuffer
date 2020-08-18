
#include "RingBuffer.h"

#define RAM_SIZE 32 * 1024

union twobyte_t {
  uint16_t word;
  uint8_t  byte[2];
};

union fourbyte_t {
  uint32_t dword;
  uint8_t  byte[4];
};

union bootSector_t {
  uint32_t dword[8];
  uint16_t word[16];
  uint8_t  byte[32];
};

const int      RingBuffer::bootSectorSize   = 32;
const uint16_t RingBuffer::bootSectorStart  = RAM_SIZE - bootSectorSize;
const uint32_t RingBuffer::magicBytes       = 0xFEEDC0DE;
const uint8_t  RingBuffer::headOffset       = 4;
const uint8_t  RingBuffer::tailOffset       = 6;
const uint8_t  RingBuffer::pageSizeOffset   = 8;
const uint8_t  RingBuffer::emptyMarkerOffset = 10;

const uint8_t  RingBuffer::startData       = 0;

RingBuffer::RingBuffer() : fram() {	
}

boolean RingBuffer::begin(uint8_t addr) { 
	boolean retVal = fram.begin(addr); 
	if (!retVal) {
		return retVal;
	}	
	return true;
}

void RingBuffer::format(uint16_t pageSize) {

    for (uint16_t i = 0; i < RAM_SIZE; i++) {
      fram.write8(i,0x00);
    }
    
	union fourbyte_t fourbyte;
	fourbyte.dword = magicBytes;
    
	union twobyte_t twobyte;
    twobyte.word = startData;
	
    fram.write8(bootSectorStart, fourbyte.byte[0]);
    fram.write8(bootSectorStart + 1, fourbyte.byte[1]);
    fram.write8(bootSectorStart + 2, fourbyte.byte[2]);
    fram.write8(bootSectorStart + 3, fourbyte.byte[3]);
	// head and tail are both pointing to the start of the data
    fram.write8(bootSectorStart + headOffset, twobyte.byte[0]);
    fram.write8(bootSectorStart + headOffset + 1, twobyte.byte[1]);
    fram.write8(bootSectorStart + tailOffset, twobyte.byte[0]);
    fram.write8(bootSectorStart + tailOffset + 1, twobyte.byte[1]);

	// save the page size
	twobyte.word = pageSize;
    fram.write8(bootSectorStart + pageSizeOffset, twobyte.byte[0]);
    fram.write8(bootSectorStart + pageSizeOffset + 1, twobyte.byte[1]);
	
	// flip marker
	fram.write8(bootSectorStart + emptyMarkerOffset, 0xFF);
}

uint32_t RingBuffer::readUint32(uint16_t framAddr) {
	union fourbyte_t fourbyte;
	fourbyte.byte[0] = fram.read8(framAddr);
	fourbyte.byte[1] = fram.read8(framAddr + 1);
	fourbyte.byte[2] = fram.read8(framAddr + 2);
	fourbyte.byte[3] = fram.read8(framAddr + 3);
	return fourbyte.dword;	
}

uint16_t RingBuffer::readUint16(uint16_t framAddr) {
	union twobyte_t twobyte;
	twobyte.byte[0] = fram.read8(framAddr);
	twobyte.byte[1] = fram.read8(framAddr + 1);
	return twobyte.word;	
}

void RingBuffer::writeUint16(uint16_t framAddr, uint16_t value) {
	union twobyte_t twobyte;
	twobyte.word = value;	
	fram.write8(framAddr, twobyte.byte[0]);
    fram.write8(framAddr + 1, twobyte.byte[1]);
}

/*
 * Writes the page to the buffer and increments the head pointer
 */
void RingBuffer::writePage(uint16_t framAddr, uint16_t pageSize, uint8_t page[], size_t size) {
	
  for (int i = 0; i < min(size,pageSize); i++) {
    fram.write8(framAddr + i, page[i]);
  }
  setEmpty(false);
}

/*
 * Read the page to the buffer and increments the tail pointer
 */
void RingBuffer::readPage(uint16_t framAddr, uint16_t pageSize, uint8_t page[], size_t size) {
	
	for (int i = 0; i < min(size,pageSize); i++) {
		page[i] = fram.read8(framAddr + i);
	}
	setTail(framAddr + pageSize);
}


void RingBuffer::write(uint8_t page[], size_t size) {
	
	uint16_t head = getHead();	
	uint16_t tail = getTail();	
	uint16_t pageSize = getPageSize();
    
    uint16_t nextHead = head + pageSize >= bootSectorStart?startData:head + pageSize;

    if (!isEmpty() && head == tail ) {
        setTail(nextHead);
    }
    writePage(head,pageSize,page,size);
    
    setHead(nextHead);
}

boolean RingBuffer::read(uint8_t page[], size_t size) {

    if (isEmpty()) {
        return false;
    }

    uint16_t head = getHead();    
    uint16_t tail = getTail();  
    uint16_t pageSize = getPageSize();

    uint16_t nextTail = tail + pageSize >= bootSectorStart?startData:tail + pageSize;

    readPage(tail,pageSize,page,size);

    setTail(nextTail);
    if (nextTail == head) {
        setEmpty(true);
    }
    return true;
}

void RingBuffer::dump(Stream& stream) {
  uint8_t value;
  for (uint16_t a = 0; a < RAM_SIZE; a++) {
    value = fram.read8(a);
    if ((a % 32) == 0) {
      stream.println(); printHex16(&a,1,stream); stream.print(": ");
    }
    printHex8(&value,1,stream); 
  }
  stream.println();
}

void RingBuffer::dumpBootsector(Stream& stream) {
	union bootSector_t bootSector;	
	
	for (uint16_t a = bootSectorStart; a < bootSectorStart + bootSectorSize; a++) {
      bootSector.byte[a - bootSectorStart] = fram.read8(a);
    }

	uint32_t *magic       = bootSector.dword;
    uint16_t *head        = bootSector.word + 2;
    uint16_t *tail        = bootSector.word + 3;
	uint16_t *pageSize    = bootSector.word + 4;
	uint8_t  *emptyMarker = bootSector.byte + 10;
	
	stream.print("Magic\t\t");    stream.print("0x"); stream.println(*magic, HEX);
    stream.print("Head\t\t");     printHex16(head,1,stream);       stream.println();
    stream.print("Tail\t\t");     printHex16(tail,1,stream);       stream.println();
	stream.print("Pagesize\t");   printHex16(pageSize,1,stream);   stream.println();
	stream.print("Empty\t\t");    printHex8(emptyMarker,1,stream); stream.println();
}

void RingBuffer::printHex8(uint8_t *data, uint8_t length, Stream& stream) {
  char tmp[16];
  for (int i=0; i<length; i++) {
    sprintf(tmp, "0x%.2X",data[i]);
    stream.print(tmp); Serial.print(" ");
  }
}

void RingBuffer::printHex16(uint16_t *data, uint8_t length, Stream& stream){
  char tmp[16];
  for (int i=0; i<length; i++) {
    sprintf(tmp, "0x%.4X",data[i]);
    stream.print(tmp); Serial.print(" ");
  }
}
