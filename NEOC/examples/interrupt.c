#include <neo.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void intsetup(int pin, int data) {
	printf("GOT RESULT: p: %d d: %d\n", pin, data);
}


int main() {
	neo_gpio_init();

	printf("%d\n",neo_gpio_attach_interrupt(13, "both", &intsetup));

	while(1) {
		sleep(1);
	}

}
