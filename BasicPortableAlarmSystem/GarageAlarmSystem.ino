/* Know Your SRAM

This little sketch prints on the serial monitor
the boundaries of .data and .bss memory areas,
of the Heap and the Stack and the amount of
free memory.

Main code from Leonardo Miliani:
www.leonardomiliani.com

freeRam() agorithm from JeeLabs:
http://jeelabs.org/2011/05/22/atmega-memory-use/

This code is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Reviewed on 26/03/2013

*/

//for some MCUs (i.e. the ATmega2560) there's no definition for RAMSTART
#ifndef RAMSTART
extern int __data_start;
#endif

extern int __data_end;
//extern int __bss_start;
//extern int __bss_end;
extern int __heap_start;
extern int __brkval;
int temp;


#if (defined(__AVR__))
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif
//#include <MemoryFree.h>
//#include <pgmStrToRAM.h>
//#include <ChipTemp.h>
#include "MyBlueTooth.h"
#include "BlueToothCommandsUtil.h"
#include "LSGEEpromRW.h" 
#include <EEPROM.h> 
#include <StringFunctions.h>
#include "MySim900.h"
#include "ActivityManager.h"
#include <TimeLib.h>


char version[15] = "-G01 1.00-alfa";

ActivityManager* _delayForTemperature = new ActivityManager(60);

ActivityManager* _delayForVoltage = new ActivityManager(60);

ActivityManager* _delayForFindPhone = new ActivityManager(30);

//ActivityManager* _delayForSignalStrength = new ActivityManager(30);

//ActivityManager* _delayForGetDataFromExternalDevice = new ActivityManager(30);

MyBlueTooth* btSerial;

MySim900* mySim900;

String _oldPassword = "";

String _newPassword = "";
//
//const byte _addressStartBufPhoneNumber = 1;
//
//const byte _addressStartBufPrecisionNumber = 12;

const byte _addressStartBufTemperatureIsOn = 14;

const byte _addressStartBufTemperatureMax = 16;

const byte _addressStartBufPirSensorIsON = 19;

const byte _addressStartDeviceAddress = 21;

const byte _addressStartDeviceName = 33;

const byte _addressStartFindOutPhonesON = 48;

const byte _addressStartBTSleepIsON = 50;

//const byte _addressDBPhoneIsON = 52;

//const byte _addressStartBufPhoneNumberAlternative = 54;

const byte _addressStartFindMode = 65;
//
//const byte _addressApn = 67;

const byte _addressOffSetTemperature = 92;

const byte _addressDelayFindMe = 94;

const byte _addressExternalInterruptIsOn = 96;

const byte _addressStartDeviceAddress2 = 98;

const byte _addressStartDeviceName2 = 110;


uint8_t _isPIRSensorActivated = 0;

bool _isBlueLedDisable = false;

//bool _isDisableCall = false;

bool _isOnMotionDetect = false;

//char _prefix[4] = "+39";

bool _isAlarmOn = false;

//String _phoneNumber;

//char _phoneNumber[11];

//String _phoneNumberAlternative;

//char _phoneNumberAlternative[11];

String _whatIsHappened = "";

//uint8_t _isTemperatureCheckOn = 0;

uint8_t _isBTSleepON = 1;

uint8_t _isExternalInterruptOn = 0;

//uint8_t _phoneNumbers = 0;

uint8_t _findOutPhonesMode = 0;

uint8_t _tempMax = 0;

uint8_t _delayFindMe = 1;

unsigned int _offSetTempValue = 324.13;

//String _signalStrength;

String _deviceAddress = "";

String _deviceAddress2 = "";

String _deviceName = "";

String _deviceName2 = "";

float _voltageValue = 0;

float _voltageMinValue = 0;

unsigned long _millsStart = 0;

bool _isMasterMode = false;

unsigned long _pirSensorTime = 0;

unsigned long _timeToTurnOfBTAfterPowerOn = 300000;

unsigned long _timeAfterPowerOnForBTFinder = 300000;

//String _apn = "";

bool _isPhoneDeviceDetected = false;

//const int BUFSIZEPHONENUMBER = 11;
////char _bufPhoneNumber[BUFSIZEPHONENUMBER];
//
//const int BUFSIZEPHONENUMBERALTERANATIVE = 11;
////char _bufPhoneNumberAlternative[BUFSIZEPHONENUMBERALTERANATIVE];

//const int BUFSIZEPRECISION = 2;
//char _bufPrecisionNumber[BUFSIZEPRECISION];
//
const int BUFSIZETEMPERATUREISON = 2;
char _bufTemperatureIsOn[BUFSIZETEMPERATUREISON];

const int BUFSIZEPIRSENSORISON = 2;
char _bufPirSensorIsON[BUFSIZEPIRSENSORISON];

const int BUFSIZEFINDOUTPHONESON = 2;
char _bufFindOutPhonesON[BUFSIZEFINDOUTPHONESON];

//const int BUFSIZEDBPHONEON = 2;
//char _bufDbPhoneON[BUFSIZEDBPHONEON];

const int BUFSIZETEMPERATUREMAX = 3;
char _bufTemperatureMax[BUFSIZETEMPERATUREMAX];

