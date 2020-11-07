//
// Yet another DHT22 acquisition routine.
// This one is for MBED OS (5)
// The API is blocking so we keep it simple, but the core is interrupt-based, 
// (c) Ch. Tronche (ch@tronche.com) 2020
// MIT Licence (basically you do what you want... At your own risks).
// Absolutely no warranty of anything.
// Version 0.1.0
//

#include "YA_DHT22_M.h"

void DHT22_acquisition::fall_cb(DHT22_acquisition *acq) {
  int now = acq->_t.read_us();
  acq->_queue.put((int *)(now - acq->_last));
  acq->_last = now;
}

void DHT22_acquisition::rise_cb(DHT22_acquisition *acq) {
  int now = acq->_t.read_us();
  acq->_queue.put((int *)(acq->_last - now));  // negative for rise (that is, it was a low state)
  acq->_last = now;
}

static const uint32_t TIMEOUT = 10; // ms

// Return the duration of the line state when an edge is
// reached. Negative if it was low state, positive if the state was
// high, 0 on error.

int DHT22_acquisition::getEdge(Queue<int,16> &queue) {
  osEvent ev = queue.get(TIMEOUT);
  if (ev.status != osEventMessage) return 0;
  return ev.value.v;
}

// Return the next byte in the range 0-255. Return a negative value on error.

int DHT22_acquisition::getByte() {
  uint8_t byte = 0;
  for(int bitNumber = 8; bitNumber; --bitNumber) {
    // line is pulled low by the DHT to synchronize for next bit
    int v = getEdge(_queue);
    if (!v) return -2;
    if (v > -25 || v < -75) return -8; // Problem with bit
				       // synchronization (low state should last 50 us)

    // DHT then transmits bit, duration is 26 to 28 us for 0, 70us for 1.
    v = getEdge(_queue);
    if (v <= 0) return -8; // problem
    byte <<= 1;
    if (v > 50) // bit 1
      byte |= 0x1;
  }
  return byte;
}

// Acquire data. return 0 if ok, or a negative number on error.
// If there is no error, temp_10 is the temperature in tenth of
// celsius, humidity_10 is the pressure in tenth of %.

int DHT22_acquisition::acquire(PinName pin_, int *humidity_10, int *temp_10) {
  DHT22_acquisition acq;
  DigitalInOut pin(pin_);
  InterruptIn interrupt(pin_);

  pin.output();
  pin = 0;
  wait_ms(18);
  pin = 1;

  acq._t.start();
  interrupt.fall(callback(fall_cb, &acq));
  interrupt.rise(callback(rise_cb, &acq));

  wait_us(25);
  pin.input();

  int v = getEdge(acq._queue);
  if (v <= 0)  return -3; // We missed the fall (if there was one)

  v = getEdge(acq._queue);
  if (v > -60) return -5; // We missed the rise (include the case 0 = error)

  v = getEdge(acq._queue);
  if (!v) return -6;
  if (v < 65) return -7; // We missed the fall

  uint8_t data[5];
  for(int byteNumber = 0; byteNumber <= 4; ++byteNumber) {
    int res = acq.getByte();
    if (res < 0) return res;
    data[byteNumber] = res;
  }
  interrupt.fall(NULL);
  interrupt.rise(NULL);
  acq._t.stop();
  
  uint8_t checksum = data[0] + data[1] + data[2] + data[3];
  if (checksum != data[4]) return -9;

  *humidity_10 = (data[0] << 8) | data[1];

  int temp = (data[2] & 0x7f << 8) | data[3];
  if (data[2] & 0x80) temp = -temp;
  *temp_10 = temp;

  return 0;
}
