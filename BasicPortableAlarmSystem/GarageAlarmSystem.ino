
#if (defined(__AVR__))
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif
#include "MyBlueTooth.h"
#include "BlueToothCommandsUtil.h"
#include "LSGEEpromRW.h" 
#include <EEPROM.h> 
#include "MySim900.h"
#include "ActivityManager.h"
#include <TimeLib.h>

char version[15] = "-G001 1.0-alfa";

ActivityManager* _delayForTemperature = new ActivityManager(60);

ActivityManager* _delayForVoltage = new ActivityManager(60);

//ActivityManager* _delayForDialCall = new ActivityManager(1); 

ActivityManager* _delayForFindPhone = new ActivityManager(30);

ActivityManager* _delayForSignalStrength = new ActivityManager(30);

MyBlueTooth* btSerial;

MySim900* mySim900;

String _oldPassword = "";

String _newPassword = "";

const byte _pin_powerLed = 13;

const byte _pin_rxSIM900 = 7;

const byte _pin_txSIM900 = 8;

const byte _addressStartBufPhoneNumber = 1;

const byte _addressStartBufPrecisionNumber = 12;

const byte _addressStartBufTemperatureIsOn = 14;

const byte _addressStartBufTemperatureMax = 16;

const byte _addressStartBufPirSensorIsON = 19;

const byte _addressStartDeviceAddress = 21;

const byte _addressStartDeviceName = 33;

const byte _addressStartFindOutPhonesON = 48;

const byte _addressStartBTSleepIsON = 50;

const byte _addressDBPhoneIsON = 52;

const byte _addressStartBufPhoneNumberAlternative = 54;

const byte _addressStartFindMode = 65;

//const byte _addressApn = 67;

const byte _addressOffSetTemperature = 92;

const byte _addressDelayFindMe = 94;

const byte _addressExternalInterruptIsOn = 96;


uint8_t _isPIRSensorActivated = 0;

bool _isDisableCall = false;

bool _isOnMotionDetect = false;

bool _isFirstTilt = true;

unsigned long _sensitivityAlarm;

uint8_t _precision = 0;

char _prefix[4] = "+39";

bool _isAlarmOn = false;

String _phoneNumber;

String _phoneNumberAlternative;

String _whatIsHappened = "";

//uint8_t _isTemperatureCheckOn = 0;

uint8_t _isBTSleepON = 1;

uint8_t _isExternalInterruptOn = 0;

uint8_t _phoneNumbers = 0;

uint8_t _findOutPhonesMode = 0;

uint8_t _tempMax = 0;

uint8_t _delayFindMe = 1;

unsigned int _offSetTempValue = 324.13;

//String _signalStrength;

String _deviceAddress = "";

String _deviceName = "";

float _voltageValue = 0;

float _voltageMinValue = 0;

unsigned long _millsStart = 0;

bool _isMasterMode = false;

unsigned long _pirSensorTime = 0;

unsigned long timeToTurnOfBTAfterPowerOn = 300000;

unsigned long _timeAfterPowerOnForBTFinder = 300000;

//String _apn = "";

bool _isDeviceDetected = false;

const int BUFSIZEPHONENUMBER = 11;
char _bufPhoneNumber[BUFSIZEPHONENUMBER];

const int BUFSIZEPHONENUMBERALTERANATIVE = 11;
char _bufPhoneNumberAlternative[BUFSIZEPHONENUMBERALTERANATIVE];

const int BUFSIZEPRECISION = 2;
char _bufPrecisionNumber[BUFSIZEPRECISION];

const int BUFSIZETEMPERATUREISON = 2;
char _bufTemperatureIsOn[BUFSIZETEMPERATUREISON];

//const int BUFSIZEFINDMODE = 2;
//char _bufFindMode[BUFSIZEFINDMODE];

const int BUFSIZEPIRSENSORISON = 2;
char _bufPirSensorIsON[BUFSIZEPIRSENSORISON];

//const int BUFSIZEBTSLEEPISON = 2;
//char _bufBTSleepIsON[BUFSIZEBTSLEEPISON];

const int BUFSIZEFINDOUTPHONESON = 2;
char _bufFindOutPhonesON[BUFSIZEFINDOUTPHONESON];

const int BUFSIZEDBPHONEON = 2;
char _bufDbPhoneON[BUFSIZEDBPHONEON];

