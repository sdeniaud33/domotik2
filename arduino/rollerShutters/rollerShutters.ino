#include "ACS712.h"
#include "OneButton.h"

const float thresholdFactor = 0.8;

enum DirectionEnum
{
	directionNone,	// The shutter is stopped
	directionOpening, // The shutter is opening
	directionClosing  // The shutter is closing
};

enum PositionEnum
{
	positionUnknown, // The shutter is neither open, nor closed
	positionOpen,	// The shutter is open
	positionClosed   // The shutter is closed
};

struct TargetStruct
{
	bool enabled;
	int position;
};

struct PositionStruct
{
	float percent;
	int lastDisplayedPercent;
	PositionEnum status;
};

TargetStruct target;
PositionStruct position = {0, positionUnknown};

struct MovementStruct
{
	DirectionEnum direction;
	unsigned long startTime;	 // The startTime of the movement start
	float startPos;				 // The position of the ruller when the movement started
	long openingDuration;		 // Duration of full 'opening' movement
	long closingDuration;		 // Duration of full 'closing' movement
	bool measureClosingDuration; // Should the 'closing' full movement duration be measured ?
	bool measureOpeningDuration; // Should the 'opening' full movement duration be measured ?
};

MovementStruct movement;
bool lastDirectionWasOpening = false;
const int PIN_BTN = 2;
const int PIN_OPEN = 3;
const int PIN_CLOSE = 4;
float consumptionThreshold = 0;

// We have 30 amps version sensor connected to A1 pin of arduino
// Replace with your version if necessary
ACS712 sensor(ACS712_05B, A1);
OneButton button(PIN_BTN, true);

void setup()
{
	Serial.begin(9600);
	pinMode(PIN_OPEN, OUTPUT);
	digitalWrite(PIN_OPEN, LOW);
	pinMode(PIN_CLOSE, OUTPUT);
	digitalWrite(PIN_CLOSE, LOW);

	// This method calibrates zero point of sensor,
	// It is not necessary, but may positively affect the accuracy
	// Ensure that no current flows through the sensor at this moment
	button.attachClick(singleClick);
	button.attachLongPressStop(longPress);
	sensor.calibrate();
	movement.openingDuration = 24200;
	movement.closingDuration = 21500;
	position.percent = 50;
	Serial.println("*******************");
	calibrate();
}

void calibrate()
{
	Serial.println("Calibration");
	digitalWrite(PIN_OPEN, LOW);
	digitalWrite(PIN_CLOSE, LOW);
	float val0 = sensor.getCurrentAC();
	val0 = max(val0, sensor.getCurrentAC());
	digitalWrite(PIN_OPEN, HIGH);
	delay(800);
	float valOpening = sensor.getCurrentAC();
	valOpening = max(valOpening, sensor.getCurrentAC());
	delay(200);
	digitalWrite(PIN_OPEN, LOW);
	delay(1000);
	digitalWrite(PIN_CLOSE, HIGH);
	delay(800);
	float valClosing = sensor.getCurrentAC();
	valClosing = max(valClosing, sensor.getCurrentAC());
	delay(200);
	digitalWrite(PIN_CLOSE, LOW);
	delay(500);
	float val1 = sensor.getCurrentAC();
	val1 = max(val1, sensor.getCurrentAC());
	consumptionThreshold = min(valOpening, valClosing) * thresholdFactor;
	Serial.print("  valOpening = ");
	Serial.println(valOpening);
	Serial.print("  valClosing = ");
	Serial.println(valClosing);
	Serial.print("  val0 = ");
	Serial.println(val0);
	Serial.print("  val1 = ");
	Serial.println(val1);
	Serial.print("  consumptionThreshold = ");
	Serial.println(consumptionThreshold);
}

void longPress()
{
	setTarget(20);
}

void setTarget(int pos)
{
	if (pos == position.percent)
		return;
	target.position = pos;
	target.enabled = true;
	if (pos < position.percent)
	{
		movement.direction = directionOpening;
		Serial.print("Target -> opening to ");
		Serial.print(pos);
		Serial.println("%");
	}
	else
	{
		movement.direction = directionClosing;
		Serial.print("Target -> closing to ");
		Serial.print(pos);
		Serial.println("%");
	}
	setPinsForMovement();
}

