#include "RingBuffer.h"

#define PAGE_SIZE 16

uint8_t writePage[PAGE_SIZE];
uint8_t readPage[PAGE_SIZE];


RingBuffer ringBuffer = RingBuffer();

void setup() {
  Serial.begin(115200);

  ringBuffer.begin();

  boolean initialized = ringBuffer.isInitialized();
  if (!initialized) {
    Serial.print("RingBuffer initializing...");
    ringBuffer.format(PAGE_SIZE);
    Serial.println("done.");
  }
  Serial.print("RingBuffer is full  : ");Serial.println(ringBuffer.isFull(),BIN);
  Serial.print("RingBuffer is empty : ");Serial.println(ringBuffer.isEmpty(),BIN);

  if (!ringBuffer.isEmpty()) {
    while (ringBuffer.read(writePage,sizeof(writePage)));

    Serial.print("RingBuffer is full       : ");Serial.println(ringBuffer.isFull(),BIN);
  }
  for (int i = 0; i < PAGE_SIZE; i++) {
    writePage[i] = i;
  }
  Serial.print("Writing one page to the RingBuffer ...");
  ringBuffer.write(writePage,sizeof(writePage));
  Serial.println("done.");

  Serial.print("RingBuffer is full       : ");Serial.println(ringBuffer.isFull(),BIN);

  Serial.print("Reading one page from the RingBuffer ...");

  ringBuffer.read(writePage,sizeof(writePage));
  Serial.println("done.");
  for (int i = 0; i < PAGE_SIZE; i++) {
    Serial.print("0x");
    if (writePage[i] <= 0x0F)
      Serial.print("0");
      Serial.print(writePage[i],HEX);Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:

}
