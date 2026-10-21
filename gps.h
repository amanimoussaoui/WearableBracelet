#pragma once
#include <Arduino.h>

void gps_init();
void gps_update();
void gps_set_raw(bool on);

bool gps_time_ok();
bool gps_fix_ok();
bool gps_get_local(struct tm& t);
String gps_hhmm();

double gps_lat();
double gps_lon();

unsigned long gps_chars();
unsigned long gps_checksum_errors();
int gps_satellites();