#include "SoftwareSerial.h"

SoftwareSerial* a = new SoftwareSerial(13, 5);

void setup(void) {
	Serial.begin(9600);
	a->begin(9600);
}

//Il simbolo 'N' è considerato simbolo di errore.
void loop(void) {
	a->print("t01Y08.50");
	a->print("t02Y07.50");
	a->print("t03Y47.50");
	a->print("t04Y48.50");
	a->print("t05Y47.50");
	a->print("t06Y47.50");
	a->print("t07Y48.50");
	a->print("t08Y47.50");
	a->print("t09Y47.50");
	a->print("t10Y48.50");
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
	a->print("t11N47.50*");
	
	delay(400);
}
