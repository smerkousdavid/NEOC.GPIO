#include <neo.h>
#include <time.h>

int main() {
	int ret;

	ret = neo_led_init();

	if(ret != NEO_OK) {
		printf("Failed initializing led!\n");
		return 1;
	}

	int i;
	//All methods have error checking
	ret = neo_led_set(0);
	sleep(1);

	if(ret != NEO_OK) {
		printf("Failed setting led!\n");
		return 1;
	}

	neo_led_set(1);

	for(i = 0; i < 20; i++) {
		neo_led_on();
		sleep(1);
		neo_led_off();
		sleep(1);
	}

	ret = neo_led_free();
	return ret;
}