const int BUFSIZEDEVICEADDRESS = 13;
char _bufDeviceAddress[BUFSIZEDEVICEADDRESS];
char _bufDeviceAddress2[BUFSIZEDEVICEADDRESS];


const int BUFSIZEDEVICENAME = 15;
char _bufDeviceName[BUFSIZEDEVICENAME];
char _bufDeviceName2[BUFSIZEDEVICENAME];

//const int BUFSIZEAPN = 25;
//char _bufApn[BUFSIZEAPN];

const int BUFSIZEOFFSETTEMPERATURE = 5;
char _bufOffSetTemperature[BUFSIZEOFFSETTEMPERATURE];

const int BUFSIZEDELAYFINDME = 2;
char _bufDelayFindMe[BUFSIZEDELAYFINDME];

const int BUFSIZEEXTERNALINTERRUPTISON = 2;
char _bufExternalInterruptIsON[BUFSIZEEXTERNALINTERRUPTISON];

//---------------------------------------------       PINS USED   ----------------------------------------------------------

//Caution: check on portableAlarm board for see what ports you can use,some of them are shorted out.

//static const uint8_t pirSensor1Pin = A4;

static const uint8_t buzzerPin = 9;

static const uint8_t voltagePin = A1;

static const uint8_t interruptExternalMotionPin = 3;

static const uint8_t relayPin = 5;

static const uint8_t pirSensor2Pin = A5;

static const uint8_t softwareSerialExternalDevicesTxPort = 12;

static const uint8_t softwareSerialExternalDevicesRxPort = 11;

static const uint8_t softwareSerialExternalDevicesPinAlarm = A2;

static const uint8_t bluetoothKeyPin = 10;

static const uint8_t bluetoothTransistorPin = 6;

static const byte _pin_powerLed = 13;

static const byte _pin_rxSIM900 = 7;

static const byte _pin_txSIM900 = 8;

SoftwareSerial *softwareSerial = new SoftwareSerial(softwareSerialExternalDevicesRxPort, softwareSerialExternalDevicesTxPort);

bool _isTimeInitialize = false;

void setup()
{
	inizializePins();

	inizializeInterrupts();

	btSerial = new MyBlueTooth(&Serial, bluetoothKeyPin, bluetoothTransistorPin, 38400, 9600);

	btSerial->Reset_To_Slave_Mode();

	_oldPassword = btSerial->GetPassword();

	btSerial->ReceveMode();

	initilizeEEPromData();

	if (_findOutPhonesMode != 0)
	{
		_isBTSleepON = 0;
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Find activated"), BlueToothCommandsUtil::Message));
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	}

	//_sensitivityAlarm = 2000 + ((_precision) * 500);

	btSerial->turnOnBlueTooth();

	_whatIsHappened = F("X");

	//Stare attenti perche' richiede un ritardo che ora è dato da creazione oggetti bluetooth.
	//mySim900->WaitSMSComing();

	pinMode(pirSensor2Pin, INPUT_PULLUP);
	//pinMode(pirSensor1Pin, INPUT_PULLUP);
	pinMode(softwareSerialExternalDevicesTxPort, OUTPUT);

	softwareSerial->begin(19200);

	blinkLed();
}

String getSerialMessage()
{
	String recevedMessage = "";
	if (softwareSerial->available() > 0)
	{
		recevedMessage = softwareSerial->readString();
		recevedMessage.trim();
	}
	return recevedMessage;
}

