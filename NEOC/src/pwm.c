#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


const char * const PWMPORTS[] = {"0", "1", "2", "3", "4", "5", "6"};
unsigned char USABLEPWM[] = {0, 0, 0, 0, 0, 0, 0};

int FAKEPWMLIST[MAXFAKEPWM + 2][2];

FILE* pwmP[PWMPORTSL + 2];
FILE* pwmD[PWMPORTSL + 2];
FILE* pwmE[PWMPORTSL + 2];

unsigned int neo_pwm_period = 20408;
unsigned int neo_pwm_duty = 50;

int neo_pwm_counting = 0;

unsigned char neo_pwm_freed = 2;

pthread_t fakePWMT[MAXFAKEPWM + 2];

struct params {
	pthread_mutex_t mutex;
	pthread_cond_t done;
	int pin, high, low, updatePeriod;
};

typedef struct params params_t;

params_t threadProps[MAXFAKEPWM + 2];


int neo_pwm_init()
{
	int i;
	int fail;

	fail = NEO_OK;

	if(neo_pwm_freed == 2) {
		FILE *eFile;
		eFile = fopen(PWMEXPORTPATH, "w");

		for(i = 0; i < PWMPORTSL; i++) {
			if(eFile != NULL) {
				fprintf(eFile, "%s", PWMPORTS[i]);
				fflush(eFile);
				USABLEPWM[i] = 1;
			} else fail = NEO_EXPORT_ERROR;
	
			size_t pwmSize = strlen(PWMPORTS[i]) + pwmL + 5;
			size_t newPathS = pwmSize + pwmPeriod;
			size_t newDPathS = pwmSize + pwmDuty;
			size_t newEPathS = pwmSize + pwmEnable;
	
			char buffP[newPathS];
			char buffD[newDPathS];
			char buffE[newEPathS];
	
			sprintf(buffP, "%s%s%s", PWMPATH, PWMPORTS[i], PWMPERIOD);
			sprintf(buffD, "%s%s%s", PWMPATH, PWMPORTS[i], PWMDUTY);
			sprintf(buffE, "%s%s%s", PWMPATH, PWMPORTS[i], PWMENABLE);
	
			pwmP[i] = fopen(buffP, "w");
	
			if(pwmP[i] != NULL) {
				fprintf(pwmP[i], "%d", neo_pwm_period);
				fflush(pwmP[i]);
			} else { 
				fail = (fail == NEO_EXPORT_ERROR || fail == NEO_UNUSABLE_EXPORT_ERROR) ? 
						NEO_UNUSABLE_EXPORT_ERROR : NEO_UNUSABLE_ERROR; 
				USABLEPWM[i] = 0;
			}
	
			pwmD[i] = fopen(buffD, "w");
			pwmE[i] = fopen(buffE, "w");
	
			if(pwmD[i] == NULL || pwmE[i] == NULL) {
				fail = (fail == NEO_EXPORT_ERROR || fail == NEO_UNUSABLE_EXPORT_ERROR) ? 
						NEO_UNUSABLE_EXPORT_ERROR : NEO_UNUSABLE_ERROR; 
				USABLEPWM[i] = 0;
	  
			}
	
		}
		neo_pwm_freed = 0;	
	}
	return fail;
}

void neo_sync_pwm(void *arg, int *pin, int *high, int *low,
		int *updatePeriod) {
	pthread_mutex_lock(&(*(params_t*)(arg)).mutex);

	params_t sets = (*(params_t*)(arg));

	*pin = sets.pin;
	*high = sets.high;
	*low = sets.low;
	*updatePeriod = sets.updatePeriod;

	pthread_mutex_unlock(&(*(params_t*)(arg)).mutex);
	//pthread_cond_signal (&(*(params_t*)(arg)).done);
}

void *pwmManager(void *arg){
	int pin, high, low;
	
	unsigned long long counter;
	
	int updatePeriod;

	//Initial sync
	neo_sync_pwm(arg, &pin, &high, &low, &updatePeriod);
	
	printf("PORT: %d\n", pin);

	int retM = neo_gpio_pin_mode(pin, OUTPUT);
	int remR = neo_gpio_digital_write(pin, 0);

	if(retM != NEO_OK) {
		printf("FAKE PWM FAILURE, COULDN'T EXPORT PIN!\n\n");
		return NULL;
	}

	if(remR != NEO_OK) {
		printf("FAKE PWM FAILURE, COULDN'T LOWER PIN");
		return NULL;
	}
	printf("STARTING FAKE PWM MANAGER ON PIN: %d\n", pin);
	counter = 0;
	while(1) {
		if(counter > updatePeriod) {
			neo_sync_pwm(arg, &pin, &high, &low, &updatePeriod);
			counter = 0;
		}
		counter++;
		neo_gpio_digital_write_no_safety(&pin, HIGH);		
		if(usleep(high) != 0) break;
		neo_gpio_digital_write_no_safety(&pin, LOW);
		if(usleep(low) != 0) break;
	}
}

//DOCS: THIS JUST SETS UP FAKE PWM
//SAME AS GPIO
int neo_fake_pwm_init() {
	return neo_gpio_init();
}

