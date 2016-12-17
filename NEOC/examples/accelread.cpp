#include <neo.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

using namespace neo;

int main() {
  Accel::init(true); //Throw errors if any
  Accel::setPoll(30); //Set the update rate to 30 milliseconds
  Accel::calibrate(100, 30); //3 seconds calibration (100 samples at 30 millis delays)
  while(1) {
     int x, y, z; //Hold the accel vals
     Accel::read(&x, &y, &z); //Update the values
     printf("X: %d, Y: %d, Z: %d\n", x, y, z); //Print the updates
     usleep(1000 * 30); //30 millis delay
  }
  return 0; //Auto free on exit
}