void initilizeEEPromData()
{
	//EEPROM.write(0, 1);
	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();



	eepromRW->eeprom_read_string(_addressStartFindOutPhonesON, _bufFindOutPhonesON, BUFSIZEFINDOUTPHONESON);
	_findOutPhonesMode = atoi(&_bufFindOutPhonesON[0]);

	eepromRW->eeprom_read_string(_addressStartBufPirSensorIsON, _bufPirSensorIsON, BUFSIZEPIRSENSORISON);
	_isPIRSensorActivated = atoi(&_bufPirSensorIsON[0]);

	/*eepromRW->eeprom_read_string(_addressStartBufPrecisionNumber, _bufPrecisionNumber, BUFSIZEPRECISION);
	_precision = atoi(&_bufPrecisionNumber[0]);*/

	eepromRW->eeprom_read_string(_addressStartBufTemperatureMax, _bufTemperatureMax, BUFSIZETEMPERATUREMAX);
	_tempMax = atoi(_bufTemperatureMax);


	eepromRW->eeprom_read_string(_addressStartDeviceAddress, _bufDeviceAddress, BUFSIZEDEVICEADDRESS);
	_deviceAddress = String(_bufDeviceAddress);

	eepromRW->eeprom_read_string(_addressStartDeviceName, _bufDeviceName, BUFSIZEDEVICENAME);
	_deviceName = String(_bufDeviceName);

	eepromRW->eeprom_read_string(_addressStartDeviceAddress2, _bufDeviceAddress2, BUFSIZEDEVICEADDRESS);
	_deviceAddress2 = String(_bufDeviceAddress2);

	eepromRW->eeprom_read_string(_addressStartDeviceName2, _bufDeviceName2, BUFSIZEDEVICENAME);
	_deviceName2 = String(_bufDeviceName2);


	//eepromRW->eeprom_read_string(_addressApn, _bufApn, BUFSIZEAPN);
	//_apn = String(_bufApn);
	//_apn.trim();

	eepromRW->eeprom_read_string(_addressOffSetTemperature, _bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
	_offSetTempValue = atoi(_bufOffSetTemperature);

	eepromRW->eeprom_read_string(_addressDelayFindMe, _bufDelayFindMe, BUFSIZEDELAYFINDME);
	_delayFindMe = atoi(_bufDelayFindMe);

	eepromRW->eeprom_read_string(_addressExternalInterruptIsOn, _bufExternalInterruptIsON, BUFSIZEEXTERNALINTERRUPTISON);
	_isExternalInterruptOn = atoi(&_bufExternalInterruptIsON[0]);

	delete(eepromRW);

}

void inizializePins()
{
	pinMode(_pin_powerLed, OUTPUT);
	pinMode(buzzerPin, OUTPUT);
	pinMode(softwareSerialExternalDevicesPinAlarm, OUTPUT);
}

void inizializeInterrupts()
{
	/*attachInterrupt(0, motionTiltInternalInterrupt, RISING);*/
	attachInterrupt(1, motionTiltExternalInterrupt, RISING);
}

void motionTiltExternalInterrupt() {
	if (_isExternalInterruptOn /*&& !_isPIRSensorActivated*/) {
		_isOnMotionDetect = true;
	}
}


//void turnOffBluetoohIfTimeIsOver()
//{
//	if (_findOutPhonesMode == 0
//		&& (millis() > _timeToTurnOfBTAfterPowerOn)
//		&& btSerial->isBlueToothOn()
//		&& _isBTSleepON
//		)
//	{
//		btSerial->turnOffBlueTooth();
//		digitalWrite(13, HIGH);
//		delay(5000);
//		digitalWrite(13, LOW);
//	}
//}

//void turnOnBlueToothIfMotionIsDetected()
//{
//	if (_isOnMotionDetect
//		&& !_isAlarmOn
//		&& btSerial->isBlueToothOff()
//		&& _isBTSleepON
//		)
//	{
//		_isOnMotionDetect = false;
//		turnOnBlueToothAndSetTurnOffTimer(false);
//	}
//}

bool isFindOutPhonesONAndSetBluetoothInMasterMode()
{
	//if (_isDisableCall) { return; }

	if ((_findOutPhonesMode == 1 || _findOutPhonesMode == 2) && (millis() > _timeAfterPowerOnForBTFinder))
	{
		if (_findOutPhonesMode == 1 && !_isAlarmOn)
		{
			_isAlarmOn = true;
		}

		if (_isMasterMode == false)
		{
			btSerial->Reset_To_Master_Mode();
			_isMasterMode = true;
		}

		/*	PIRSensor* pirSensor = new PIRSensor(0, A5, 0, 0, "PirSensor01");
		if (humanDetectedWithFindOutPhonesON = 0 && _isPIRSensorActivated && pirSensor->isHumanDetected())
		{
		humanDetectedWithFindOutPhonesON = millis();
		}
		else if (!pirSensor->isHumanDetected())
		{
		humanDetectedWithFindOutPhonesON = 0;
		}*/
		/*if (IsDeviceDetected(String(_bufDeviceAddress), String(_bufDeviceName)))*/

	/*	uint8_t i = 0;
		while (!isDeviceDetected || i < 5)
		{
			isDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress, _deviceName);
			i++;
		}
		i = 0;*/

		for (uint8_t i = 0; i < _delayFindMe; i++)
		{
			_isPhoneDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress, _deviceName);
			if (_isPhoneDeviceDetected) { break; }
			if (_findOutPhonesMode == 1)
			{
				_deviceAddress2.trim();
				_deviceName2.trim();
				if (_deviceAddress2.length() > 1 && _deviceName2.length() > 1) {
					_isPhoneDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress2, _deviceName2);
					if (_isPhoneDeviceDetected) { break; };
				}
			}
		}

		//bool isHumanDetected = pirSensor->isHumanDetected();
		if (_isPhoneDeviceDetected
			//(isDeviceDetected
			//	/*&& (millis() - humanDetectedWithFindOutPhonesON >= 7000)
			//	&& humanDetectedWithFindOutPhonesON != 0*/
			//	&&
			//	isHumanDetected
			//	&& _isPIRSensorActivated)
			//||
			//(
			//	!_isPIRSensorActivated
			//	&&

			)
			//)
		{
			blinkLed();
			//reedRelaySensorActivity(A2);
		}
		else
		{
			if (_findOutPhonesMode == 2)
			{
				//callSim900();
				//delay(10000);
				//_isMasterMode = false;
			}
		}

		/*	delete(pirSensor);*/
		return _isPhoneDeviceDetected;
	}
}

void readMemoryAtRunTime()
{
	//this is necessary to wait for the Arduino Leonardo to get the serial interface up and running
#if defined(__ATmega32U4__)
	while (!Serial);
#else
	delay(2000);
#endif

#ifndef RAMSTART
	serialPrint("SRAM and .data space start: ", (int)&__data_start);
#else
	serialPrint("SRAM and .data space start: ", RAMSTART);
#endif
	serialPrint(".data space end/.bss start: ", (int)&__data_end); //same as "(int)&__bss_start)"
	serialPrint(".bss space end/HEAP start: ", (int)&__heap_start); //same as "(int)&__bss_end);"
	serialPrint("HEAP end: ", (int)__brkval);
	int tempRam = freeRam();
	serialPrint("STACK start: ", temp);
	serialPrint("STACK and SRAM end: ", RAMEND);
	serialPrint("Free memory at the moment: ", tempRam);
	Serial.println("----------------------------------------------------------------------------------");
}

