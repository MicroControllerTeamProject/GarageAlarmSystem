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

// for arduino IDE


// for some MCUs (i.e. the ATmega2560) there's no definition for RAMSTART
#ifndef RAMSTART
extern int __data_start;
#endif

extern int __data_end;
// extern int __bss_start;
// extern int __bss_end;
extern int __heap_start;
extern int __brkval;
int temp;

#if (defined(__AVR__))
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif
// #include <MemoryFree.h>
// #include <pgmStrToRAM.h>
// #include <ChipTemp.h>

#include <MyBlueTooth.h>
#include <BlueToothCommandsUtil.h>
#include <LSGEEpromRW.h>
#include <EEPROM.h>
#include <StringFunctions.h>
#include <MySim900.h>
#include <ActivityManager.h>
#include <TimeLib.h>

char version[15] = "-G01 1.10-alfa";

ActivityManager* _delayForTemperature = new ActivityManager(2 * 60);

ActivityManager* _delayForVoltage = new ActivityManager(5 * 60);

// ActivityManager* _delayForFindPhone = new ActivityManager(30);

// ActivityManager* _delayForSignalStrength = new ActivityManager(30);

// ActivityManager* _delayForGetDataFromExternalDevice = new ActivityManager(30);

String _oldPassword = "";

String _newPassword = "";

const byte _addressStartBufTemperatureIsOn = 14;

const byte _addressStartBufTemperatureMax = 16;

const byte _addressStartBufPirSensorIsON = 19;

const byte _addressStartDeviceAddress = 21;

const byte _addressStartDeviceName = 33;

const byte _addressStartFindOutPhonesON = 48;

const byte _addressStartBTSleepIsON = 50;

const byte _addressStartFindMode = 65;

const byte _addressOffSetTemperature = 92;

const byte _addressDelayFindMe = 94;

const byte _addressExternalInterruptIsOn = 96;

const byte _addressStartDeviceAddress2 = 98;

const byte _addressStartDeviceName2 = 110;

uint8_t _isPIRSensorActivated = 0;

bool _isBlueLedDisable = false;

bool _isOnMotionDetect = false;

bool _isAlarmOn = true;

String _whatIsHappened = "";

uint8_t _isBTSleepON = 1;

uint8_t _isExternalInterruptOn = 0;

uint8_t _findOutPhonesMode = 0;

uint8_t _tempMax = 0;

uint8_t _delayFindMe = 1;

unsigned int _offSetTempValue = 324.13;

// String _signalStrength;

String _deviceAddress = "";

String _deviceAddress2 = "";

String _deviceName = "";

String _deviceName2 = "";

float _voltageValue = 0;

float _voltageMinValue = 0;

unsigned long _millsStart = 0;

bool _isMasterMode = false;

unsigned long _pirSensorTime = 0;

unsigned long _timeForSetBTConfiguration = 0;

// String _apn = "";

bool _isPhoneDeviceDetected = false;

bool isBuzzerDisable = false;

// const int BUFSIZEPHONENUMBER = 11;
////char _bufPhoneNumber[BUFSIZEPHONENUMBER];
//
// const int BUFSIZEPHONENUMBERALTERANATIVE = 11;
////char _bufPhoneNumberAlternative[BUFSIZEPHONENUMBERALTERANATIVE];

// const int BUFSIZEPRECISION = 2;
// char _bufPrecisionNumber[BUFSIZEPRECISION];
//
const int BUFSIZETEMPERATUREISON = 2;

char _bufTemperatureIsOn[BUFSIZETEMPERATUREISON];

const int BUFSIZEPIRSENSORISON = 2;

char _bufPirSensorIsON[BUFSIZEPIRSENSORISON];

const int BUFSIZEFINDOUTPHONESON = 2;

char _bufFindOutPhonesON[BUFSIZEFINDOUTPHONESON];

const int BUFSIZETEMPERATUREMAX = 3;

char _bufTemperatureMax[BUFSIZETEMPERATUREMAX];

