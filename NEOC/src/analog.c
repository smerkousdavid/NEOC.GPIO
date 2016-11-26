#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


const char * const ANALOGPORTS[][2] = {{"0", "0"}, {"0", "1"}, {"0", "2"}, {"0", "3"},
							{"1", "0"}, {"1", "1"}}; 

unsigned char USABLEANALOG[GPIOPORTSL];
float  ANALOGSCALE[ANALOGSCALEL];

FILE* analogR[GPIOPORTSL];

int neo_analog_init() 
{
	int i, gi, fail;
	for(gi = 0; gi < ANALOGPORTSL; gi++) {USABLEANALOG[gi] = 1;}

#ifdef SCALEANALOG
	for(gi = 0; gi < ANALOGSCALEL; gi++) {ANALOGSCALE[gi] = 0.25;}
#endif

	fail = NEO_OK;

#ifdef SCALEANALOG	
	for(i = 0; i < ANALOGSCALEL; i++) { 
		size_t scaleSize = 10 + analogL + analogBL + analogSL;

		char buff[scaleSize];
		sprintf(buff, "%s%d%s%s", ANALOGPATHP, i, ANALOGBASEP, ANALOGSCALEP);

		FILE *sFile;
		sFile = fopen(buff, "r");

		if(sFile == NULL) {
			fail = NEO_EXPORT_ERROR;
			continue;
		}

		float curScale;
		curScale  = 0.25f;
		fscanf(sFile, "%f", &curScale);
		fclose(sFile);
		ANALOGSCALE[i] = curScale;
	}
#endif

	for(i = 0; i < ANALOGPORTSL; i++) {
		size_t rawSize = 3 + analogL + analogBL + analogRL;

		char buffR[rawSize];
		
		sprintf(buffR, "%s%s%s%s%s", ANALOGPATHP, ANALOGPORTS[i][0],
					ANALOGBASEP, ANALOGPORTS[i][1], ANALOGRAWP);
		
		analogR[i] = fopen(buffR, "r");

		if(analogR[i] == NULL) {
			fail = NEO_UNUSABLE_ERROR;
			USABLEANALOG[i] = 0;
		}
	}
	return fail;
}

float neo_analog_read(int pin) {
	if(pin < 0 || pin > ANALOGPORTSL) return NEO_PIN_ERROR;


	FILE *curR = analogR[pin];

	if(curR == NULL || !USABLEANALOG[pin]) return NEO_UNUSABLE_ERROR;

	float curRaw;
	
	fflush(curR); //Update buffer
	fseek(curR, 0, SEEK_SET);

	if(fscanf(curR, "%f", &curRaw) == EOF) return NEO_READ_ERROR;

#ifdef SCALEANALOG
	int curScale;
	if(sscanf(ANALOGPORTS[pin][0], "%d", &curScale) == EOF) return NEO_READ_ERROR;
	
	if(curScale < 0 || curScale > ANALOGSCALEL) return NEO_SCALE_ERROR;

	return curRaw * ANALOGSCALE[curScale];
#endif
	return curRaw;
}

int neo_analog_free() 
{
	int fail;
	int i;

	fail = NEO_OK;
	for(i = 0; i < ANALOGPORTSL; i++) {
		if(USABLEANALOG[i]) {
			FILE *curR = analogR[i];

			if(curR != NULL) fclose(curR);
			else fail = NEO_UNUSABLE_ERROR;
		}
	}
	return fail;
}

/*int main() {
	printf("INIT %d\n", neo_analog_init());
	while(1) {
		printf("MODE %f\n", neo_analog_read(0));
		sleep(1);	
	}
	printf("FREE %d\n", neo_analog_free());
	return 0;
}*/
