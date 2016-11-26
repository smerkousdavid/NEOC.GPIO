#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *controller;

int neo_led_init() {
	controller = fopen(LEDPATH, "w");
	if(controller == NULL) return NEO_UNUSABLE_ERROR;
	return NEO_OK;
}

int neo_led_set(int state) {
	if(controller == NULL) return NEO_UNUSABLE_ERROR;
	if(state < 0 || state > 1) return NEO_LED_STATE_ERROR;

	fflush(controller); //Clear
	fseek(controller, 0, SEEK_SET);
	fprintf(controller, "%d", state);
	fflush(controller);
	return NEO_OK;
}

int neo_led_on() {
	return neo_led_set(1);
}

int neo_led_off() {
	return neo_led_set(0);
}

int neo_led_free() {
	if(controller == NULL) return NEO_UNUSABLE_ERROR;	
	fclose(controller);
	return NEO_OK;
}