const int BUFSIZEDEVICEADDRESS = 13;

char _bufDeviceAddress[BUFSIZEDEVICEADDRESS];

char _bufDeviceAddress2[BUFSIZEDEVICEADDRESS];

const int BUFSIZEDEVICENAME = 15;

char _bufDeviceName[BUFSIZEDEVICENAME];

char _bufDeviceName2[BUFSIZEDEVICENAME];

const int BUFSIZEOFFSETTEMPERATURE = 5;

char _bufOffSetTemperature[BUFSIZEOFFSETTEMPERATURE];

const int BUFSIZEDELAYFINDME = 2;

char _bufDelayFindMe[BUFSIZEDELAYFINDME];

const int BUFSIZEEXTERNALINTERRUPTISON = 2;

char _bufExternalInterruptIsON[BUFSIZEEXTERNALINTERRUPTISON];

static const uint8_t buzzerPin = 9;

static const uint8_t voltagePin = A1;

static const uint8_t interruptExternalMotionPin = 3;

static const uint8_t relayPin = A4;

static const uint8_t pirSensor2Pin = A5;

static const uint8_t softwareSerialExternalDevicesTxPort = 12;

static const uint8_t softwareSerialExternalDevicesRxPort = 11;

static const uint8_t softwareSerialExternalDevicesPinAlarm = A2;

static const uint8_t bluetoothKeyPin = 10;

static const uint8_t bluetoothTransistorPin = 6;

static const byte _pin_powerLed = 13;

static const byte _pin_rxSIM900 = 7;

static const byte _pin_txSIM900 = 8;

SoftwareSerial softwareSerial(softwareSerialExternalDevicesRxPort, softwareSerialExternalDevicesTxPort);

bool _isTimeInitialize = false;

byte _doorState = 0;

MyBlueTooth btSerial(&Serial, bluetoothKeyPin, bluetoothTransistorPin, 38400, 9600);


//using in arduino ide enviroment
#define _DEBUG 
#define _HARDWARE_CODE

void setup()
{
	softwareSerial.begin(9600);

	inizializePins();

	digitalWrite(softwareSerialExternalDevicesPinAlarm, HIGH);

	inizializeInterrupts();

	btSerial.Reset_To_Slave_Mode();

#ifdef _DEBUG
	_oldPassword = btSerial.GetPasswordV3();
	Serial.print(F("\r\nPSW: "));
	Serial.println(_oldPassword);
#endif // _DEBUG

	btSerial.ReceveMode();

	initilizeEEPromData();

	if (_findOutPhonesMode != 0)
	{
		_isBTSleepON = 0;
		btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Find activated"), BlueToothCommandsUtil::Message));
		btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	}

	btSerial.turnOnBlueTooth();

	_whatIsHappened = F("X");

	pinMode(pirSensor2Pin, INPUT_PULLUP);

	blinkLed();
}

