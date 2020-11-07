#include "YA_DHT22_M.h"

static Serial pc(SERIAL_TX, SERIAL_RX);

DigitalOut led(LED1);
 
int main() {
  pc.baud(115200);
  printf("Starting...\n");
  led = 1;
  for(;;) {
    // At least 1s between acquisitions (DHT 22 specs)
    wait_ms(3000);
    led = !led;
    int temp_10, humidity_10;
    int error = DHT22_acquisition::acquire(PC_3, &humidity_10, &temp_10);
    if (error) {
      printf("error %d\n", error);
      continue;
    }
    printf("T=%.1f\tH=%.1f\n", float(temp_10) * 0.1, float(humidity_10) * 0.1);
  }
}
