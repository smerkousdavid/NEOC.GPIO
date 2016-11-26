#include <neo.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

char attached = 0;

int servoPins[GPIOPORTSL][2];
struct timeval lastCall, curCall;

unsigned short ind = 0;

int neo_servo_init() {
	neo_gpio_init();
	
	int i;
	for(i = 0; i < GPIOPORTSL; i++) {
		servoPins[i][0] = -1;
		servoPins[i][1] = -1;
	}
	gettimeofday(&lastCall, 0);
}


int neo_servo_attach(int gpio) {
	if(gpio < 0 || gpio > GPIOPORTSL || ind > GPIOPORTSL) return NEO_PIN_ERROR;
	servoPins[gpio][0] = gpio;
	ind += 1;
	return neo_gpio_pin_mode(gpio, OUTPUT);
}

int neo_servo_detach(int gpio) {
	if(gpio < 0 || gpio > GPIOPORTSL || ind > GPIOPORTSL) return NEO_PIN_ERROR;
	
	int i;
	//Double checking method, even though it's mapped to gpio you have to be safe
	for(i = 0; i < GPIOPORTSL; i++) {
		if(servoPins[i][0] == gpio) {
			servoPins[i][0] = -1;
			ind -= 1;
		}
	}
	return NEO_OK;
}

int neo_servo_write(int gpio, int angle) {
	if(gpio < 0 || gpio > GPIOPORTSL)  return NEO_PIN_ERROR;
	if(angle < 0) angle = 0;
	else if(angle > 180) angle = 180;

	servoPins[gpio][1] = neo_re_map(angle, 0, 180, 1000, 2000); 
	return NEO_OK;
}

int neo_servo_refresh() {
	gettimeofday(&curCall, 0);
	long diff = ((curCall.tv_sec*1e6 + curCall.tv_usec) 
		- (lastCall.tv_sec*1e6 + lastCall.tv_usec)) / 1000L; 
	if(diff < 20L) {if(usleep(1000L * (21L - diff)) != 0L) return NEO_DUTY_ERROR;}
	int i;	
	for(i = 0; i < GPIOPORTSL; i++) {
		if(servoPins[i][0] != -1 && servoPins[i][1] != -1) {
			neo_gpio_digital_write(servoPins[i][0], HIGH);
			if(usleep(servoPins[i][1]) != 0) break;
			neo_gpio_digital_write(servoPins[i][0], LOW);
		}
	}
	gettimeofday(&lastCall, 0);
	return NEO_OK;
}


int main() {
	neo_servo_init();
	neo_servo_attach(13);
	neo_servo_write(13, 0);
	while(1) {
		neo_servo_refresh();
		usleep(1000 * 15);
	}
}