void loop()
{

	while (!_isTimeInitialize)
	{
		String receivedMessage = "";

		digitalWrite(softwareSerialExternalDevicesPinAlarm, LOW);

		receivedMessage = getSerialMessageFromExternalDevice();

		// receivedMessage = "H1700";  using for test

		if (receivedMessage.startsWith("H"))
		{
			String hour = receivedMessage.substring(1, 3);

			String minute = (receivedMessage.substring(3, 5));

			setTime(hour.toInt(), minute.toInt(), 1, 1, 1, 2019);

#ifdef _DEBUG
			Serial.print(F("got dateTime: "));
			Serial.print(hour);
			Serial.print(":");
			Serial.println(minute);
#endif // _DEBUG

			_isTimeInitialize = true;

			digitalWrite(softwareSerialExternalDevicesPinAlarm, HIGH);

			_timeForSetBTConfiguration = millis() + (/*0.1F*/ 3UL * 60UL * 1000UL);

			buzzerFunction(buzzerPin, 1000, 500);

			buzzerFunction(buzzerPin, 1200, 500);

			buzzerFunction(buzzerPin, 1400, 500);

			// Serial.println(F("startBT"));
		}
	}

	if (!(_isOnMotionDetect))
	{
		voltageActivity();
	}

	// disable data require.
	// On start to configure bluetooth parameters      ----- STOP CODE FOR BLUETOOTH CONFIGURATION
	if (millis() < _timeForSetBTConfiguration)
	{
		blueToothConfigurationSystem();
		return;
	}

	////When hour was sent and bluetooth time is finished always turn on alarm
	_isAlarmOn = true;

	if (_findOutPhonesMode == 1 && _isMasterMode == false)
	{
		// Serial.println(F("endBT"));
		// btSerial.Reset_To_Master_Mode();
		btSerial.findModeV3();
		_isMasterMode = true;
		buzzerFunction(buzzerPin, 1000, 500);
		buzzerFunction(buzzerPin, 1200, 500);
		buzzerFunction(buzzerPin, 1400, 500);
	}

	// resetTimeAlarm();

	isMyPhoneDetected();

	isExternalInterruptMotionDetect();

	if (!(_isOnMotionDetect))
	{
		internalTemperatureActivity();
	}

	if (!(_isOnMotionDetect))
	{
		openGarageDoorWithPhone();
	}
}

String getSerialMessageFromExternalDevice()
{
	String recevedMessage = "";
	if (softwareSerial.available() > 0)
	{
		recevedMessage = softwareSerial.readString();
		recevedMessage.trim();
	}
	return recevedMessage;
}

void initilizeEEPromData()
{
	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();

#ifdef _HARDWARE_CODE  
	char hardware_code[4] = {};
	eepromRW->eeprom_read_string(500, hardware_code, 4);
	Serial.print(F("\r\nhardware_code: ")); Serial.println(hardware_code);
#endif 

	eepromRW->eeprom_read_string(_addressStartFindOutPhonesON, _bufFindOutPhonesON, BUFSIZEFINDOUTPHONESON);
	_findOutPhonesMode = atoi(&_bufFindOutPhonesON[0]);

	eepromRW->eeprom_read_string(_addressStartBufPirSensorIsON, _bufPirSensorIsON, BUFSIZEPIRSENSORISON);
	_isPIRSensorActivated = atoi(&_bufPirSensorIsON[0]);

	eepromRW->eeprom_read_string(_addressStartBufTemperatureMax, _bufTemperatureMax, BUFSIZETEMPERATUREMAX);
	_tempMax = atoi(_bufTemperatureMax);

	eepromRW->eeprom_read_string(_addressStartDeviceAddress, _bufDeviceAddress, BUFSIZEDEVICEADDRESS);
	_deviceAddress = "DC" + String(_bufDeviceAddress);

	eepromRW->eeprom_read_string(_addressStartDeviceName, _bufDeviceName, BUFSIZEDEVICENAME);
	_deviceName = String(_bufDeviceName);

	eepromRW->eeprom_read_string(_addressStartDeviceAddress2, _bufDeviceAddress2, BUFSIZEDEVICEADDRESS);
	_deviceAddress2 = String(_bufDeviceAddress2);

	eepromRW->eeprom_read_string(_addressStartDeviceName2, _bufDeviceName2, BUFSIZEDEVICENAME);
	_deviceName2 = String(_bufDeviceName2);

	eepromRW->eeprom_read_string(_addressOffSetTemperature, _bufOffSetTemperature, BUFSIZEOFFSETTEMPERATURE);
	_offSetTempValue = atoi(_bufOffSetTemperature);

	eepromRW->eeprom_read_string(_addressDelayFindMe, _bufDelayFindMe, BUFSIZEDELAYFINDME);
	_delayFindMe = atoi(_bufDelayFindMe);

	eepromRW->eeprom_read_string(_addressExternalInterruptIsOn, _bufExternalInterruptIsON, BUFSIZEEXTERNALINTERRUPTISON);
	_isExternalInterruptOn = atoi(&_bufExternalInterruptIsON[0]);

	delete (eepromRW);
}

