//
// Yet another DHT22 acquisition routine.
// This one is for MBED OS (5)
// The API is blocking so we keep it simple, but the core is interrupt-based, 
// (c) Ch. Tronche (ch@tronche.com) 2020
// MIT Licence (basically you do what you want... At your own risks).
// Absolutely no warranty of anything.
// Version 0.1.0
//

#include <mbed.h>

#ifndef _YA_DHT22_
#define _YA_DHT22_

class DHT22_acquisition {
public: 

// This is what you call. Acquires data. returns 0 if ok, or a negative
// number on error.  If there is no error, temp_10 is the temperature
// in tenth of celsius, humidity_10 is the pressure in tenth of %.

  static int acquire(PinName pin_, int *humidity_10, int *temp_10);

private:
  DHT22_acquisition(): _last(0) { }

  int getByte();

  static void rise_cb(DHT22_acquisition *);
  static void fall_cb(DHT22_acquisition *);
  static inline int getEdge(Queue<int,16>&);

private:
  Timer _t;
  Queue<int,16> _queue; // comm between ISR and calling thread
  int _last; // last timer value (to clock line state duration)

};  

#endif // _YA_DHT22_