void singleClick()
{
	Serial.println("Click");
	if (movement.direction == directionNone)
	{
		movement.direction = (lastDirectionWasOpening ? directionClosing : directionOpening);
	}
	else
	{
		movement.direction = directionNone;
	}
	setPinsForMovement();
}

void setPinsForMovement()
{
	switch (movement.direction)
	{
	case directionNone:
		digitalWrite(PIN_OPEN, LOW);
		digitalWrite(PIN_CLOSE, LOW);
		Serial.println("Mvt : stop");
		target.enabled = false;
		movement.measureClosingDuration = false;
		movement.measureOpeningDuration = false;
		break;
	case directionOpening:
		movement.measureOpeningDuration = (position.status == positionClosed);
		Serial.print("measureOpeningDuration = ");
		Serial.println(movement.measureOpeningDuration);
		digitalWrite(PIN_CLOSE, LOW);
		digitalWrite(PIN_OPEN, HIGH);
		Serial.println("Mvt : open");
		movement.startTime = millis();
		movement.startPos = position.percent;
		lastDirectionWasOpening = true;
		break;
	case directionClosing:
		movement.measureClosingDuration = (position.status == positionOpen);
		Serial.print("measureClosingDuration = ");
		Serial.println(movement.measureClosingDuration);
		digitalWrite(PIN_OPEN, LOW);
		digitalWrite(PIN_CLOSE, HIGH);
		Serial.println("Mvt : close");
		movement.startTime = millis();
		movement.startPos = position.percent;
		lastDirectionWasOpening = false;
		break;
	}
	// Make sure that the ACS712 will not be requested before the motor
	// consumes power
	delay(200);
}

void handleMvt()
{
	if (movement.direction == directionNone)
		return;
	float valCur = sensor.getCurrentAC();
	// Serial.println(valCur);
	if (valCur < consumptionThreshold)
	{
		// The motor of the shutter does not consumes any power : then shutter is
		// either fully closed or fully open
		if (movement.direction == directionOpening)
		{
			// The shutter is fully open
			position.status = positionOpen;
			position.percent = 0;
			if (movement.measureOpeningDuration)
			{
				movement.openingDuration = millis() - movement.startTime;
				Serial.print("openingDuration = ");
				Serial.println(movement.openingDuration);
			}
		}
		else
		{
			// The shutter is fully closed
			position.status = positionClosed;
			position.percent = 100;
			if (movement.measureClosingDuration)
			{
				movement.closingDuration = millis() - movement.startTime;
				Serial.print("closingDuration = ");
				Serial.println(movement.closingDuration);
			}
		}
		movement.direction = directionNone;
		Serial.print("Stopped : ");
		Serial.println(valCur);
		setPinsForMovement();
	}

	if ((movement.direction == directionOpening) && (movement.openingDuration > 0))
	{
		// compute the new position
		long mvtDuration = millis() - movement.startTime;
		float posDelta = ((float)mvtDuration / movement.openingDuration) * 100;
		position.percent = movement.startPos - posDelta;
		if (position.percent < 0)
			position.percent = 0;
	}
	else if ((movement.direction == directionClosing) && (movement.closingDuration > 0))
	{
		// compute the new position
		long mvtDuration = millis() - movement.startTime;
		float posDelta = ((float)mvtDuration / movement.closingDuration) * 100;
		position.percent = movement.startPos + posDelta;
		if (position.percent > 100)
			position.percent = 100;
	}

	if (movement.direction != directionNone)
	{
		int posToDisplay = (int)position.percent;
		if (position.lastDisplayedPercent != posToDisplay)
		{
			Serial.print("Pos = ");
			Serial.print(posToDisplay);
			if (target.enabled) {
				Serial.print(" / ");
				Serial.print(target.position);
			}
			Serial.println();
			position.lastDisplayedPercent = posToDisplay;
		}
	}

	if (target.enabled)
	{
		if ((movement.direction == directionClosing) && (position.percent >= target.position))
		{
			// The target position has been reached
			movement.direction = directionNone;
			Serial.println("Target reached");
			setPinsForMovement();
		}
		else if ((movement.direction == directionOpening) && (position.percent <= target.position))
		{
			// The target position has been reached
			movement.direction = directionNone;
			Serial.println("Target reached");
			setPinsForMovement();
		}
	}
}

void loop()
{
	button.tick();
	handleMvt();
	delay(10);
}