void inizializePins()
{
	pinMode(relayPin, OUTPUT);
	digitalWrite(relayPin, LOW);
	pinMode(_pin_powerLed, OUTPUT);
	pinMode(buzzerPin, OUTPUT);
	pinMode(softwareSerialExternalDevicesPinAlarm, OUTPUT);
}

void inizializeInterrupts()
{
	attachInterrupt(1, motionTiltExternalInterrupt, RISING);
}

void motionTiltExternalInterrupt()
{
	if (_isExternalInterruptOn /*&& !_isPIRSensorActivated*/)
	{
		_isOnMotionDetect = true;
	}
}

bool isMyPhoneDetected()
{
	_isPhoneDeviceDetected = false;

	if (millis() > _timeForSetBTConfiguration)
	{
		for (uint8_t i = 0; i < _delayFindMe; i++)
		{
			_isPhoneDeviceDetected = btSerial.IsDeviceDetected(_deviceAddress, _deviceName);
			/*if (_isPhoneDeviceDetected) { break; }*/

			if (_isPhoneDeviceDetected)
			{
				blinkLed();
				return true;
			}
			/*if (_findOutPhonesMode == 1)
			{
				_deviceAddress2.trim();
				_deviceName2.trim();
				if (_deviceAddress2.length() > 1 && _deviceName2.length() > 1) {
					_isPhoneDeviceDetected = btSerial.IsDeviceDetected(_deviceAddress2, _deviceName2);
					if (_isPhoneDeviceDetected) { break; };
				}
			}*/
		}

		/*	if (_isPhoneDeviceDetected)
			{
				blinkLed();
			}
			return _isPhoneDeviceDetected;*/
	}
	return false;
}


void isExternalInterruptMotionDetect()
{
	if ((_isOnMotionDetect && _isAlarmOn) || (_isAlarmOn && _isExternalInterruptOn && !digitalRead(interruptExternalMotionPin))) //&& !isOnConfiguration)									 /*if(true)*/
	{
		detachInterrupt(1);
#ifdef _DEBUG
		Serial.println(F("motion detect"));
#endif
		_whatIsHappened = F("M");

		String message = F("M01N");
		
		if (!_isPhoneDeviceDetected)
		{
			blinkLed();
			sendMessageToComunicatorDevice(message);
		}
		_isOnMotionDetect = false;
		//}
		EIFR |= 1 << INTF1; // clear external interrupt 1
		// EIFR |= 1 << INTF0; //clear external interrupt 0
		sei();
		attachInterrupt(1, motionTiltExternalInterrupt, RISING);

	}
}

// void turnOnBlueToothAndSetTurnOffTimer(bool isFromSMS)
//{
//	//TODO: Fare metodo su mybluetooth per riattivarlo.
//	//Essenziale tutta la trafila di istruzioni altrimenti non si riattiva bluetooth
//	Serial.flush();
//	btSerial.Reset_To_Slave_Mode();
//	//btSerial.ReceveMode();
//	//btSerial.turnOnBlueTooth();
//	if (_findOutPhonesMode == 0 || isFromSMS)
//	{
//		btSerial.ReceveMode();
//		btSerial.turnOnBlueTooth();
//		//_timeToTurnOfBTAfterPowerOn = millis() + 300000;
//		_timeForSetBTConfiguration = millis() + 120000;
//	}
//	_isMasterMode = false;
// }

