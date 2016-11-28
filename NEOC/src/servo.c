#include <neo.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

int neo_servo_pins[GPIOPORTSL][2];
struct timeval neo_last_call, neo_current_call;

unsigned short neo_servo_index = 0;

int neo_servo_init() {
	neo_gpio_init();
	
	int i;
	for(i = 0; i < GPIOPORTSL; i++) {
		neo_servo_pins[i][0] = -1;
		neo_servo_pins[i][1] = -1;
	}
	gettimeofday(&neo_last_call, 0);
}


int neo_servo_attach(int gpio) {
	if(gpio < 0 || gpio > GPIOPORTSL || neo_servo_index > GPIOPORTSL) return NEO_PIN_ERROR;
	neo_servo_pins[gpio][0] = gpio;
	neo_servo_index += 1;
	return neo_gpio_pin_mode(gpio, OUTPUT);
}

int neo_servo_detach(int gpio) {
	if(gpio < 0 || gpio > GPIOPORTSL || neo_servo_index > GPIOPORTSL) return NEO_PIN_ERROR;
	
	int i;
	//Double checking method, even though it's mapped to gpio you have to be safe
	for(i = 0; i < GPIOPORTSL; i++) {
		if(neo_servo_pins[i][0] == gpio) {
			neo_servo_pins[i][0] = -1;
			neo_servo_index -= 1;
		}
	}
	return NEO_OK;
}

int neo_servo_write(int gpio, int angle) {
	if(gpio < 0 || gpio > GPIOPORTSL)  return NEO_PIN_ERROR;
	if(angle < 0) angle = 0;
	else if(angle > 180) angle = 180;

	neo_servo_pins[gpio][1] = neo_re_map(angle, 0, 180, 1000, 2000); 
	return NEO_OK;
}

int neo_servo_refresh() {
	gettimeofday(&neo_current_call, 0);
	long diff = ((neo_current_call.tv_sec*1e6 + neo_current_call.tv_usec) 
		- (neo_last_call.tv_sec*1e6 + neo_last_call.tv_usec)) / 1000L; 
	if(diff < 20L) {if(usleep(1000L * (21L - diff)) != 0L) return NEO_DUTY_ERROR;}
	int neo_i;	
	for(neo_i = 0; neo_i < GPIOPORTSL; neo_i++) {
		if(neo_servo_pins[neo_i][0] != -1 && neo_servo_pins[neo_i][1] != -1) {
			neo_gpio_digital_write(neo_servo_pins[neo_i][0], HIGH);
			if(usleep(neo_servo_pins[neo_i][1]) != 0) break;
			neo_gpio_digital_write(neo_servo_pins[neo_i][0], LOW);
		}
	}
	gettimeofday(&neo_last_call, 0);
	return NEO_OK;
}


/*int main() {
	neo_servo_init();
	neo_servo_attach(13);
	neo_servo_write(13, 0);
	while(1) {
		neo_servo_refresh();
		usleep(1000 * 15);
	}
}*/