char problematicDevice[4];

char problematicDeviceValue[5];

byte _doorState = 0;

void loop()
{
	String receivedMessage = "";
	while (!_isTimeInitialize)
	{
		digitalWrite(softwareSerialExternalDevicesPinAlarm, LOW);
		receivedMessage = getSerialMessage();
		if (receivedMessage.startsWith("H"))
		{
			String hour = receivedMessage.substring(1, 3);
			String minute = (receivedMessage.substring(3, 5));
			setTime(hour.toInt(), minute.toInt(), 1, 1, 1, 2019);
			//Serial.print(hour); Serial.print(":"); Serial.println(minute);
			_isTimeInitialize = true;
		}
	}

	digitalWrite(softwareSerialExternalDevicesPinAlarm, HIGH);

	/*digitalWrite(softwareSerialExternalDevicesPinAlarm, HIGH);

	if (receivedMessage.startsWith("H"))
	{
		softwareSerial->print("t01N08.50");
		softwareSerial->print("t02Y07.50");
		softwareSerial->print("t03Y47.50");
		softwareSerial->print("t04Y48.50");
		softwareSerial->print("t05Y47.50");
		softwareSerial->print("t06Y47.50");
		softwareSerial->print("t07Y48.50");
		softwareSerial->print("t08Y47.50");
		softwareSerial->print("t09Y47.50");
		softwareSerial->print("t10Y48.50");
		softwareSerial->print("t11Y47.50*");
	}*/


	if ((!(_isOnMotionDetect && _isAlarmOn)) || _findOutPhonesMode != 0)
	{
		if (_delayForFindPhone->IsDelayTimeFinished(true))
		{
			//Serial.println("Sto cercando");
			isFindOutPhonesONAndSetBluetoothInMasterMode();
		}
	}
	//if (!(_isOnMotionDetect && _isAlarmOn))
	//{
	//	turnOffBluetoohIfTimeIsOver();
	//}
	//if (!(_isOnMotionDetect && _isAlarmOn))
	//{
	//	turnOnBlueToothIfMotionIsDetected();
	//}
	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		internalTemperatureActivity();
	}
	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		voltageActivity();
	}

	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		pirSensorActivity();
	}
	isExternalInterruptMotionDetect();
	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		blueToothConfigurationSystem();
	}
	//readMemoryAtRunTime();
}

void isExternalInterruptMotionDetect()
{
	/*if (_findOutPhonesMode == 2 || _isPIRSensorActivated) {*/
	if (_findOutPhonesMode == 2) {
		_isOnMotionDetect = false;
		return;
	}

	if ((_isOnMotionDetect && _isAlarmOn) || (_isAlarmOn && _isExternalInterruptOn && !digitalRead(interruptExternalMotionPin))) //&& !isOnConfiguration)									 /*if(true)*/
	{
		detachInterrupt(0);
		detachInterrupt(1);
		_whatIsHappened = F("M");
		String message = F("M01N");
		if (_findOutPhonesMode == 1)
		{
			if (!_isPhoneDeviceDetected)
			{
				blinkLed();
				sendMessageToComunicatorDevice(message);
			}
		}
		else
		{
			blinkLed();
			sendMessageToComunicatorDevice(message);

		}
		
		EIFR |= 1 << INTF1; //clear external interrupt 1
		EIFR |= 1 << INTF0; //clear external interrupt 0
		sei();
		attachInterrupt(1, motionTiltExternalInterrupt, RISING);

		_isOnMotionDetect = false;
	}
}

void turnOnBlueToothAndSetTurnOffTimer(bool isFromSMS)
{
	//TODO: Fare metodo su mybluetooth per riattivarlo.
	//Essenziale tutta la trafila di istruzioni altrimenti non si riattiva bluetooth
	Serial.flush();
	btSerial->Reset_To_Slave_Mode();
	//btSerial->ReceveMode();
	//btSerial->turnOnBlueTooth();
	if (_findOutPhonesMode == 0 || isFromSMS)
	{
		btSerial->ReceveMode();
		btSerial->turnOnBlueTooth();
		_timeToTurnOfBTAfterPowerOn = millis() + 300000;
		_timeAfterPowerOnForBTFinder = millis() + 120000;
	}
	_isMasterMode = false;
}

void blinkLed()
{
	if (_isBlueLedDisable) { return; }
	pinMode(_pin_powerLed, OUTPUT);
	for (uint8_t i = 0; i < 3; i++)
	{
		digitalWrite(_pin_powerLed, HIGH);
		delay(50);
		digitalWrite(_pin_powerLed, LOW);
		delay(50);
	}
}

