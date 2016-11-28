#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *neo_led_controller;

unsigned char neo_led_freed = 2;

int neo_led_init() {
	if(neo_led_freed == 2) {
		neo_led_controller = fopen(LEDPATH, "w");
		if(neo_led_controller == NULL) return NEO_UNUSABLE_ERROR;
		neo_led_freed = 0;
	}
	return NEO_OK;
}

int neo_led_set(int state) {
	if(neo_led_controller == NULL) return NEO_UNUSABLE_ERROR;
	if(state < 0 || state > 1) return NEO_LED_STATE_ERROR;

	fflush(neo_led_controller); //Clear
	fseek(neo_led_controller, 0, SEEK_SET);
	fprintf(neo_led_controller, "%d", state);
	fflush(neo_led_controller);
	return NEO_OK;
}

int neo_led_on() {
	return neo_led_set(1);
}

int neo_led_off() {
	return neo_led_set(0);
}

int neo_led_free() {
	if(neo_led_freed == 0) {
		if(neo_led_controller == NULL) return NEO_UNUSABLE_ERROR;	
		fclose(neo_led_controller);
		neo_led_freed = 2;
	}
	return NEO_OK;
}
