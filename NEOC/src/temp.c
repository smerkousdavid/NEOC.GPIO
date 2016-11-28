#include <neo.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

FILE *neo_temp_controllerr;

unsigned char neo_temp_freed = 2;

int neo_temp_init() {
	if(neo_temp_freed == 2) {
		neo_check_root("The temperature module requires root!");
		//Restart the module
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
	
		neo_temp_controllerr = fopen(TEMPREAD, "r");
	
		if(neo_temp_controllerr == NULL) return NEO_UNUSABLE_ERROR;
		neo_temp_freed = 0;
	}
	return NEO_OK;
}

int neo_temp_read() {
	if(neo_temp_controllerr == NULL) return NEO_UNUSABLE_ERROR;

	int milliCelc = 0;

	fflush(neo_temp_controllerr);
	fseek(neo_temp_controllerr, 0, SEEK_SET);
	if(fscanf(neo_temp_controllerr, "%d", &milliCelc) == EOF) 
		return NEO_READ_ERROR;

	return milliCelc;
}

int neo_temp_free() {
	if(neo_temp_freed == 2) {
		if(neo_temp_controllerr == NULL) return NEO_UNUSABLE_ERROR;
		fclose(neo_temp_controllerr);
		neo_temp_freed = 0;
	}
	return NEO_OK;
}

/*
int main() {
	neo_temp_init();
	printf("TEMP: %d\n", neo_temp_read());
	neo_temp_free();
}*/
