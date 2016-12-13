#include <neo.h>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

using namespace neo; 
using namespace std;
//Main function
int main() {
    std::unique_ptr<PWM> pwm;

    cout << "Initializing PWM" << endl;

    try {
       pwm.reset(new PWM(0)); //Start on PWM pin 0 (pwm 5) (NOT THE SAME AS GPIO MAPPING)      
    } catch(error::UnusableError &err) {
      cout << "Failed initializing PWM: " << err.what() << endl;
    }
    pwm->setPeriod(20408); //Example set period
    while(1) {
      //Fade up to 255
      for(int i = 0; i < 255; i++) {
         pwm->write(i); //Write up pwm value
         this_thread::sleep_for(chrono::milliseconds(10));
      }
      
      //Fade down back to 0
      for(int d = 255; d > 0; d--) {
         pwm->write(d); //Write down pwm value
         this_thread::sleep_for(chrono::milliseconds(10));
      }
         this_thread::sleep_for(chrono::milliseconds(400));
    }
    cout << "Wrote to PWM" << endl;

    return 0;
}
