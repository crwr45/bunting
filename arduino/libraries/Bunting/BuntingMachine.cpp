#include "Arduino.h"
#include "BasicStepperDriver.h"
#include "BuntingMachine.h"

BuntingMachine::BuntingMachine(int dir, short steps, short dir_pin, short step_pin, 
							   short enable_pin, short stopperPin, 
							   int deployDegrees, short rpm, short microsteps)
: _stepperDriver(steps, dir_pin, step_pin, enable_pin)
{
	_dir_multi = dir;
	_stopperPin = stopperPin;
	_deployDegrees = deployDegrees;
	pinMode(_stopperPin, INPUT_PULLUP);
	
	_stepperDriver.begin(rpm, microsteps);
}

BuntingMachine::BuntingMachine(BasicStepperDriver stepper, int dir, short stopperPin, 
							   int deployDegrees, short rpm, short microsteps)
: _stepperDriver(stepper)
{
	_dir_multi = dir;
	_stopperPin = stopperPin;
	_deployDegrees = deployDegrees;
	pinMode(_stopperPin, INPUT_PULLUP);

	_stepperDriver.begin(rpm, microsteps);
}

void BuntingMachine::_reverse() {
	if (_direction == DEPLOYING) {
		_stepperDriver.move(150 * _dir_multi);
	} else {
		_stepperDriver.move(-150 * _dir_multi);
	}
}

void BuntingMachine::setup(void (*ISR)(void)) {
	attachInterrupt(digitalPinToInterrupt(_stopperPin), ISR, FALLING);
	delay(100);
	store();
}

void BuntingMachine::stop() {
	_stepperDriver.stop();
}

void BuntingMachine::store() {
	if (_state != STORED) {
		Serial.print("Storing Bunting... ");
		enableMotor();
		_direction = STORING;
		_stepperDriver.rotate(-30 * _dir_multi); // Back off a small amount.
		_stepperDriver.rotate(100 * _deployDegrees * _dir_multi);
		disableMotor();
		Serial.println("Bunting Stored");
		_state = STORED;	
	}
}

void BuntingMachine::deploy() {
	if (_state != DEPLOYED) {
		Serial.print("Deploying Bunting... ");
		enableMotor();
		_direction = DEPLOYING;
		_stepperDriver.rotate(-1 * _deployDegrees * _dir_multi);
		disableMotor();
		Serial.println("Bunting Deployed");
		_state = DEPLOYED;
	}
}

void BuntingMachine::enableMotor() {
	_stepperDriver.enable();
}

void BuntingMachine::disableMotor() {
	_reverse();
	_stepperDriver.disable();
}
