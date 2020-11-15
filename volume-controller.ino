#include <Encoder.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <vector>
#include <bitset>
#include <WS2812Serial.h>

extern "C" {
	__attribute__((weak)) int __exidx_start() { return -1; }
	__attribute__((weak)) int __exidx_end() { return -1; }
}

constexpr int latchPin = 21;
constexpr int clockPin = 19;
constexpr int dataPin = 18;

constexpr int enc_Master_A = 0;
constexpr int enc_Master_B = 1;
constexpr int enc_Master_SW = 2;
constexpr int enc_Master_Red = 0;
constexpr int enc_Master_Green = 1;
constexpr int enc_Master_Blue = 2;


constexpr int enc_Current_A = 3;
constexpr int enc_Current_B = 4;
constexpr int enc_Current_SW = 5;
constexpr int enc_Current_Red = 8;
constexpr int enc_Current_Green = 9;
constexpr int enc_Current_Blue = 10;

/*
constexpr int enc_Discord_A = 6;
constexpr int enc_Discord_B = 7;
constexpr int enc_Discord_SW = 8;
constexpr int enc_Discord_Red = 6;
constexpr int enc_Discord_Green = 7;
constexpr int enc_Current_Blue = 8;

constexpr int enc_Game_A = 9;
constexpr int enc_Game_B = 10;
constexpr int enc_Game_SW = 11;
constexpr int enc_Game_Red = 9;
constexpr int enc_Game_Green = 10;
constexpr int enc_Game_Blue = 11;

constexpr int enc_Music_A = 12;
constexpr int enc_Music_B = 13;
constexpr int enc_Music_SW = 14;
constexpr int enc_Music_Red = 12;
constexpr int enc_Music_Green = 13;
constexpr int enc_Music_Blue = 14;

constexpr int enc_Firefox_A = 15;
constexpr int enc_Firefox_B = 16;
constexpr int enc_Firefox_SW = 17;
constexpr int enc_Firefox_Red = 15;
constexpr int enc_Firefox_Green = 16;
constexpr int enc_Firefox_Blue = 17;
*/


bool masterMute = false;
bool currentMute = false;
bool discordMute = false;
bool gameMute = false;
bool musicMute = false;
bool firefoxMute = false;

int masterVol, currentVol, discordVol, gameVol, musicVol, firefoxVol;

std::vector<bool*> muteStatus = { &masterMute, &currentMute, &discordMute, &gameMute, &musicMute, &firefoxMute };

std::bitset<16> LEDStream;

// Create encoders

Encoder encMaster(enc_Master_A,enc_Master_B);
Encoder encCurrent(enc_Current_A,enc_Current_B);
/*
Encoder encDiscord(enc_Discord_A,enc_Discord_B);
Encoder encGame(enc_Game_A,enc_Game_B);
Encoder encMusic(enc_Music_A,enc_Music_B);
Encoder encFirefox(enc_Firefox_A,enc_Firefox_B);
*/

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
		RED = B011,
		GREEN = B101,
		YELLOW = B001,
		BLUE = B110,
		PURPLE = B010,
		CYAN = B100,
		WHITE = B000
	};

	LED(int RedPin, int GreenPin, int BluePin, unsigned char colour)
	{
		redPin = RedPin;
		bluePin = BluePin;
		greenPin = GreenPin;
		this->Colour = colour;
		this->setColour(Colour);
	}

	void setColour(unsigned char newColour)
	{
		Colour = newColour;
		bool redOut = (Colour >> 2) & 0x1;
		bool greenOut = (Colour >> 1) & 0x1;
		bool blueOut = Colour & 0x1;
		LEDStream.set(redPin, redOut);
		LEDStream.set(greenPin, greenOut);
		LEDStream.set(bluePin, blueOut);
		updateShiftRegister();
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

LED masterLED(enc_Master_Red, enc_Master_Green, enc_Master_Blue, LED::WHITE);
LED currentLED(enc_Current_Red, enc_Current_Green, enc_Current_Blue, LED::CYAN);
//LED discordLED(ENC_DISCORD_RED, ENC_DISCORD_GREEN, ENC_DISCORD_BLUE, LED::PURPLE);
//LED gameLED(ENC_GAME_RED, ENC_GAME_GREEN, ENC_GAME_BLUE, LED::BLUE);
//LED musicLED(ENC_MUSIC_RED, ENC_MUSIC_GREEN, ENC_MUSIC_BLUE, LED::GREEN);
//LED firefoxLED(ENC_FIREFOX_RED, ENC_FIREFOX_GREEN, ENC_FIREFOX_BLUE, LED::YELLOW);

const int numled = 32;
const int pin = 20;

byte drawingMemory[numled * 3];
DMAMEM byte displayMemory[numled * 12];

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB);

