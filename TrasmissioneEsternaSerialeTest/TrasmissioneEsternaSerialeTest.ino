#include "SoftwareSerial.h"

SoftwareSerial* a = new SoftwareSerial(13, 5);

void setup(void) {
	Serial.begin(9600);
	a->begin(9600);
}


void loop(void) {
	a->print("t01,08.50");
	a->print("t02,07.50");
	a->print("t03,47.50");
	a->print("t04,48.50");
	a->print("t05,47.50");
	a->print("t06,47.50");
	a->print("t07,48.50");
	a->print("t08,47.50");
	a->print("t09,47.50");
	a->print("t10,48.50");
	/*a->print("t11,48.50,");
	a->print("t12,47.50,");
	a->print("t13,47.50,");
	a->print("t14,48.50,");
	a->print("t15,47.50,");
	a->print("t16,47.50,");
	a->print("t17,48.50,");
	a->print("t18,47.50,");
	a->print("t19,47.50,");
	a->print("t20,48.50,");
	a->print("t21,47.50,");*/
	a->println("t11,47.50*");
	
	delay(400);
}
