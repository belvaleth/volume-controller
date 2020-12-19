#include <Encoder.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <vector>

#define ENC_MASTER_A 0
#define ENC_MASTER_B 1
#define ENC_MASTER_SW 2
#define ENC_MASTER_RED 16
#define ENC_MASTER_GREEN 17
#define ENC_MASTER_BLUE 18

#define ENC_CURRENT_A 3
#define ENC_CURRENT_B 4
#define ENC_CURRENT_SW 5
#define ENC_CURRENT_RED 19
#define ENC_CURRENT_GREEN 20
#define ENC_CURRENT_BLUE 21

#define ENC_DISCORD_A 6
#define ENC_DISCORD_B 7
#define ENC_DISCORD_SW 8
#define ENC_DISCORD_RED 22
#define ENC_DISCORD_GREEN 23
#define ENC_DISCORD_BLUE 24

#define ENC_GAME_A 9
#define ENC_GAME_B 10
#define ENC_GAME_SW 11
#define ENC_GAME_RED 22
#define ENC_GAME_GREEN 23
#define ENC_GAME_BLUE 24

#define ENC_MUSIC_A 14
#define ENC_MUSIC_B 15
#define ENC_MUSIC_SW 16
#define ENC_MUSIC_RED 22
#define ENC_MUSIC_GREEN 23
#define ENC_MUSIC_BLUE 24

#define ENC_FIREFOX_A 17
#define ENC_FIREFOX_B 18
#define ENC_FIREFOX_SW 19
#define ENC_FIREFOX_RED 22
#define ENC_FIREFOX_GREEN 23
#define ENC_FIREFOX_BLUE 24

bool masterMute = false;
bool currentMute = false;
bool discordMute = false;
bool gameMute = false;
bool musicMute = false;
bool firefoxMute = false;

int masterVol, currentVol, discordVol, gameVol, musicVol, firefoxVol;

std::vector<bool*> muteStatus = { &masterMute, &currentMute, &discordMute, &gameMute, &musicMute, &firefoxMute };

// Number of encoders

const int NUM_ENCODERS = 2;

// Create encoders

Encoder encMaster(ENC_MASTER_A,ENC_MASTER_B);
Encoder encCurrent(ENC_CURRENT_A,ENC_CURRENT_B);
//Encoder encDiscord(ENC_DISCORD_A,ENC_DISCORD_B);
//Encoder encGame(ENC_GAME_A,ENC_GAME_B);
//Encoder encMusic(ENC_MUSIC_A,ENC_MUSIC_B);
//Encoder encFirefox(ENC_FIREFOX_A,ENC_FIREFOX_B);

std::vector<Encoder*> encoders = { &encMaster, &encCurrent };

class LED
{

private:
	int redPin, greenPin, bluePin;
	unsigned char Colour;
	unsigned char previousColour;

public:

	enum Colours : unsigned char
	{
		LEDOFF = B111,
		RED = B110,
		GREEN = B101,
		YELLOW = B100,
		BLUE = B011,
		PURPLE = B010,
		CYAN = B001,
		WHITE = B000
	};

	LED(int RedPin, int GreenPin, int BluePin, unsigned char colour)
	{
		redPin = RedPin;
		bluePin = BluePin;
		greenPin = GreenPin;
		pinMode(redPin, OUTPUT);
		pinMode(greenPin, OUTPUT);
		pinMode(bluePin, OUTPUT);
		this->Colour = colour;
		this->setColour(Colour);
		
	}

	void setColour(unsigned char newColour)
	{
		digitalWrite(redPin, newColour & B001);
		digitalWrite(greenPin, newColour & B010);
		digitalWrite(bluePin, newColour & B100);

		Colour = newColour;
	}
	void muteToggle()
	{
		if (Colour == RED)
		{
			this->setColour(previousColour);
		}
		else
		{
			previousColour = Colour;
			this->setColour(RED);
		}
	}

	~LED()
	{
		this->setColour(LEDOFF);
	}

};

LED masterLED(ENC_MASTER_RED, ENC_MASTER_GREEN, ENC_MASTER_BLUE, LED::WHITE);
LED currentLED(ENC_CURRENT_RED, ENC_CURRENT_GREEN, ENC_CURRENT_BLUE, LED::CYAN);
//LED discordLED(ENC_DISCORD_RED, ENC_DISCORD_GREEN, ENC_DISCORD_BLUE, LED::PURPLE);
//LED gameLED(ENC_GAME_RED, ENC_GAME_GREEN, ENC_GAME_BLUE, LED::BLUE);
//LED musicLED(ENC_MUSIC_RED, ENC_MUSIC_GREEN, ENC_MUSIC_BLUE, LED::GREEN);
//LED firefoxLED(ENC_FIREFOX_RED, ENC_FIREFOX_GREEN, ENC_FIREFOX_BLUE, LED::YELLOW);


void setup()
{
	pinMode(ENC_MASTER_SW, INPUT);
	pinMode(ENC_CURRENT_SW, INPUT);
	//pinMode(ENC_DISCORD_SW, INPUT);
	//pinMode(ENC_GAME_SW, INPUT);
	//pinMode(ENC_MUSIC_SW, INPUT);
	//pinMode(ENC_FIREFOX_SW, INPUT);

	attachInterrupt(digitalPinToInterrupt(ENC_MASTER_SW), muteMaster, RISING);
	attachInterrupt(digitalPinToInterrupt(ENC_CURRENT_SW), muteCurrent, RISING);
	//attachInterrupt(digitalPinToInterrupt(ENC_DISCORD_SW), muteDiscord, RISING);
	//attachInterrupt(digitalPinToInterrupt(ENC_GAME_SW), muteGame, RISING);
	//attachInterrupt(digitalPinToInterrupt(ENC_MUSIC_SW), muteMusic, RISING);
	//attachInterrupt(digitalPinToInterrupt(ENC_FIREFOX_SW), muteFirefox, RISING);

	Serial.begin(9600);

}


void loop()
{
	String sendString = String("");
	int reading = 0;
	for (int i = 0; i < encoders.size(); i++)
	{
		if (*muteStatus[i])
		{
			reading = 0;
		}
		else
		{
			reading = (encoders[i])->read();
			if (reading > 102)
			{
				reading = 102;
				(encoders[i])->write(reading);
			}
			if (reading < 0)
			{
				reading = 0;
				(encoders[i])->write(reading);
			}
			else
			{
				reading = (reading * 10);
			}
		}
		sendString += (String)reading;
		if (i < (encoders.size() - 1))
		{
			sendString += String("|");
		}
	}
	Serial.println(sendString);
	delay(10);
}

//button interrupt handlers
void muteMaster()
{
	if (!masterMute)
	{
		masterVol = encMaster.read();
	}
	else
	{
		encMaster.write(masterVol);
	}
	masterMute = !masterMute;
	masterLED.muteToggle();
}
void muteCurrent()
{
	if (!currentMute)
	{
		currentVol = encCurrent.read();
	}
	else
	{
		encCurrent.write(currentVol);
	}
	currentMute = !currentMute;
	currentLED.muteToggle();
}

/*
void muteDiscord()
{
	discordMute = !discordMute;
	discordLED.muteToggle();
}
void muteGame()
{
	gameMute = !gameMute;
	gameLED.muteToggle();
}
void muteMusic()
{
	musicMute = !musicMute;
	musicLED.muteToggle();
}
void muteFirefox()
{
	firefoxMute = !firefoxMute;
	firefoxLED.muteToggle();
}
*/