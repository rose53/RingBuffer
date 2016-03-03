
#include "RingBuffer.h"

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
const uint16_t RingBuffer::bootSectorStart  = 32 * 1024 - bootSectorSize;
const uint8_t  RingBuffer::headOffset       = 4;
const uint8_t  RingBuffer::tailOffset       = 6;
const uint8_t  RingBuffer::pageSizeOffset   = 8;
const uint8_t  RingBuffer::flipMarkerOffset = 10;

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
	union fourbyte_t fourbyte;
	fourbyte.dword = 0xFEEDC0DE;
    
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
	fram.write8(bootSectorStart + flipMarkerOffset, 0x00);
}

uint16_t RingBuffer::readUint16(uint16_t framAddr) 
{
	union twobyte_t twobyte;
	twobyte.byte[0] = fram.read8(framAddr);
	twobyte.byte[1] = fram.read8(framAddr + 1);
	return twobyte.word;	
}

void RingBuffer::writeUint16(uint16_t framAddr, uint16_t value) 
{
	union twobyte_t twobyte;
	twobyte.word = value;	
	fram.write8(framAddr, twobyte.byte[0]);
    fram.write8(framAddr + 1, twobyte.byte[1]);
}

void RingBuffer::writePage(uint16_t framAddr, uint8_t page[], size_t size) {
	
	for (int i = 0; i < size; i++) {
		fram.write8(framAddr + i, page[i]);
	}
}

void RingBuffer::readPage(uint16_t framAddr, uint8_t page[], size_t size) {
	
	for (int i = 0; i < size; i++) {
		page[i] = fram.read8(framAddr + i);
	}
}

boolean RingBuffer::write(uint8_t page[], size_t size) {
	
	uint16_t head = getHead();	
	uint16_t tail = getTail();	
	uint16_t pageSize = getPageSize();
	uint8_t  flipMarker = getFlipMarker();
	
	// check, if we wrapped 
	if (flipMarker == 0) {
		if (head + pageSize > bootSectorStart) {
			// we reached the end of the memory, go to start and flip marker
			head = startData;
			flipMarker = 0xFF;
			setHead(head);
			setFlipMarker(flipMarker);
			if (head + pageSize < tail) {
				// there is some space, write the page
				writePage(head,page,min(size,pageSize));
				setHead(head + pageSize);
				return true;
			} else {
				// we have no space left 
				return false;
			}
		} else {
			// there is some space, write the page
			writePage(head,page,min(size,pageSize));
			setHead(head + pageSize);
			return true;
		}
	} else {
		if (head < tail) {
			// we have some sapace left
			writePage(head,page,min(size,pageSize));
			setHead(head + pageSize);
			return true;
		} else {
			// we have no space left 
			return false;
		}
	}	
}

boolean RingBuffer::read(uint8_t page[], size_t size) {
	
	uint16_t head = getHead();	
	uint16_t tail = getTail();	
	uint16_t pageSize = getPageSize();
	uint8_t  flipMarker = getFlipMarker();
	
	if (flipMarker == 0) {
		if (tail < head) {
			readPage(tail,page,min(size,pageSize));
			setTail(tail + pageSize);
			return true;
		} else {
			return false;
		}
	} else {
		if (tail + pageSize > bootSectorStart) {
			tail = startData;
			flipMarker = 0x00;
			setTail(tail);
			setFlipMarker(flipMarker);
			if (tail < head) {
				readPage(tail,page,min(size,pageSize));
				setTail(tail + pageSize);
				return true;
			} else {
				return false;
			}
		} else {
			readPage(tail,page,min(size,pageSize));
			setTail(tail + pageSize);
			return true;
		}
	}
}

void RingBuffer::dump(Stream& stream) {
	uint8_t value;
	for (uint16_t a = 0; a < 32768; a++) {
		value = fram.read8(a);
    if ((a % 32) == 0) {
      stream.print("\n 0x"); stream.print(a, HEX); stream.print(": ");
    }
    stream.print("0x"); 
    if (value < 0x0F) 
      stream.print('0');
    stream.print(value, HEX); stream.print(" ");
  }
}
void RingBuffer::dumpBootsector(Stream& stream) {
	union bootSector_t bootSector;	
	
	for (uint16_t a = bootSectorStart; a < bootSectorStart + bootSectorSize; a++) {
      bootSector.byte[a - bootSectorStart] = fram.read8(a);
    }

	uint32_t *magic      = bootSector.dword;
    uint16_t *head       = bootSector.word + 2;
    uint16_t *tail       = bootSector.word + 3;
	uint16_t *pageSize   = bootSector.word + 4;
	uint8_t  *flipMarker = bootSector.byte + 11;
	
	stream.println(*magic, HEX);
    stream.println(*head, HEX);
    stream.println(*tail, HEX);
	stream.println(*pageSize, HEX);
	stream.println(*flipMarker, HEX);
}