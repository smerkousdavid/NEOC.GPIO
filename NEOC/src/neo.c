#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

float neo_re_map(float value, float omin, float omax, float nmin, float nmax)
{
    return ((value - omin) / (omax - omin)) * (nmax - nmin) + nmin;
}

void neo_check_root(char const * saying) {
	if(geteuid() != 0) {
		fprintf(stderr, "No root privlegdes on UID(%d)! %s\n", geteuid(), saying);
		exit(1);
	}
}

int neo_disable_m4() {
	neo_check_root("Can't disable m4 without root!");
	return system("udoom4ctl disable") == 0;
}

int neo_enable_m4() {
	neo_check_root("Can't enable m4 without root!");
	return system("udoom4ctl enable") == 0;
}

int neo_screen_set_lvds() {
	neo_check_root("To set to lvds you must be root!");
	return system("udooscreenctl set lvds7") == 0;
}

int neo_screen_set_hdmi() {
	neo_check_root("To set to hdmi you must be root!");
	return system("udooscreenctl set hdmi") == 0;
}