String splitStringIndex(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String calculateBatteryLevel(float batteryLevel)

{
	if (batteryLevel <= 3.25)
		return F("[    ]+");
	if (batteryLevel <= 3.30)
		return F("[|   ]+");
	if (batteryLevel <= 3.40)
		return F("[||  ]+");
	if (batteryLevel <= 3.60)
		return F("[||| ]+");
	if (batteryLevel <= 5.50)
		return F("[||||]+");

}

void loadMainMenu()
{
	char* alarmStatus = new char[15];

	if (_isAlarmOn)
	{
		String(F("Alarm ON")).toCharArray(alarmStatus, 15);
	}
	else
	{
		String(F("Alarm OFF")).toCharArray(alarmStatus, 15);
	}

	char result[30];   // array to hold the result.

	strcpy(result, alarmStatus); // copy string one into the result.
	strcat(result, version); // append string two to the result.

	//ChipTemp* chipTemp = new ChipTemp();
	int internalTemperature = getTemp();//chipTemp->celsius();
	/*delete (chipTemp);*/
	delete(alarmStatus);



	String battery = calculateBatteryLevel(_voltageValue);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(result, BlueToothCommandsUtil::Title));

	//char* commandString = new char[15];

	//String(F("Configuration")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Configuration"), BlueToothCommandsUtil::Menu, F("001")));


	//String(F("Security")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Menu, F("004")));

	if (!_isAlarmOn)
	{
		//String(F("Alarm On")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Alarm On"), BlueToothCommandsUtil::Command, F("002")));
	}
	else
	{
		//String(F("Alarm OFF")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Alarm OFF"), BlueToothCommandsUtil::Command, F("003")));
	}

	//String(F("Temp.:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Temp.:" + String(internalTemperature), BlueToothCommandsUtil::Info));

	/*btSerial->println(BlueToothCommandsUtil::CommandConstructor("Batt.value:" + String(_voltageValue), BlueToothCommandsUtil::Info));*/

	//String(F("Batt.level:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Batt.level:" + battery, BlueToothCommandsUtil::Info));

	//String(F("WhatzUp:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("WhatzUp:" + _whatIsHappened, BlueToothCommandsUtil::Info));

	String hours = String(hour());

	String minutes = String(minute());

	if (hour() < 10)
	{
		hours = "0" + String(hour());
	}

	if (minute() < 10)
	{
		minutes = "0" + String(minute());
	}

	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Time:" + hours + ":" + String(minutes), BlueToothCommandsUtil::Info));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	btSerial->Flush();
	//delete(commandString);
}

void loadConfigurationMenu()
{
	//char* commandString = new char[15];
	//String(F("Configuration")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Configuration"), BlueToothCommandsUtil::Title));
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("TempMax:" + String(_tempMax), BlueToothCommandsUtil::Data, F("004")));
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("OffSetTemp:" + String(_offSetTempValue), BlueToothCommandsUtil::Data, F("095")));

	if (_findOutPhonesMode != 2)
	{
		//String(F("PIR status:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("PIR status:" + String(_isPIRSensorActivated), BlueToothCommandsUtil::Data, F("005")));
	}

	if (_findOutPhonesMode != 0)
	{
		//String(F("Addr:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Addr:" + _deviceAddress, BlueToothCommandsUtil::Data, F("010")));

		//String(F("Name:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Name:" + _deviceName, BlueToothCommandsUtil::Data, F("011")));

		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Addr2:" + _deviceAddress2, BlueToothCommandsUtil::Data, F("015")));

		//String(F("Name:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("Name2:" + _deviceName2, BlueToothCommandsUtil::Data, F("016")));

		//String(F("FindTime.:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor("FindLoop:" + String(_delayFindMe), BlueToothCommandsUtil::Data, F("094")));
	}

	//String(F("Find phone:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("FindMode:" + String(_findOutPhonesMode), BlueToothCommandsUtil::Data, F("012")));

	//String(F("Ext.Int:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor("Ext.Int:" + String(_isExternalInterruptOn), BlueToothCommandsUtil::Data, F("013")));

	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	//delete(commandString);
}