void blinkLed()
{
	if (_isBlueLedDisable)
	{
		return;
	}
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

	for (int i = 0; i <= maxIndex && found <= index; i++)
	{
		if (data.charAt(i) == separator || i == maxIndex)
		{
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

	char result[30]; // array to hold the result.

	strcpy(result, alarmStatus); // copy string one into the result.
	strcat(result, version);	 // append string two to the result.

	// ChipTemp* chipTemp = new ChipTemp();
	int internalTemperature = getTemp(); // chipTemp->celsius();
	/*delete (chipTemp);*/
	delete (alarmStatus);

	String battery = calculateBatteryLevel(_voltageValue);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor(result, BlueToothCommandsUtil::Title));

	// char* commandString = new char[15];

	// String(F("Configuration")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Configuration"), BlueToothCommandsUtil::Menu, F("001")));

	// String(F("Security")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Menu, F("004")));

	if (!_isAlarmOn)
	{
		// String(F("Alarm On")).toCharArray(commandString, 15);
		btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Alarm On"), BlueToothCommandsUtil::Command, F("002")));
	}
	else
	{
		// String(F("Alarm OFF")).toCharArray(commandString, 15);
		btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Alarm OFF"), BlueToothCommandsUtil::Command, F("003")));
	}

	// String(F("Temp.:")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Temp.:" + String(internalTemperature), BlueToothCommandsUtil::Info));

	/*btSerial.println(BlueToothCommandsUtil::CommandConstructor("Batt.value:" + String(_voltageValue), BlueToothCommandsUtil::Info));*/

	// String(F("Batt.level:")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Batt.level:" + battery, BlueToothCommandsUtil::Info));

	// String(F("WhatzUp:")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("WhatzUp:" + _whatIsHappened, BlueToothCommandsUtil::Info));

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

	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Time:" + hours + ":" + String(minutes), BlueToothCommandsUtil::Info));

	btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	btSerial.Flush();
	// delete(commandString);
}

void loadConfigurationMenu()
{
	// char* commandString = new char[15];
	// String(F("Configuration")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Configuration"), BlueToothCommandsUtil::Title));
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("TempMax:" + String(_tempMax), BlueToothCommandsUtil::Data, F("004")));
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("OffSetTemp:" + String(_offSetTempValue), BlueToothCommandsUtil::Data, F("095")));

	if (_findOutPhonesMode != 2)
	{
		// String(F("PIR status:")).toCharArray(commandString, 15);
		btSerial.println(BlueToothCommandsUtil::CommandConstructor("PIR status:" + String(_isPIRSensorActivated), BlueToothCommandsUtil::Data, F("005")));
	}

	if (_findOutPhonesMode != 0)
	{
		// String(F("Addr:")).toCharArray(commandString, 15);
		btSerial.println(BlueToothCommandsUtil::CommandConstructor("Addr:" + _deviceAddress, BlueToothCommandsUtil::Data, F("010")));

		// String(F("Name:")).toCharArray(commandString, 15);
		btSerial.println(BlueToothCommandsUtil::CommandConstructor("Name:" + _deviceName, BlueToothCommandsUtil::Data, F("011")));

		btSerial.println(BlueToothCommandsUtil::CommandConstructor("Addr2:" + _deviceAddress2, BlueToothCommandsUtil::Data, F("015")));

		// String(F("Name:")).toCharArray(commandString, 15);
		btSerial.println(BlueToothCommandsUtil::CommandConstructor("Name2:" + _deviceName2, BlueToothCommandsUtil::Data, F("016")));

		// String(F("FindTime.:")).toCharArray(commandString, 15);
		btSerial.println(BlueToothCommandsUtil::CommandConstructor("FindLoop:" + String(_delayFindMe), BlueToothCommandsUtil::Data, F("094")));
	}

	// String(F("Find phone:")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("FindMode:" + String(_findOutPhonesMode), BlueToothCommandsUtil::Data, F("012")));

	// String(F("Ext.Int:")).toCharArray(commandString, 15);
	btSerial.println(BlueToothCommandsUtil::CommandConstructor("Ext.Int:" + String(_isExternalInterruptOn), BlueToothCommandsUtil::Data, F("013")));

	btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	// delete(commandString);
}

