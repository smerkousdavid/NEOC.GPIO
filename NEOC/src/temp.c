#include <neo.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

FILE *tempSensor;

int neo_temp_init() {
	neo_check_root("The temperature module requires root!");
	//REstart the module
	system("rmmod lm75");
	system("modprobe lm75");

	FILE* creator;
	creator = fopen(TEMPPATH, "w");

	if(creator != NULL) {
		fseek(creator, 0, SEEK_SET);
		fprintf(creator, "%s", TEMPINIT);
		fflush(creator);
		fclose(creator);
	}

	tempSensor = fopen(TEMPREAD, "r");

	if(tempSensor == NULL) return NEO_UNUSABLE_ERROR;
	
	return NEO_OK;
}

int neo_temp_read() {
	if(tempSensor == NULL) return NEO_UNUSABLE_ERROR;

	int milliCelc = 0;

	fflush(tempSensor);
	fseek(tempSensor, 0, SEEK_SET);
	if(fscanf(tempSensor, "%d", &milliCelc) == EOF) return NEO_READ_ERROR;

	return milliCelc;
}

int neo_temp_free() {
	if(tempSensor == NULL) return NEO_UNUSABLE_ERROR;
	fclose(tempSensor);
	return NEO_OK;
}

/*
int main() {
	neo_temp_init();
	printf("TEMP: %d\n", neo_temp_read());
	neo_temp_free();
}*/