const int BUFSIZETEMPERATUREMAX = 3;
char _bufTemperatureMax[BUFSIZETEMPERATUREMAX];

const int BUFSIZEDEVICEADDRESS = 13;
char _bufDeviceAddress[BUFSIZEDEVICEADDRESS];

const int BUFSIZEDEVICENAME = 15;
char _bufDeviceName[BUFSIZEDEVICENAME];

//const int BUFSIZEAPN = 25;
//char _bufApn[BUFSIZEAPN];

const int BUFSIZEOFFSETTEMPERATURE = 5;
char _bufOffSetTemperature[BUFSIZEOFFSETTEMPERATURE];

const int BUFSIZEDELAYFINDME = 2;
char _bufDelayFindMe[BUFSIZEDELAYFINDME];

const int BUFSIZEEXTERNALINTERRUPTISON = 2;
char _bufExternalInterruptIsON[BUFSIZEEXTERNALINTERRUPTISON];

unsigned long _timeLastCall = 0;

void setup()
{
	mySim900 = new MySim900(_pin_rxSIM900, _pin_txSIM900, false);

	mySim900->Begin(19200);

	mySim900->IsCallDisabled(false);

	inizializePins();

	inizializeInterrupts();

	btSerial = new MyBlueTooth(&Serial, 10, 6, 38400, 9600);

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

	_sensitivityAlarm = 2000 + ((_precision) * 500);

	btSerial->turnOnBlueTooth();

	_whatIsHappened = F("X");

	//Stare attenti perche' richiede un ritardo che ora � dato da creazione oggetti bluetooth.
	//mySim900->WaitSMSComing();


	mySim900->ATCommand("AT+CPMS=\"SM\"");
	//delay(5000);
	/*if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/
	mySim900->ATCommand("AT+CMGF=1");
	/*delay(5000);
	if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/
	mySim900->ATCommand("AT+CMGD=1,4");
	/*if (mySim900->IsAvailable() > 0)
	{
		String s = mySim900->ReadIncomingChars2();
		Serial.println(s);
	}*/

	blinkLed();
}

void initilizeEEPromData()
{


	EEPROM.write(0, 1);

	LSG_EEpromRW* eepromRW = new LSG_EEpromRW();

	eepromRW->eeprom_read_string(_addressStartBufPhoneNumber, _bufPhoneNumber, BUFSIZEPHONENUMBER);

	_phoneNumber = String(_bufPhoneNumber);

	eepromRW->eeprom_read_string(_addressStartBufPhoneNumberAlternative, _bufPhoneNumberAlternative, BUFSIZEPHONENUMBERALTERANATIVE);

	_phoneNumberAlternative = String(_bufPhoneNumberAlternative);

	eepromRW->eeprom_read_string(_addressDBPhoneIsON, _bufDbPhoneON, BUFSIZEDBPHONEON);

	_phoneNumbers = atoi(&_bufDbPhoneON[0]);

	//eepromRW->eeprom_read_string(_addressStartBTSleepIsON, _bufBTSleepIsON, BUFSIZEBTSLEEPISON);

	//_isBTSleepON = atoi(&_bufBTSleepIsON[0]);


	/*eepromRW->eeprom_read_string(_addressStartFindMode, _bufFindMode, BUFSIZEFINDMODE);

	_findMode = atoi(&_bufFindMode[0]);*/


	/*eepromRW->eeprom_read_string(_addressStartBufTemperatureIsOn, _bufTemperatureIsOn, BUFSIZETEMPERATUREISON);

	_isTemperatureCheckOn = atoi(&_bufTemperatureIsOn[0]);
*/
	eepromRW->eeprom_read_string(_addressStartFindOutPhonesON, _bufFindOutPhonesON, BUFSIZEFINDOUTPHONESON);

	_findOutPhonesMode = atoi(&_bufFindOutPhonesON[0]);

	eepromRW->eeprom_read_string(_addressStartBufPirSensorIsON, _bufPirSensorIsON, BUFSIZEPIRSENSORISON);

	_isPIRSensorActivated = atoi(&_bufPirSensorIsON[0]);

	eepromRW->eeprom_read_string(_addressStartBufPrecisionNumber, _bufPrecisionNumber, BUFSIZEPRECISION);

	_precision = atoi(&_bufPrecisionNumber[0]);

	eepromRW->eeprom_read_string(_addressStartBufTemperatureMax, _bufTemperatureMax, BUFSIZETEMPERATUREMAX);

	_tempMax = atoi(_bufTemperatureMax);


	eepromRW->eeprom_read_string(_addressStartDeviceAddress, _bufDeviceAddress, BUFSIZEDEVICEADDRESS);

	_deviceAddress = String(_bufDeviceAddress);

	eepromRW->eeprom_read_string(_addressStartDeviceName, _bufDeviceName, BUFSIZEDEVICENAME);

	_deviceName = String(_bufDeviceName);

	/*eepromRW->eeprom_read_string(_addressApn, _bufApn, BUFSIZEAPN);

	_apn = String(_bufApn);

	_apn.trim();*/


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
}