constexpr int RING_COLOURS[16] = { 0x001600, 0x021600, 0x041600, 0x061500, 0x081500, 0x101400, 0x101200, 0x101000, 0x100800, 0x100600, 0x100400, 0x110400, 0x120300, 0x130200, 0x140100, 0x160000 };

class Timer
{

private:
	unsigned long StartTime;

public:
	unsigned long Duration = 0;

	Timer(unsigned long duration)
	{
		StartTime = millis();
		Duration = duration;
	}

	bool isExpired()
	{
		if (millis() - StartTime >= Duration)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void restart()
	{
		StartTime = millis();
	}

};

Timer masterTimer(50);
Timer currentTimer(50);
Timer discordTimer(50);
Timer gameTimer(50);
Timer musicTimer(50);
Timer firefoxTimer(50);


void setup()
{
	pinMode(latchPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, OUTPUT);


	pinMode(enc_Master_SW, INPUT);
	pinMode(enc_Current_SW, INPUT);
	//pinMode(ENC_DISCORD_SW, INPUT);
	//pinMode(ENC_GAME_SW, INPUT);
	//pinMode(ENC_MUSIC_SW, INPUT);
	//pinMode(ENC_FIREFOX_SW, INPUT);

	attachInterrupt(digitalPinToInterrupt(enc_Master_SW), muteMaster, RISING);
	attachInterrupt(digitalPinToInterrupt(enc_Current_SW), muteCurrent, RISING);
	//attachInterrupt(digitalPinToInterrupt(ENC_DISCORD_SW), muteDiscord, RISING);
	//attachInterrupt(digitalPinToInterrupt(ENC_GAME_SW), muteGame, RISING);
	//attachInterrupt(digitalPinToInterrupt(ENC_MUSIC_SW), muteMusic, RISING);
	//attachInterrupt(digitalPinToInterrupt(ENC_FIREFOX_SW), muteFirefox, RISING);

	leds.begin();

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
		updateWS2812(reading, (i*16));
		sendString += (String)reading;
		if (i < (encoders.size() - 1))
		{
			sendString += String("|");
		}
	}
	Serial.println(sendString);
	delay(10);
}

void updateWS2812(int volume, int offset)
{
	for (int i = 0; i < 16; i++)
	{
		leds.setPixel(i+offset, 0x000000);
	}
	int activeLEDs = volume / 60;
	if (activeLEDs > 16)
	{
		activeLEDs = 16;
	}
	for (int i = 0; i < activeLEDs; i++)
	{
		leds.setPixel(i+offset, RING_COLOURS[i]);
	}
	leds.show();
}


void updateShiftRegister()
{
	digitalWrite(latchPin, LOW);
	unsigned char byte1 = ((LEDStream.to_ulong()) & 0xFF000000UL) >> 24;
	unsigned char byte2 = ((LEDStream.to_ulong()) & 0x00FF0000UL) >> 16;
	unsigned char byte3 = ((LEDStream.to_ulong()) & 0x0000FF00UL) >> 8;
	unsigned char byte4 = ((LEDStream.to_ulong()) & 0x000000FFUL);

	shiftOut(dataPin, clockPin, MSBFIRST, byte3);
	shiftOut(dataPin, clockPin, MSBFIRST, byte4);
	digitalWrite(latchPin, HIGH);
}

//button interrupt handlers

void muteMaster()
{
	if (masterTimer.isExpired() == true)
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
		masterTimer.restart();
	}
}

void muteCurrent()
{
	if (currentTimer.isExpired() == true)
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
		currentTimer.restart();
	}
}
/*
void muteDiscord()
{
	if (!discordMute)
	{
		discordVol = encDiscord.read();
	}
	else
	{
		encdiscord.write(discordVol);
	}
	discordMute = !discordMute;
	discordLED.muteToggle();
}
void muteGame()
{
	if (!gameMute)
	{
		gameVol = encGame.read();
	}
	else
	{
		encGame.write(gameVol);
	}
	gameMute = !gameMute;
	gameLED.muteToggle();
}
void muteMusic()
{
	if (!musicMute)
	{
		musicVol = encMusic.read();
	}
	else
	{
		encMusic.write(musicVol);
	}
	musicMute = !musicMute;
	musicLED.muteToggle();
}
void muteFirefox()
{
	if (!firefoxMute)
	{
		firefoxVol = encFirefox.read();
	}
	else
	{
		encFirefox.write(firefoxVol);
	}
	firefoxMute = !firefoxMute;
	firefoxLED.muteToggle();
}
*/