int neo_fake_pwm_write_period(int gpioPin, int period, int duty) {
	int i;
	char passed;

	if(gpioPin < 0 || gpioPin > GPIOPORTSL) return NEO_PIN_ERROR;
	
	if(duty < 0 || duty > 255) return NEO_DUTY_ERROR;

	int remap = (int) neo_re_map((float)duty, 0.0f, 255.0f, 0.0f,(float)period); 
	int lower = (period - remap);
	int upPeriod = (int)(PWMMUTEXRATE * ((float)period)); 

	passed = 1;
	for(i = 0; i < MAXFAKEPWM; i++) {
		if(FAKEPWMLIST[i] != NULL) { 
			if(FAKEPWMLIST[i][0] == gpioPin) {
				passed = 0;					

				params_t paramL = threadProps[i]; 
			

				//pthread_mutex_lock(&paramL.mutex);

				paramL.pin = gpioPin;
				paramL.high = remap;
				paramL.low = lower;
				paramL.updatePeriod = upPeriod; //20 Cycles

				FAKEPWMLIST[i][0] = gpioPin;
				FAKEPWMLIST[i][1] = i;

				threadProps[i] = paramL;

				//pthread_cond_wait (&paramL.done, &paramL.mutex);
				//pthread_mutex_unlock(&paramL.mutex);
			}
		}
	}

	if(passed) {
		int curI = neo_pwm_counting;
		if(curI > MAXFAKEPWM) return NEO_EXPORT_ERROR;
		
		params_t initP;
		initP.pin = gpioPin;
		initP.high = remap;
		initP.low = lower;
		initP.updatePeriod = upPeriod;

		threadProps[curI] = initP;	

		pthread_mutex_init(&threadProps[curI].mutex , NULL);
		//pthread_cond_init(&threadProps[curI].done, NULL);
		//pthread_mutex_lock(&threadProps[curI].mutex);

		pthread_create(&fakePWMT[curI], NULL, pwmManager, &threadProps[curI]);
		//pthread_cond_wait (&threadProps[curI].done, &threadProps[curI].mutex);
		FAKEPWMLIST[curI][0] = gpioPin;
		FAKEPWMLIST[curI][1] = curI;	
		neo_pwm_counting++;
	}
	return NEO_OK;
}

int neo_fake_pwm_write(int gpioPin, int duty) {
	return neo_fake_pwm_write_period(gpioPin, neo_pwm_period, duty);
}

int neo_pwm_write(int pin, int duty) {
	if(pin < 0 || pin > PWMPORTSL) return NEO_PIN_ERROR;
	if(duty < 0 || duty > 255) return NEO_DUTY_ERROR; 

	FILE *curD = pwmD[pin];
	FILE *curE = pwmE[pin];

	if(curD == NULL || curE == NULL || !USABLEPWM[pin]) return NEO_UNUSABLE_ERROR;

	if(duty == 0) {
		fprintf(curE, "%d", 0); //Disable pwm
		fflush(curE);
	} else {
		int remap = (int) neo_re_map((float)duty, 0.0f, 255.0f, 0.0f,(float)neo_pwm_period); 
		fprintf(curE, "%d", 1);
		fflush(curE);
		
		fprintf(curD, "%d", remap);
		fflush(curD);
	}
	return NEO_OK;
}

int neo_pwm_set_period_all(int period) {
	int i;
	int fail;

	fail = NEO_OK;
	for(i = 0; i < PWMPORTSL; i++) {
		if(USABLEPWM[i] == 1) {
			int errCode = neo_pwm_set_period(i, period);
			if(errCode != NEO_OK) fail = errCode;
		}		
	}

	return fail;
}

int neo_pwm_set_period(int pin, int period) {
	if(pin < 0 || pin > PWMPORTSL) return NEO_PIN_ERROR;

	if(period < 0 || period > 1000000000) return NEO_PERIOD_ERROR; 

	FILE *curP = pwmP[pin];
	
	if(curP == NULL || !USABLEPWM[pin]) return NEO_UNUSABLE_ERROR;

	neo_pwm_period = period;
	fprintf(curP, "%d", period);
	fflush(curP);
	return NEO_OK;	
}

int neo_pwm_free()
{
	int fail;
	int i;

	fail = NEO_OK;
	
	if(neo_pwm_freed == 0) {
		for(i = 0; i < PWMPORTSL; i++) {
			if(USABLEPWM[i]) {
				FILE *curP = pwmP[i];
				FILE *curD = pwmD[i];
				FILE *curE = pwmE[i];
	
				if(curP != NULL) fclose(curP);
				else fail = NEO_UNUSABLE_ERROR;
	
				if(curD != NULL) fclose(curD);
				else fail = NEO_UNUSABLE_ERROR;
	
				if(curE != NULL) fclose(curE);
				else fail = NEO_UNUSABLE_ERROR;
			}
		}
		neo_pwm_freed = 2;
	}
	return fail;
}

/*int main() {
	neo_gpio_init();
	neo_fake_pwm_write(12, 127);
	neo_fake_pwm_write(11, 30);
	neo_fake_pwm_write(2, 50);
	neo_fake_pwm_write(3, 100);
	neo_fake_pwm_write(4, 100);
	neo_fake_pwm_write(5, 100);
	int a;
	for(a = 16; a < 37; a++) neo_fake_pwm_write(a, 200);	

	while(1) {
		int i;
		for(i = 0; i < 255; i++) {
			neo_fake_pwm_write(13, i);
			usleep(1000*10);
		}
		int d;
		for(d = 255; d > 0; d--) {
			neo_fake_pwm_write(13, d);
			usleep(1000*10);
		}
		usleep(1000*400);
	}
	neo_gpio_free();
	return 0;
}*/
