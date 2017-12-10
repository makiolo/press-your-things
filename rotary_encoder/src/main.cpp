#include <Arduino.h>

#define FW_NAME "rotary_encoder"
#define FW_VERSION "1.0.0"
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";

int pinCLK = D7;
int pinDT = D6;
int pinBUTTON = D5;
int encoderPosCount = 0;
int pinALast;  
int aVal;
boolean bCW;

class pusher
{
public:
	explicit pusher()
	{
		;
	}

	void begin()
	{
		pinMode (D3, OUTPUT);
		pinMode (D4, OUTPUT);
		stop();
	}

	void turn_left()
	{
		digitalWrite(D3, HIGH);
		digitalWrite(D4, LOW);
	}

	void turn_right()
	{
		digitalWrite(D3, LOW);
		digitalWrite(D4, HIGH);
	}

	void stop()
	{
		digitalWrite(D3, LOW);
		digitalWrite(D4, LOW);
	}

	void push_button()
	{
		turn_left();
		delay(300);
		turn_right();
		delay(600);
		turn_left();
		delay(50);
		stop();
	}
};

pusher p;

void setup()
{
	p.begin();
	pinMode (pinCLK, INPUT);
	pinMode (pinDT, INPUT);
	pinMode (pinBUTTON, INPUT_PULLUP);
	pinALast = digitalRead(pinCLK);
	Serial.begin(9600);
	Serial.println("BEGIN");
	Serial.println();
}

void loop() 
{ 
	if(digitalRead(pinBUTTON) == LOW)
	{
		Serial.println ("Reset to 0");
		encoderPosCount = 0;
		p.push_button();
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