void inizializeInterrupts()
{
	attachInterrupt(0, motionTiltInternalInterrupt, RISING);
	attachInterrupt(1, motionTiltExternalInterrupt, RISING);
}

void callSim900(char isLongCaller)
{
	/*if (_delayForDialCall->IsDelayTimeFinished(true))
	{*/

	if (_isDisableCall) { return; }

	//Clear buffer before call
	mySim900->ReadIncomingChars2();

	String phoneNumber = _prefix + _phoneNumber;
	String phoneNumberAlternative = _prefix + _phoneNumberAlternative;
	char completePhoneNumber[14];
	char completePhoneNumberAlternative[14];
	phoneNumber.toCharArray(completePhoneNumber, 14);
	phoneNumberAlternative.toCharArray(completePhoneNumberAlternative, 14);

	unsigned long callTime;

	switch (isLongCaller)
	{
	case '0':
		callTime = 30000;
		break;
	case '1':
		callTime = 40000;
		break;
	deafault:
		callTime = 40000;
		break;
	}

	mySim900->DialVoiceCall(completePhoneNumber);

	if (_phoneNumbers == 2)
	{

		delay(callTime);

		mySim900->ATCommand("AT+CHUP");

		delay(2000);

		mySim900->DialVoiceCall(completePhoneNumberAlternative);

		if (isLongCaller != 1)
		{
			delay(callTime);
			mySim900->ATCommand("AT+CHUP");
		}
		//mySim900->ReadIncomingChars2();

	}

	mySim900->ClearBuffer(2000);

	/*if (_findOutPhonesMode == 0 || _findOutPhonesMode == 1)
	{*/
	turnOnBlueToothAndSetTurnOffTimer(false);
	//}
//}

}

void motionTiltExternalInterrupt() {
	if (_isExternalInterruptOn /*&& !_isPIRSensorActivated*/) {
		_isOnMotionDetect = true;
	}
}

void motionTiltInternalInterrupt()
{
	/*if (!_isPIRSensorActivated) {*/
	_isOnMotionDetect = true;
	//}
}

//String getSignalStrength()
//{
//	String signalStrength = "";
//	signalStrength = mySim900->GetSignalStrength();
//	return signalStrength;
//}

void turnOffBluetoohIfTimeIsOver()
{
	if (_findOutPhonesMode == 0
		&& (millis() > timeToTurnOfBTAfterPowerOn)
		&& btSerial->isBlueToothOn()
		&& _isBTSleepON
		)
	{
		btSerial->turnOffBlueTooth();
		digitalWrite(13, HIGH);
		delay(5000);
		digitalWrite(13, LOW);
	}
}

void turnOnBlueToothIfMotionIsDetected()
{
	if (_isOnMotionDetect
		&& !_isAlarmOn
		&& btSerial->isBlueToothOff()
		&& _isBTSleepON
		)
	{
		_isOnMotionDetect = false;
		turnOnBlueToothAndSetTurnOffTimer(false);
	}
}

bool isFindOutPhonesONAndSetBluetoothInMasterMode()
{
	if (_isDisableCall) { return; }

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
			_isDeviceDetected = btSerial->IsDeviceDetected(_deviceAddress, _deviceName);

			if (_isDeviceDetected) { break; };
		}

		//bool isHumanDetected = pirSensor->isHumanDetected();
		if (_isDeviceDetected
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
				callSim900('1');
				//delay(10000);
				_isMasterMode = false;
			}
		}

		/*	delete(pirSensor);*/
		return _isDeviceDetected;
	}
}