void blueToothConfigurationSystem()
{
	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();
	String _bluetoothData = "";
	if (btSerial.available())
	{
		_bluetoothData = btSerial.readString();
		// BluetoothData.trim();

		// ROOT: Main
#pragma region Main Menu-#0
		if (_bluetoothData.indexOf(F("#0")) > -1)
		{
			loadMainMenu();
		}

#pragma region Commands

		if (_bluetoothData.indexOf(F("C002")) > -1)
		{
			// do something
			// digitalWrite(_pin_powerLed, HIGH);
			_isAlarmOn = true;
			_isOnMotionDetect = false;
			_timeForSetBTConfiguration = 0;
			// btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Portable Alarm ON"), BlueToothCommandsUtil::Title));
			/*	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Alarm ON"), BlueToothCommandsUtil::Message));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
			loadMainMenu();
		}

		if (_bluetoothData.indexOf(F("C003")) > -1)
		{
			// do something
			_isAlarmOn = false;

			// digitalWrite(_pin_powerLed, LOW);
			// btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Portable Alarm OFF"), BlueToothCommandsUtil::Title));
			/*	btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Alarm OFF"), BlueToothCommandsUtil::Message));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
			loadMainMenu();
		}
#pragma endregion

#pragma region Data

#pragma endregion

#pragma endregion

		// ROOT Main/Configuration
#pragma region Configuration Menu-#M001
		if (_bluetoothData.indexOf(F("M001")) > -1)
		{
			loadConfigurationMenu();
		}
#pragma region Commands

#pragma endregion

#pragma region Data

		// if (_bluetoothData.indexOf(F("D097")) > -1)
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
		//	/*btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("FindMode updated"), BlueToothCommandsUtil::Message));
		//	btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
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

		// if (_bluetoothData.indexOf(F("D096")) > -1)
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

		// if (_bluetoothData.indexOf(F("D002")) > -1)
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
		// }

		// if (_bluetoothData.indexOf(F("D003")) > -1)
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

		// if (_bluetoothData.indexOf(F("D033")) > -1)
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
		//	/*btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("BTSleep updated"), BlueToothCommandsUtil::Message));
		//	btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));*/
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
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Security"), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw.:"), BlueToothCommandsUtil::Menu, F("005")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change name:"), BlueToothCommandsUtil::Menu, F("006")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}
#pragma region Menu
		if (_bluetoothData.indexOf(F("M005")) > -1)
		{
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Insert old passw.:"), BlueToothCommandsUtil::Data, F("006")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("M006")) > -1)
		{
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Insert name:"), BlueToothCommandsUtil::Data, F("007")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
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
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Insert new passw:"), BlueToothCommandsUtil::Data, F("008")));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}
			else
			{
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Wrong passw:"), BlueToothCommandsUtil::Message));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			}
		}

		if (_bluetoothData.indexOf(F("D008")) > -1)
		{
			_newPassword = splitStringIndex(_bluetoothData, ';', 1);
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Confirm pass:"), BlueToothCommandsUtil::Data, F("009")));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
		}

		if (_bluetoothData.indexOf(F("D009")) > -1)
		{
			if (_newPassword == splitStringIndex(_bluetoothData, ';', 1))
			{
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				delay(2000);
				btSerial.SetPassword(_newPassword);
				_oldPassword = _newPassword;
			}

			else
			{
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("passw. doesn't match"), BlueToothCommandsUtil::Message));
				btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
				btSerial.println("D006");
			}
		}

		if (_bluetoothData.indexOf(F("D007")) > -1)
		{
			String btName = splitStringIndex(_bluetoothData, ';', 1);

			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("Change passw."), BlueToothCommandsUtil::Title));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(F("changed:"), BlueToothCommandsUtil::Message));
			btSerial.println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
			delay(2000);
			btSerial.SetBlueToothName(btName);
		}

#pragma endregion

#pragma endregion

		delay(100);
	}
	delete (eepromRW);
}

boolean isValidNumber(String str)
{
	for (byte i = 0; i < str.length(); i++)
	{
		if (isDigit(str.charAt(i)))
			return true;
	}
	return false;
}

