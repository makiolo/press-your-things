#include <Arduino.h>
int pinCLK = D7;
int pinDT = D6;
int pinBUTTON = D5;
int encoderPosCount = 0;
int pinALast;  
int aVal;
boolean bCW;

void setup()
{
	pinMode (pinCLK, INPUT);
	pinMode (pinDT, INPUT);
	pinMode (pinBUTTON, INPUT_PULLUP);
	pinALast = digitalRead(pinCLK);//Read Pin A 
	Serial.begin(9600);
	Serial.println("BEGIN");
	Serial.println();
}

void loop() 
{ 
	if( digitalRead(pinBUTTON) == LOW)
	{
		Serial.println ("Reset to 0");
		encoderPosCount = 0;
	}
	// 
	aVal = digitalRead(pinCLK);
	if (aVal != pinALast)
	{ 
		if (digitalRead(pinDT) != aVal) //We're Rotating Clockwise
		{ 
			encoderPosCount ++;
			bCW = true;
		} 
		else 
		{
			bCW = false;
			encoderPosCount--;
		}

		if (bCW)
		{
			Serial.println ("Rotate Clockwise");
		}
		else
		{
			Serial.println("Rotate Counterclockwise");
		}

		Serial.print("Encoder Count: ");
		Serial.println(encoderPosCount);
		Serial.println();
	} 

	pinALast = aVal;
} 