void blueToothConfigurationSystem()
{
	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();
	String _bluetoothData = "";
	if (btSerial->available())
	{
		_bluetoothData = btSerial->readString();
		//BluetoothData.trim();

		//ROOT: Main
#pragma region Main Menu-#0
		if (_bluetoothData.indexOf(F("#0")) > -1)
		{
			loadMainMenu();
		}


#pragma region Commands

		if (_bluetoothData.indexOf(F("C002")) > -1)
		{
			//do something
			//digitalWrite(_pin_powerLed, HIGH);
			_isAlarmOn = true;
			_isOnMotionDetect = false;
			_timeAfterPowerOnForBTFinder = 0;
			//btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Portable Alarm ON"), BlueToothCommandsUtil::Title));
		/*	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Alarm ON"), BlueToothCommandsUtil::Message));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
			loadMainMenu();
		}

		if (_bluetoothData.indexOf(F("C003")) > -1)
		{
			//do something
			_isAlarmOn = false;

			//digitalWrite(_pin_powerLed, LOW);
			//btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Portable Alarm OFF"), BlueToothCommandsUtil::Title));
		/*	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Alarm OFF"), BlueToothCommandsUtil::Message));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
			loadMainMenu();
		}
#pragma endregion

#pragma region Data


#pragma endregion


#pragma endregion

		//ROOT Main/Configuration
#pragma region Configuration Menu-#M001
		if (_bluetoothData.indexOf(F("M001")) > -1)
		{
			loadConfigurationMenu();
		}
#pragma region Commands

#pragma endregion


#pragma region Data


		//if (_bluetoothData.indexOf(F("D097")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZEFINDMODE = 2;
		//		char _bufFindMode[BUFSIZEFINDMODE];*/

		//		splitString.toCharArray(_bufFindMode, BUFSIZEFINDMODE);
		//		eepromRW->eeprom_write_string(_addressStartFindMode, _bufFindMode);
		//		_findMode = atoi(&_bufFindMode[0]);
		//	}
		//	updateCommand();
		//	/*btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("FindMode updated"), BlueToothCommandsUtil::Message));
		//	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
		//}

		if (_bluetoothData.indexOf(F("D094")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEDBPHONEON = 2;
				char _bufDbPhoneON[BUFSIZEDBPHONEON];*/

				splitString.toCharArray(_bufDelayFindMe, BUFSIZEDEVICEADDRESS);
				eepromRW->eeprom_write_string(_addressDelayFindMe, _bufDelayFindMe);
				_delayFindMe = atoi(&_bufDelayFindMe[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D095")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
				eepromRW->eeprom_write_string(_addressOffSetTemperature, _bufOffSetTemperature);
				_offSetTempValue = atoi(&_bufOffSetTemperature[0]);
			}

			loadConfigurationMenu();
		}

		//if (_bluetoothData.indexOf(F("D096")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	/*if (isValidNumber(splitString))
		//	{*/
		//	/*const int BUFSIZEPHONENUMBERALTERNATIVE = 11;
		//	char _bufPhoneNumberAlternative[BUFSIZEPHONENUMBERALTERNATIVE];*/

		//	splitString.toCharArray(_bufApn, BUFSIZEAPN);
		//	eepromRW->eeprom_write_string(_addressApn, _bufApn);
		//	_apn = splitString;
		//	//}
		//	loadConfigurationMenu();
		//}


		//if (_bluetoothData.indexOf(F("D002")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*char _bufPrecisionNumber[2];*/
		//		splitString.toCharArray(_bufPrecisionNumber, 2);
		//		eepromRW->eeprom_write_string(12, _bufPrecisionNumber);
		//		_precision = atoi(&_bufPrecisionNumber[0]);
		//		_sensitivityAlarm = 2000 + ((_precision) * 500);
		//	}
		//	loadConfigurationMenu();
		//}

		//if (_bluetoothData.indexOf(F("D003")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZETEMPERATUREISON = 2;
		//		char _bufTemperatureIsOn[BUFSIZETEMPERATUREISON];*/

		//		splitString.toCharArray(_bufTemperatureIsOn, BUFSIZETEMPERATUREISON);
		//		eepromRW->eeprom_write_string(14, _bufTemperatureIsOn);
		//		_isTemperatureCheckOn = atoi(&_bufTemperatureIsOn[0]);
		//	}
		//	updateCommand();
		//}

		//if (_bluetoothData.indexOf(F("D033")) > -1)
		//{
		//	String splitString = splitStringIndex(_bluetoothData, ';', 1);
		//	if (isValidNumber(splitString))
		//	{
		//		/*const int BUFSIZEBTSLEEPISON = 2;
		//		char _bufBTSleepIsON[BUFSIZEBTSLEEPISON];*/

		//		splitString.toCharArray(_bufBTSleepIsON, BUFSIZEBTSLEEPISON);
		//		eepromRW->eeprom_write_string(_addressStartBTSleepIsON, _bufBTSleepIsON);
		//		_isBTSleepON = atoi(&_bufBTSleepIsON[0]);
		//	}
		//	updateCommand();
		//	/*btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("BTSleep updated"), BlueToothCommandsUtil::Message));
		//	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
		//}

		if (_bluetoothData.indexOf(F("D004")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZETEMPERATUREVALUE = 3;
				char _bufTemperatureValue[BUFSIZETEMPERATUREVALUE];*/

				splitString.toCharArray(_bufTemperatureMax, BUFSIZETEMPERATUREMAX);
				eepromRW->eeprom_write_string(16, _bufTemperatureMax);
				_tempMax = atoi(_bufTemperatureMax);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D005")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*	const int BUFSIZEPIRSENSORISON = 2;
					char _bufPirSensorIsON[BUFSIZEPIRSENSORISON];*/
				splitString.toCharArray(_bufPirSensorIsON, BUFSIZEPIRSENSORISON);
				eepromRW->eeprom_write_string(19, _bufPirSensorIsON);
				_isPIRSensorActivated = atoi(&_bufPirSensorIsON[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D010")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEDEVICEADDRESS = 13;
				char _bufDeviceAddress[BUFSIZEDEVICEADDRESS];*/

				splitString.toCharArray(_bufDeviceAddress, BUFSIZEDEVICEADDRESS);
				eepromRW->eeprom_write_string(_addressStartDeviceAddress, _bufDeviceAddress);
				_deviceAddress = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D011")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEDEVICENAME = 15;
				char _bufDeviceName[BUFSIZEDEVICENAME];*/

				splitString.toCharArray(_bufDeviceName, BUFSIZEDEVICENAME);
				eepromRW->eeprom_write_string(_addressStartDeviceName, _bufDeviceName);
				_deviceName = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D015")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDeviceAddress2, BUFSIZEDEVICEADDRESS);
				eepromRW->eeprom_write_string(_addressStartDeviceAddress2, _bufDeviceAddress2);
				_deviceAddress2 = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D016")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufDeviceName2, BUFSIZEDEVICENAME);
				eepromRW->eeprom_write_string(_addressStartDeviceName2, _bufDeviceName2);
				_deviceName2 = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D012")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEFINDOUTPHONESON = 2;
				char _bufFindOutPhonesON[BUFSIZEFINDOUTPHONESON];*/

				splitString.toCharArray(_bufFindOutPhonesON, BUFSIZEFINDOUTPHONESON);
				eepromRW->eeprom_write_string(_addressStartFindOutPhonesON, _bufFindOutPhonesON);
				_findOutPhonesMode = atoi(&_bufFindOutPhonesON[0]);
				if (_findOutPhonesMode != 0)
				{
					_isBTSleepON = 0;
					if (_findOutPhonesMode == 2)
					{
						_isPIRSensorActivated = 0;
					}
				}
				else
				{
					/*const int BUFSIZEBTSLEEPISON = 2;
					char _bufBTSleepIsON[BUFSIZEBTSLEEPISON];*/
					/*eepromRW->eeprom_read_string(_addressStartBTSleepIsON, _bufBTSleepIsON, BUFSIZEBTSLEEPISON);
					_isBTSleepON = atoi(&_bufBTSleepIsON[0]);*/

					_isBTSleepON = 1;
				}

			}

			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D013")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				splitString.toCharArray(_bufExternalInterruptIsON, BUFSIZEEXTERNALINTERRUPTISON);
				eepromRW->eeprom_write_string(_addressExternalInterruptIsOn, _bufExternalInterruptIsON);
				_isExternalInterruptOn = atoi(&_bufExternalInterruptIsON[0]);
			}
			loadConfigurationMenu();
		}

#pragma endregion

#pragma Configuration Menu endregion


#pragma region Security-M004
		if (_bluetoothData.indexOf(F("M004")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw.:"), BlueToothCommandsUtil::Menu, F("005")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change name:"), BlueToothCommandsUtil::Menu, F("006")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}
#pragma region Menu
		if (_bluetoothData.indexOf(F("M005")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert old passw.:"), BlueToothCommandsUtil::Data, F("006")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("M006")) > -1)
		{
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert name:"), BlueToothCommandsUtil::Data, F("007")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}
#pragma endregion


#pragma region Commands

#pragma endregion

#pragma region Data
		if (_bluetoothData.indexOf(F("D006")) > -1)
		{
			String confirmedOldPassword = splitStringIndex(_bluetoothData, ';', 1);

			if (_oldPassword == confirmedOldPassword)
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Insert new passw:"), BlueToothCommandsUtil::Data, F("008")));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}
			else
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Wrong passw:"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}

		}

		if (_bluetoothData.indexOf(F("D008")) > -1)
		{
			_newPassword = splitStringIndex(_bluetoothData, ';', 1);
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Confirm pass:"), BlueToothCommandsUtil::Data, F("009")));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("D009")) > -1)
		{
			if (_newPassword == splitStringIndex(_bluetoothData, ';', 1))
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				delay(2000);
				btSerial->SetPassword(_newPassword);
				_oldPassword = _newPassword;
			}

			else
			{
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("passw. doesn't match"), BlueToothCommandsUtil::Message));
				btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				btSerial->println("D006");
			}
		}


		if (_bluetoothData.indexOf(F("D007")) > -1)
		{
			String btName = splitStringIndex(_bluetoothData, ';', 1);

			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
			btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			delay(2000);
			btSerial->SetBlueToothName(btName);
		}


#pragma endregion

#pragma endregion

		delay(100);
	}
	delete(eepromRW);
}

boolean isValidNumber(String str)
{
	for (byte i = 0; i < str.length(); i++)
	{
		if (isDigit(str.charAt(i))) return true;
	}
	return false;
}
unsigned long deltaTimeForOpenTheDoor = 0;
void pirSensorActivity()
{
	if (isGarageDoorClosed() && _doorState == 1)
	{
		if (deltaTimeForOpenTheDoor == 0)
		{
			deltaTimeForOpenTheDoor = millis();
			Serial.println("Attesa per riapertura garage");
		}
		if (millis() - deltaTimeForOpenTheDoor > 60000)
		{
			Serial.println("Garage pronto per riapertura");
			_doorState = 0;
			deltaTimeForOpenTheDoor = 0;
		}
	}
	if (_isPIRSensorActivated && _isAlarmOn)
	{
		if (isThereSomeOneInFrontOfGarage())
		{
			//Serial.println("isThereSomeOneInFrontOfGarage");
			_whatIsHappened = F("P");

			if (_findOutPhonesMode == 1 && _isPhoneDeviceDetected && isGarageDoorClosed() && _doorState == 0)
			{
				blinkLed();
				buzzerFunction(buzzerPin, 100, 1000);
				Serial.println("Apertura garage");
				_doorState = 1;
				//Aggiungere codice che gestisce interrupt pin aperto.
				reedRelaySensorActivity(relayPin);
				delay(30000);
			}
			else if ((isAM() && hour() < 6) && !_isPhoneDeviceDetected)
			{
				if ((millis() - _pirSensorTime) > 30000)
				{
					_pirSensorTime = millis();
				}
				else if ((millis() - _pirSensorTime) > 25000)
				{
					blinkLed();
					String message = "P01N";
					sendMessageToComunicatorDevice(message);
				}
			}

		}
		/*unsigned int count0 = 0;
		for (unsigned int i = 0; i < 110; i++)
		{
		if (digitalRead(5))
		{
		count0++;
		}
		}

		if (count0 > 100)
		{
		blinkLed();
		_whatIsHappened = F("P");
		reedRelaySensorActivity(A4);
		callSim900('1');
		}*/
	}
}

bool isThereSomeOneInFrontOfGarage()
{
	return digitalRead(pirSensor2Pin);
}

bool isGarageDoorClosed()
{
	return digitalRead(interruptExternalMotionPin);
}

void reedRelaySensorActivity(uint8_t pin)
{
	pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);
	delay(1000);
	digitalWrite(pin, LOW);
}

void internalTemperatureActivity()
{
	if (_delayForTemperature->IsDelayTimeFinished(true))
	{
		/*if (_isTemperatureCheckOn == 0) return;*/

		/*ChipTemp* chipTemp = new ChipTemp();*/

		if ((uint8_t)getTemp() > _tempMax)
		{
			_whatIsHappened = F("T");
			String message = "TINN" + String((uint8_t)getTemp());
			sendMessageToComunicatorDevice(message);
			//callSim900();
		}
		/*delete chipTemp;*/
	}
}

void voltageActivity()
{

	if (_delayForVoltage->IsDelayTimeFinished(true))
	{
		//Serial.println(analogRead(voltagePin));
		_voltageValue = (5.1 / 1023.00) * analogRead(voltagePin);
		//Serial.println(_voltageValue);
		_voltageMinValue = 3.25;
		if (_voltageValue < _voltageMinValue)
		{
			_whatIsHappened = F("V");

			String message = "V01N" + String(_voltageValue);
			sendMessageToComunicatorDevice(message);
		}
	}
}

void activateFunctionAlarm()
{
	_timeToTurnOfBTAfterPowerOn = 0;
	_timeAfterPowerOnForBTFinder = 0;
	//_isDisableCall = false;
	_isAlarmOn = true;
	//callSim900();
}

double getTemp(void)
{
	unsigned int wADC;
	double t;

	// The internal temperature has to be used
	// with the internal reference of 1.1V.
	// Channel 8 can not be selected with
	// the analogRead function yet.

	// Set the internal reference and mux.
	ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
	ADCSRA |= _BV(ADEN);  // enable the ADC

	delay(20);            // wait for voltages to become stable.

	ADCSRA |= _BV(ADSC);  // Start the ADC

						  // Detect end-of-conversion
	while (bit_is_set(ADCSRA, ADSC));

	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
	wADC = ADCW;

	// The offset of 324.31 could be wrong. It is just an indication.
	t = (wADC - _offSetTempValue) / 1.22;

	// The returned temperature is in degrees Celsius.
	return (t);
}

//unsigned int offSetTempValue(double externalTemperature)
//{
//	unsigned int wADC;
//	double t;
//	// The internal temperature has to be used
//	// with the internal reference of 1.1V.
//	// Channel 8 can not be selected with
//	// the analogRead function yet.
//
//	// Set the internal reference and mux.
//	ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
//	ADCSRA |= _BV(ADEN);  // enable the ADC
//
//	delay(20);            // wait for voltages to become stable.
//
//	ADCSRA |= _BV(ADSC);  // Start the ADC
//
//						  // Detect end-of-conversion
//	while (bit_is_set(ADCSRA, ADSC));
//
//	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
//	wADC = ADCW;
//
//	// The offset of 324.31 could be wrong. It is just an indication.
//	//t = (wADC - _offSetTempValue) / 1.22;
//
//	// The returned temperature is in degrees Celsius.
//	return (-(externalTemperature * 1.22) + wADC);
//}

//print function
void serialPrint(String tempString, int tempData) {
	Serial.print(tempString);
	Serial.print(tempData, DEC);
	Serial.print(" $");
	Serial.println(tempData, HEX);
}

//computes the free memory (from JeeLabs)
int freeRam() {
	int v;
	temp = (int)&v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void sendMessageToComunicatorDevice(String message)
{
	digitalWrite(softwareSerialExternalDevicesPinAlarm, LOW);
	bool isMessageReceived = false;
	while (!isMessageReceived)
	{
		if (softwareSerial->available() > 0)
		{
			String messageReceived = softwareSerial->readString();
			if (messageReceived.startsWith("H"))
			{
				//Serial.println("Pronto a trasmettere");
				softwareSerial->print(message); softwareSerial->print("*");
				isMessageReceived = true;
			}
		}
	}
	digitalWrite(softwareSerialExternalDevicesPinAlarm, HIGH);
}

void buzzerFunction(byte buzzerPin,int frequency,int time)
{
	tone(buzzerPin, frequency, time);
}