//Test used only for test tilt sensor.
//void testForTiltSensor()
//{
//	Serial.print("digitalRead = "); Serial.print(digitalRead(2));
//	Serial.print("--pinPowerBluetooth = "); Serial.print(digitalRead(6));
//	Serial.print("--_isOnMotionDetect = "); Serial.println(_isOnMotionDetect);
//
//	if (_isOnMotionDetect == true) {
//		_isOnMotionDetect = false;
//		Serial.println("Eccezione rilevata");
//		//delay(1000000000);
//	}
//	delay(1000);
//}

void loop()
{
	//Serial.print(hour()); Serial.print(":"); Serial.print(minute()); Serial.print(":"); Serial.println(second());
	//testForTiltSensor()
	//return;

	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		readIncomingSMS();
	}

	/*if (_delayForSignalStrength->IsDelayTimeFinished(true))
	{
		_signalStrength = getSignalStrength();
	}*/

	//if (_delayForCheckBlueToothWorking->IsDelayTimeFinished(true) && !_isBTSleepON && _isFindOutPhonesON == 0)
	//{
	//	restartBlueTooth();
	//}

	if ((!(_isOnMotionDetect && _isAlarmOn)) || _findOutPhonesMode == 2)
	{
		if (_delayForFindPhone->IsDelayTimeFinished(true))
		{
			//Serial.println("Sto cercando");
			isFindOutPhonesONAndSetBluetoothInMasterMode();
		}
	}

	//if (_delayForCallNumbers->IsDelayTimeFinished(true))
	//{
	//	_callNumbers = 0;
	//}
	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		turnOffBluetoohIfTimeIsOver();
	}
	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		turnOnBlueToothIfMotionIsDetected();
	}
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
	isMotionDetect();
	if (!(_isOnMotionDetect && _isAlarmOn))
	{
		blueToothConfigurationSystem();
	}
	//getCoordinates();
}

void isMotionDetect()
{
	if (_isDisableCall || _findOutPhonesMode == 2 /*|| _isPIRSensorActivated*/) {
		_isOnMotionDetect = false;
		//readIncomingSMS();
		return;
	}
	if ((millis() - _millsStart) > _sensitivityAlarm)
	{
		_millsStart = 0;
		_isFirstTilt = true;
	}

	if ((_isOnMotionDetect && _isAlarmOn) || (_isAlarmOn && _isExternalInterruptOn && !digitalRead(3))) //&& !isOnConfiguration)									 /*if(true)*/
	{
		//Serial.println("lampeggio");
		blinkLed();

		detachInterrupt(0);
		detachInterrupt(1);

		if ((!_isFirstTilt || (_precision == 9)) && _precision != 0)
		{
			_whatIsHappened = F("M");

			if (_findOutPhonesMode == 1)
			{
				if (!_isDeviceDetected)
				{
					callSim900('1');

				}
			}
			else
			{
				callSim900('1');

			}
			//Accendo bluetooth con ritardo annesso solo se � scattato allarme,troppo critico
			//per perdere tempo se non scattato allarme.
			if (btSerial->isBlueToothOff() && _findOutPhonesMode == 0)
			{
				delay(30000);
				turnOnBlueToothAndSetTurnOffTimer(false);
			}
			//}

			readIncomingSMS();

			isFindOutPhonesONAndSetBluetoothInMasterMode();
		}
		else
		{
			_isFirstTilt = false;
			_millsStart = millis();
		}

		EIFR |= 1 << INTF1; //clear external interrupt 1
		EIFR |= 1 << INTF0; //clear external interrupt 0
		//EIFR = 0x01;
		sei();

		attachInterrupt(0, motionTiltInternalInterrupt, RISING);
		attachInterrupt(1, motionTiltExternalInterrupt, RISING);

		_isOnMotionDetect = false;
	}
}

//void restartBlueTooth()
//{
//	Serial.readString();
//	btSerial->ReceveMode();
//}

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
		timeToTurnOfBTAfterPowerOn = millis() + 300000;
		_timeAfterPowerOnForBTFinder = millis() + 120000;
	}
	_isMasterMode = false;
}