unsigned long deltaTimeForOpenTheDoor = 0;

void openGarageDoorWithPhone()
{
	if (isGarageDoorClosed() && _doorState == 1)
	{
		if (deltaTimeForOpenTheDoor == 0)
		{
			deltaTimeForOpenTheDoor = millis();
			// Serial.println("Attesa per riapertura garage");
		}
		if ((millis() - deltaTimeForOpenTheDoor) > 45000)
		{
			// Serial.println("Garage pronto per riapertura");
			_doorState = 0;
			deltaTimeForOpenTheDoor = 0;
		}
	}
	if (/*_isPIRSensorActivated &&*/ _isAlarmOn)
	{
		/*	if (isThereSomeOneInFrontOfGarage())
			{*/
			/*Serial.println("isThereSomeOneInFrontOfGarage");*/

			// isMyPhoneDetected();

		_whatIsHappened = F("P");

		if (_findOutPhonesMode == 1 && _isPhoneDeviceDetected && isGarageDoorClosed() && _doorState == 0)
		{
			blinkLed();
			buzzerFunction(buzzerPin, 100, 1000);
			// Serial.println("Apertura garage");
			_doorState = 1;
			// Aggiungere codice che gestisce interrupt pin aperto.
			reedRelaySensorActivity(relayPin);
			delay(60000);
		}
		/*else if ((isAM() && hour() < 6) && !_isPhoneDeviceDetected)
		{
			blinkLed();
			String message = "P01N";
			sendMessageToComunicatorDevice(message);
		}*/

		//}
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
		}
		/*delete chipTemp;*/
	}
}

void voltageActivity()
{
	if (_delayForVoltage->IsDelayTimeFinished(true))
	{
		// Serial.println(analogRead(voltagePin));
		_voltageValue = (5.1 / 1024.00) * analogRead(voltagePin);
		// Serial.println(_voltageValue);
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
	//_timeToTurnOfBTAfterPowerOn = 0;
	_timeForSetBTConfiguration = 0;
	//_isDisableCall = false;
	_isAlarmOn = true;
	// callSim900();
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
	ADCSRA |= _BV(ADEN); // enable the ADC

	delay(20); // wait for voltages to become stable.

	ADCSRA |= _BV(ADSC); // Start the ADC

	// Detect end-of-conversion
	while (bit_is_set(ADCSRA, ADSC))
		;

	// Reading register "ADCW" takes care of how to read ADCL and ADCH.
	wADC = ADCW;

	// The offset of 324.31 could be wrong. It is just an indication.
	t = (wADC - _offSetTempValue) / 1.22;

	// The returned temperature is in degrees Celsius.
	return (t);
}

// unsigned int offSetTempValue(double externalTemperature)
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
// }

// print function
// void serialPrint(String tempString, int tempData) {
//	Serial.print(tempString);
//	Serial.print(tempData, DEC);
//	Serial.print(" $");
//	Serial.println(tempData, HEX);
// }

// computes the free memory (from JeeLabs)
int freeRam()
{
	int v;
	temp = (int)&v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void sendMessageToComunicatorDevice(String message)
{
	digitalWrite(softwareSerialExternalDevicesPinAlarm, LOW);
	bool isMessageReceived = false;
	delay(1000);
	while (!isMessageReceived)
	{
		if (softwareSerial.available() > 0)
		{
			String messageReceived = softwareSerial.readString();

			if (messageReceived.startsWith("H"))
			{
				// Serial.print(F("Trasm: ")); Serial.println(message);
				softwareSerial.print(message);
				softwareSerial.print("*");
				isMessageReceived = true;
			}
		}
	}
	digitalWrite(softwareSerialExternalDevicesPinAlarm, HIGH);
}

void buzzerFunction(byte buzzerPin, int frequency, int time)
{
	if (isBuzzerDisable)
		return;
	tone(buzzerPin, frequency, time);
	delay(time * 2);
}
