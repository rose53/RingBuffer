#RingBuffer#

Implementation of a [Ring buffer (Circular buffer)](https://en.wikipedia.org/wiki/Circular_buffer) for the
[Adafruit I2C FRAM breakout](http://www.adafruit.com/products/1895).

The implementation uses the [Adafruit I2C FRAM Driver](https://github.com/adafruit/Adafruit_FRAM_I2C) fo the I2C communication with the FRAM.

## Available functions ##

To work with the ring buffer, it has to be formatted with a page size. The page size defines the number of bytes one page has in the buffer.

### begin ###
Initializes the the buffer and the driver to the FRAM

### format ###
Formats the ring buffer. After formating, the buffer is empty and the page size is set.

### write ###
Writes one page to the buffer. If the successful, _true_ will be returned, otherwise _false_.

### read ###
Reads one page from the buffer. If the successful, _true_ will be returned, otherwise _false_.

### getPageSize ###
Gets the page size.

### getHead ###
Returns the address in the memory next used for page writing.

### getTail ###
Returns the address in the memory next used for page reading.

###isInitialized ###
Returns _true_, if the ring buffer is initialized (formatted).

### isFull ###
Returns _true_, if the ring buffer is full. A call to _write_ will also return _false_.

###containsData	###
Returns _true_, if there is some data in the ring buffer. A call to _resd_ will be successful.

### dump ###
Dumps the complete memory.