void blinkLed()
{
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

	char* commandString = new char[15];

	String(F("Configuration")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString, BlueToothCommandsUtil::Menu, F("001")));

	String(F("Security")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString, BlueToothCommandsUtil::Menu, F("004")));

	if (!_isAlarmOn)
	{
		String(F("Alarm On")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString, BlueToothCommandsUtil::Command, F("002")));
	}
	else
	{
		String(F("Alarm OFF")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString, BlueToothCommandsUtil::Command, F("003")));
	}

	String(F("Temp.:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(internalTemperature), BlueToothCommandsUtil::Info));

	/*btSerial->println(BlueToothCommandsUtil::CommandConstructor("Batt.value:" + String(_voltageValue), BlueToothCommandsUtil::Info));*/

	String(F("Batt.level:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + battery, BlueToothCommandsUtil::Info));

	String(F("WhatzUp:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + _whatIsHappened, BlueToothCommandsUtil::Info));

	String(F("Time:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(hour()) + ":" + String(minute()), BlueToothCommandsUtil::Info));

	/*String(F("Signal:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + _signalStrength, BlueToothCommandsUtil::Info));*/

	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	btSerial->Flush();
	delete(commandString);
}

void loadConfigurationMenu()
{
	char* commandString = new char[15];
	String(F("Configuration")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString, BlueToothCommandsUtil::Title));

	String(F("Phone:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + _phoneNumber, BlueToothCommandsUtil::Data, F("001")));

	String(F("Ph.Altern.:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + _phoneNumberAlternative, BlueToothCommandsUtil::Data, F("099")));

	String(F("N.Phone:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_phoneNumbers), BlueToothCommandsUtil::Data, F("098")));

	String(F("TempMax:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_tempMax), BlueToothCommandsUtil::Data, F("004")));

	String(F("OffSetTemp:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_offSetTempValue), BlueToothCommandsUtil::Data, F("095")));

	/*String(F("Apn:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + _apn, BlueToothCommandsUtil::Data, F("096")));*/

	if (!_isPIRSensorActivated && _findOutPhonesMode == 0)
	{
		String(F("Prec.:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_precision), BlueToothCommandsUtil::Data, F("002")));
	}

	/*String(F("TempON:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_isTemperatureCheckOn), BlueToothCommandsUtil::Data, F("003")));*/



	if (_findOutPhonesMode != 2)
	{
		String(F("PIR status:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_isPIRSensorActivated), BlueToothCommandsUtil::Data, F("005")));
	}

	if (_findOutPhonesMode != 0)
	{
		String(F("Addr:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + _deviceAddress, BlueToothCommandsUtil::Data, F("010")));

		String(F("Name:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + _deviceName, BlueToothCommandsUtil::Data, F("011")));

		String(F("FindTime.:")).toCharArray(commandString, 15);
		btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_delayFindMe), BlueToothCommandsUtil::Data, F("094")));

	}

	String(F("Find phone:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_findOutPhonesMode), BlueToothCommandsUtil::Data, F("012")));

	String(F("Ext.Int:")).toCharArray(commandString, 15);
	btSerial->println(BlueToothCommandsUtil::CommandConstructor(commandString + String(_isExternalInterruptOn), BlueToothCommandsUtil::Data, F("013")));



	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
	delete(commandString);
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
		if (_bluetoothData.indexOf(F("D001")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEPHONENUMBER = 11;
				char _bufPhoneNumber[BUFSIZEPHONENUMBER];*/

				splitString.toCharArray(_bufPhoneNumber, BUFSIZEPHONENUMBER);
				eepromRW->eeprom_write_string(1, _bufPhoneNumber);
				_phoneNumber = splitString;
			}
			loadConfigurationMenu();
		}

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

		if (_bluetoothData.indexOf(F("D098")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEDBPHONEON = 2;
				char _bufDbPhoneON[BUFSIZEDBPHONEON];*/

				splitString.toCharArray(_bufDbPhoneON, BUFSIZEDBPHONEON);
				eepromRW->eeprom_write_string(_addressDBPhoneIsON, _bufDbPhoneON);
				_phoneNumbers = atoi(&_bufDbPhoneON[0]);
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D099")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*const int BUFSIZEPHONENUMBERALTERNATIVE = 11;
				char _bufPhoneNumberAlternative[BUFSIZEPHONENUMBERALTERNATIVE];*/

				splitString.toCharArray(_bufPhoneNumberAlternative, BUFSIZEPHONENUMBERALTERANATIVE);
				eepromRW->eeprom_write_string(_addressStartBufPhoneNumberAlternative, _bufPhoneNumberAlternative);
				_phoneNumberAlternative = splitString;
			}
			loadConfigurationMenu();
		}

		if (_bluetoothData.indexOf(F("D002")) > -1)
		{
			String splitString = splitStringIndex(_bluetoothData, ';', 1);
			if (isValidNumber(splitString))
			{
				/*char _bufPrecisionNumber[2];*/
				splitString.toCharArray(_bufPrecisionNumber, 2);
				eepromRW->eeprom_write_string(12, _bufPrecisionNumber);
				_precision = atoi(&_bufPrecisionNumber[0]);
				_sensitivityAlarm = 2000 + ((_precision) * 500);
			}
			loadConfigurationMenu();
		}

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

#pragma endregion


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

//void updateCommand()
//{
//	btSerial->println(BlueToothCommandsUtil::CommandConstructor(F("Updated"), BlueToothCommandsUtil::Message));
//	btSerial->println(BlueToothCommandsUtil::CommandConstructor(BlueToothCommandsUtil::EndTrasmission));
//}

boolean isValidNumber(String str)
{
	for (byte i = 0; i < str.length(); i++)
	{
		if (isDigit(str.charAt(i))) return true;
	}
	return false;
}

void pirSensorActivity()
{
	if (_isDisableCall) { return; }

	if (_isPIRSensorActivated && _isAlarmOn && digitalRead(5))
	{
		blinkLed();
		_whatIsHappened = F("P");

		if (isAM() && hour() < 6)
		{
			if ((millis() - _pirSensorTime) > 40000)
			{
				_pirSensorTime = 0;
			}
			if (_pirSensorTime != 0)
			{
				//Serial.println("Alarme scattato");
				callSim900('1');
				_isMasterMode = false;
			}
			else
			{
				_pirSensorTime = millis();
			}

			delay(15000);
		}
		else if (_findOutPhonesMode == 1 && _isDeviceDetected && digitalRead(3))
		{
			//Aggiungere codice che gestisce interrupt pin aperto.
			reedRelaySensorActivity(A5);
			delay(15000);
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
			callSim900('1');
		}
		/*delete chipTemp;*/
	}
}

void voltageActivity()
{
	if (_delayForVoltage->IsDelayTimeFinished(true))
	{
		_voltageValue = (5.10 / 1023.00) * analogRead(A1);
		_voltageMinValue = 3.25;

		if (_voltageValue < _voltageMinValue)
		{
			_whatIsHappened = F("V");
			callSim900('1');
		}
	}
}

void readIncomingSMS()
{
	mySim900->ATCommand("AT+CMGL");//=\"REC UNREAD\"");
	//mySim900->ATCommand("AT+CMGR=1");
	delay(100);
	/**/

	if (mySim900->IsAvailable() > 0)
	{
		String response = mySim900->ReadIncomingChars2();
		//Serial.println(response);
		response.trim();
		//if (response.substring(0, 5) == F("+CMT:"))
		//if (response.indexOf("+CMT:") != -1)
		if (response.indexOf("+CMGL:") != -1)
		{
			blinkLed();
			int index = response.lastIndexOf('"');
			String smsCommand = response.substring(index + 1, index + 8);
			smsCommand.trim();
			//Serial.println(smsCommand);
			delay(1000);
			listOfSmsCommands(smsCommand);
		}
	}
}

void listOfSmsCommands(String command)
{
	_timeLastCall = 0;
	command.trim();
	//Imposta ora di sistema
	if (command.startsWith("H"))
	{
		String hour = command.substring(1, 3);
		String minute = (command.substring(3, 5));
		setTime(hour.toInt(), minute.toInt(), 1, 1, 1, 2019);
		callSim900('0');
	}
	//Accende il sistema
	if (command == F("Ac"))
	{
		_isDisableCall = false;
		_isMasterMode = false;
		callSim900('0');
	}
	//Spegne il sistema
	if (command == F("Sp"))
	{
		//Serial.println("Disabilito chiamate");
		callSim900('0');
		_isDisableCall = true;

	}
	//Allarme ON
	if (command == F("Ao"))
	{
		_isDisableCall = false;
		callSim900('0');
		if (_findOutPhonesMode == 1 || _findOutPhonesMode == 2)
		{
			_timeAfterPowerOnForBTFinder = 0;
		}
		_isAlarmOn = true;

	}
	//Accende bluetooth
	if (command == F("Ab"))
	{
		turnOnBlueToothAndSetTurnOffTimer(true);
	}
	//Spegne bluetooth
	if (command == F("Sb"))
	{
		btSerial->turnOffBlueTooth();
		callSim900('0');
	}
	////Coordinate geolocalizzazione.
	//if (command == F("Po"))
	//{
	//	getCoordinates();
	//}
}

//void getCoordinates()
//{
//	mySim900->ReadIncomingChars2();
//
//	char * apnCommand = new char[50];
//
//	char * apnString = new char[25];
//
//	_apn.toCharArray(apnString, (_apn.length() + 1));
//
//	strcpy(apnCommand, "AT + SAPBR = 3, 1,\"APN\", \"");
//
//	strcat(apnCommand, apnString);
//
//	strcat(apnCommand, "\"");
//
//
//	//Serial.println(apnCommand);
//
//	mySim900->ATCommand(apnCommand);
//
//	delete(apnString);
//
//	delete(apnCommand);
//
//	//"AT + SAPBR = 3, 1, \"Contype\", \"GPRS\""
//	/*mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"internet.wind\"");*/
//	/*mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"web.coopvoce.it\"");*/
//	//mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"mobile.vodafone.it\"");
//	//mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"wap.tim.it\"");
//	/*mySim900->ATCommand("AT + SAPBR = 3, 1,\"APN\", \"ibox.tim.it\"");*/
//
//	delay(1500);
//	//if (mySim900->IsAvailable() > 0)
//	//{
//	//	/*Serial.println(mySim900->ReadIncomingChars2());*/
//	//	mySim900->ReadIncomingChars2();
//
//	//}
//
//	
//
//	mySim900->ATCommand("AT + SAPBR = 0, 1");
//	delay(2000);
//	//if (mySim900->IsAvailable() > 0)
//	//{
//	//	//Serial.println(mySim900->ReadIncomingChars2());
//	//	mySim900->ReadIncomingChars2();
//
//	//}
//	mySim900->ATCommand("AT + SAPBR = 1, 1");
//	delay(2000);
//	//if (mySim900->IsAvailable() > 0)
//	//{
//	//	//Serial.println(mySim900->ReadIncomingChars2());
//	//	mySim900->ReadIncomingChars2();
//
//	//}
//	mySim900->ATCommand("AT + SAPBR = 2, 1");
//	delay(1500);
//	//if (mySim900->IsAvailable() > 0)
//	//{
//	//	//Serial.println(mySim900->ReadIncomingChars2());
//	//	mySim900->ReadIncomingChars2();
//
//	//}
//	mySim900->ReadIncomingChars2();
//	mySim900->ATCommand("AT + CIPGSMLOC = 1, 1");
//	delay(10000);
//	if (mySim900->IsAvailable() > 0)
//	{
//		String h = mySim900->ReadIncomingChars2();
//		h.trim();
//		/*Serial.println(h);*/
//		/*Serial.println(h);
//		Serial.println(h.indexOf(F("+CIPGSMLOC:")));
//		Serial.println(h.substring(24, 35));*/
//
//		if (h.substring(24, 35) == F("+CIPGSMLOC:"))
//		{
//			/*Serial.println("Entrato");*/
//			String a = splitStringIndex(h, ',', 3);
//			String b = splitStringIndex(h, ',', 2);
//			//String c = F(",");
//			a.trim(); b.trim();
//			String site = F("google.com/maps/search/?api=1&query=");
//			site = site + a + F(","); //+ F(",") + b;
//			//site = site + c;
//			site = site + b;
//			/*Serial.println(site);*/
//			if (a != F("") && b != F(""))
//			{
//				mySim900->SendTextMessageSimple(site, _prefix + _phoneNumber);
//			}
//		}
//
//		
//
//	}
//	//Resetta la sim dopo SAPBR 1,1
//	delay(10000);
//	mySim900->ATCommand("AT + SAPBR = 0, 1");
//}

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