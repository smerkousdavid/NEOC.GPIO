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

#define MAXFAKEPWM 5
#define PWMMUTEXRATE 0.00010f

#define ANALOGPATHP "/sys/bus/iio/devices/iio\:device"
#define ANALOGBASEP ((const char *) "/in_voltage")
#define ANALOGRAWP "_raw"
#define ANALOGSCALEP "_scale"
#define ANALOGHIGH 4095
#define ANALOGLOW 0
#define ANALOGBIT 12

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

#define OUTPUT 1
#define INPUT 0
#define HIGH 1
#define LOW 0

#include <string.h>
#include <cstring>
#include <stdio.h>

extern const char * const GPIOPORTS[];
extern unsigned char USABLEGPIO[];
extern const char * const PWMPORTS[];
extern unsigned char USABLEPWM[];
extern const char * const ANALOGPORTS[][2];
extern unsigned char USABLEANALOG[];
extern float ANALOGSCALE[];


const size_t exportL = strlen(EXPORTPATH);
const size_t gpioL = strlen(GPIOPATH);
const size_t valueL = strlen(VALUEPATH);
const size_t directionL = strlen(DIRECTIONPATH);
const size_t pwmexportL = strlen(PWMEXPORTPATH);
const size_t pwmL = strlen(PWMPATH);
const size_t pwmPeriod = strlen(PWMPERIOD);
const size_t pwmDuty = strlen(PWMDUTY);
const size_t pwmEnable = strlen(PWMENABLE);
const size_t analogL = strlen(ANALOGPATHP);
const size_t analogBL = strlen(ANALOGBASEP);
const size_t analogRL = strlen(ANALOGRAWP);
const size_t analogSL = strlen(ANALOGSCALEP);

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

#endif
