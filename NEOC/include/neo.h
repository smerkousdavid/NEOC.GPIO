#ifndef NEOC_H
#define NEOC_H

#define GPIOPORTSL 48
#define PWMPORTSL 7
#define ANALOGPORTSL 5
#define ANALOGSCALEL 2

#define EXPORTPATH "/sys/class/gpio/export"
#define GPIOPATH "/sys/class/gpio/gpio"
#define VALUEPATH "/value"
#define DIRECTIONPATH "/direction"

#define PWMEXPORTPATH "/sys/class/pwm/pwmchip0/export"
#define PWMPATH "/sys/class/pwm/pwmchip0/pwm"
#define PWMPERIOD "/period"
#define PWMDUTY "/duty_cycle"
#define PWMENABLE "/enable"

#define MAXFAKEPWM 47
#define PWMMUTEXRATE 0.00010f

#define ANALOGPATHP "/sys/bus/iio/devices/iio\:device"
#define ANALOGBASEP ((const char *) "/in_voltage")
#define ANALOGRAWP "_raw"
#define ANALOGSCALEP "_scale"
#define ANALOGHIGH 4095
#define ANALOGLOW 0
#define ANALOGBIT 12


#define LEDPATH "/sys/class/leds/led0/brightness"

//To scale the analog based on board specs set to true
//Auto find scale will be done
//#define SCALEANALOG

#define NEO_OK 0
#define NEO_PIN_ERROR -2
#define NEO_PERIOD_ERROR -3
#define NEO_DUTY_ERROR -4
#define NEO_UNUSABLE_ERROR -5
#define NEO_EXPORT_ERROR -6
#define NEO_UNUSABLE_EXPORT_ERROR -7
#define NEO_DIR_ERROR -8
#define NEO_READ_ERROR -9
#define NEO_SCALE_ERROR -10
#define NEO_LED_STATE_ERROR -11

#define ACCELBASE "/sys/class/misc/FreescaleAccelerometer/"
#define ACCELENABLE ACCELBASE"enable"
#define ACCELDATA ACCELBASE"data"
#define ACCELPOLLP ACCELBASE"poll_delay"
#define ACCELPOLL 20

#define GYROBASE "/sys/class/misc/FreescaleGyroscope/"
#define GYROENABLE GYROBASE"enable"
#define GYRODATA GYROBASE"data"
#define GYROPOLLP GYROBASE"poll_delay"
#define GYROPOLL 20

#define MAGNOBASE "/sys/class/misc/FreescaleMagnetometer/"
#define MAGNOENABLE MAGNOBASE"enable"
#define MAGNODATA MAGNOBASE"data"
#define MAGNOPOLLP MAGNOBASE"poll_delay"
#define MAGNOPOLL 20

#define TEMPINIT "lm75 0x48"
#define TEMPPATH "/sys/class/i2c-dev/i2c-1/device/new_device"
#define TEMPREAD "/sys/class/i2c-dev/i2c-1/device/1-0048/temp1_input"


#define OUTPUT 1
#define INPUT 0
#define HIGH 1
#define LOW 0

#include <string.h>
#include <stdio.h>

extern const char * const GPIOPORTS[];
extern unsigned char USABLEGPIO[];
extern const char * const PWMPORTS[];
extern unsigned char USABLEPWM[];
extern const char * const ANALOGPORTS[][2];
extern unsigned char USABLEANALOG[];
extern float ANALOGSCALE[];


#define exportL strlen(EXPORTPATH)
#define gpioL  strlen(GPIOPATH)
#define valueL strlen(VALUEPATH)
#define directionL strlen(DIRECTIONPATH)
#define pwmexportL strlen(PWMEXPORTPATH)
#define pwmL strlen(PWMPATH)
#define pwmPeriod strlen(PWMPERIOD)
#define pwmDuty strlen(PWMDUTY)
#define pwmEnable strlen(PWMENABLE)
#define analogL strlen(ANALOGPATHP)
#define analogBL strlen(ANALOGBASEP)
#define analogRL strlen(ANALOGRAWP)
#define analogSL strlen(ANALOGSCALEP)

float neo_re_map(float, float, float, float, float);
void neo_check_root(char const *);
int neo_enable_m4();
int neo_disable_m4();

//extern unsigned int neo_pwm_period = 2040816;
//extern unsigned int neo_pwm_duty = 50;

int neo_gpio_init();
int neo_gpio_pin_mode(int, int);
int neo_gpio_digital_write_no_safety(int*, int);
int neo_gpio_digital_write(int, int);
int neo_gpio_free();

int neo_pwm_init();
int neo_pwm_set_period(int, int);
int neo_pwm_set_period_all(int);
int neo_pwm_write(int, int);
int neo_pwm_fake_write(int, int);
int neo_pwm_free();

int neo_analog_init();
float neo_analog_read(int);
int neo_analog_free();

int neo_led_init();
int neo_led_set(int);
int neo_led_on();
int neo_led_off();

#endif
