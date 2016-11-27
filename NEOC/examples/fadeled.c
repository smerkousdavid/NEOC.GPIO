#include <neo.h>
#include <time.h>

int main() {
	neo_gpio_init();
	neo_gpio_pin_mode(13, OUTPUT);
	neo_fake_pwm_write(13, 40);
	sleep(5);
	neo_gpio_free();
	printf("HEY\n");
	return 0;
}
