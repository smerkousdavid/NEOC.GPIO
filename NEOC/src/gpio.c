#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

const char * const GPIOPORTS[] = {"178", "179", "104", "143", "142", "141", "140", 
			"149", "105", "148", "146", "147", "100", "102",
			"102", "106", "106", "107", "180", "181", "172", 
			"173", "182", "124", "25", "22", "14", "15", "16",
			"17", "18", "19", "20", "21", "203", "202", "177", 
			"176", "175", "174", "119", "124", "127", "116", 
			"7", "6", "5", "4"};

unsigned char USABLEGPIO[GPIOPORTSL];

FILE* gpioP[GPIOPORTSL];
FILE* gpioD[GPIOPORTSL];

unsigned char neo_gpio_freed = 2;

int neo_gpio_init() 
{
	int i, gi, fail;
	fail = NEO_OK;

	if(neo_gpio_freed == 2) {
		for(gi = 0; gi < GPIOPORTSL; gi++) {USABLEGPIO[gi] = 1;}
	
		FILE *eFile;
		eFile = fopen(EXPORTPATH, "w");
		
		if(eFile == NULL) fail = NEO_EXPORT_ERROR;
	
		for(i = 0; i < GPIOPORTSL; i++) { 
	
			if(eFile != NULL) {
				fprintf(eFile, "%s", GPIOPORTS[i]);
				fflush(eFile);
			}
	
			size_t newPathS = strlen(GPIOPORTS[i]) + gpioL + valueL;
			size_t newDPathS = strlen(GPIOPORTS[i]) + gpioL + directionL;
	
			char buff[newPathS];
			char buffD[newDPathS];
	
			sprintf(buff, "%s%s%s", GPIOPATH, GPIOPORTS[i], VALUEPATH);
			sprintf(buffD, "%s%s%s", GPIOPATH, GPIOPORTS[i], DIRECTIONPATH);
	
			gpioP[i] = fopen(buff, "r+");
			gpioD[i] = fopen(buffD, "r+");
	
			
			if(gpioP[i] == NULL || gpioD[i] == NULL) {
				fail = NEO_UNUSABLE_ERROR;
				USABLEGPIO[i] = 0;
			}
	
		}
		neo_gpio_freed = 0;
	}

	return fail;
}

int neo_gpio_pin_mode(int pin, int direction) {
	if(direction < 0 || direction > 1) return NEO_DIR_ERROR;
	
	if(pin < 0 || pin > GPIOPORTSL) return NEO_PIN_ERROR;

	FILE *curD = gpioD[pin];
	if(curD == NULL || !USABLEGPIO[pin]) return NEO_UNUSABLE_ERROR;
	fseek(curD, 0, SEEK_SET);
	fprintf(curD, "%s", (direction == 0) ? "in" : "out");
	fflush(curD);
	
	return NEO_OK;
}

int neo_gpio_digital_write_no_safety(int *pin, int direction) {
	char toPut[2];
	FILE *curP = gpioP[(*pin)];

	fseek(curP, 0, SEEK_SET);
	sprintf(toPut, "%d", direction);
	fputc(toPut[0], curP);
	fflush(curP);
}

int neo_gpio_digital_write(int pin, int direction) {
	if(direction < 0 || direction > 1) return NEO_DIR_ERROR;
	
	if(pin < 0 || pin > GPIOPORTSL) return NEO_PIN_ERROR;


	FILE *curP = gpioP[pin];

	if(curP == NULL || !USABLEGPIO[pin]) return NEO_UNUSABLE_ERROR;

	neo_gpio_digital_write_no_safety(&pin, direction);

	return NEO_OK;
}

int neo_gpio_digital_read(int pin) {
	if(pin < 0 || pin > GPIOPORTSL) return NEO_PIN_ERROR;

	FILE *curP = gpioP[pin];

	if(curP == NULL || !USABLEGPIO[pin]) return NEO_UNUSABLE_ERROR;

	int dir;
	fflush(curP); //Update buffer
	fseek(curP, 0, SEEK_SET);
	if(fscanf(gpioP[pin], "%d", &dir) == EOF) return NEO_READ_ERROR;
	return dir;
}

int neo_gpio_free() 
{
	int fail;
	int i;

	fail = NEO_OK;

	if(neo_gpio_freed == 0) {

		for(i = 0; i < GPIOPORTSL; i++) {
			if(USABLEGPIO[i]) {
				FILE *curP = gpioP[i];
				FILE *curD = gpioD[i];
	
				if(curP != NULL) fclose(curP);
				else fail = NEO_UNUSABLE_ERROR;
				if(curD != NULL) fclose(curD);
				else fail = NEO_UNUSABLE_ERROR;
			}
		}
		neo_gpio_freed = 2;
	}
	
	return fail;
}

/*
int main(int argc, char ** argv) {
	printf("INIT\n");
	neo_gpio_init();
	printf("MODE\n");
	neo_gpio_pin_mode(13, OUTPUT);
	printf("WRITE");
	int i;
	sscanf(argv[1], "%d", &i);
	while(1) {
		neo_gpio_digital_write(13, HIGH);
		usleep(50);
		neo_gpio_digital_write(13, LOW);
		usleep(5000);
	}
	//sleep(1);
	//neo_gpio_digital_write(13, LOW);
	//sleep(1)r
	neo_gpio_pin_mode(12, INPUT);
	printf("Mode: %d\n", neo_gpio_digital_read(13));
	neo_gpio_free();
	return 0;
}*/
