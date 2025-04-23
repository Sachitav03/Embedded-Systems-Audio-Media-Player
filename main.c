/*
Computer Science 145: Embedded Systems
-   Project 3
    Ananya Karra, Sachita Rayapati
*/

#include <stdio.h>
#include "avr.h"
#include "lcd.h"

// Frequencies for notes on Octave 3
#define A 220
#define As 233.08
#define B 246.94
#define C 130.81
#define C1 277.18
#define D 146.83
#define Ds 155.56
#define E 164.81
#define F 174.61
#define Fs 185
#define G 196
#define Gs 207.65

const char keypad[4][4] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'},
};

// Checks if key is pressed on the Keypad
// ----ALL 8 GPIOs TO N/C---
int is_pressed(int r, int c) {
	DDRC = 0x00;
	PORTC = 0x00;

	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	CLR_BIT(DDRC, c+4);
	SET_BIT(PORTC, c+4);
	avr_wait(10);
	if (GET_BIT(PINC, c+4)==0){
		return 1;
	}
	return 0;
}

// If key was pressed, return the key value from the keypad.
char get_key() {
	int i, j;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			if(is_pressed(i,j)){
				avr_wait(50);
				return keypad[i][j];
			}
		}
		avr_wait(10);
	}
	return '$';
}

// Initial output on display screen
void print_first() {
	char buf[17];
	char buff[17];
	lcd_pos(0, 0);
	sprintf(buf, "Project 3");
	
	lcd_puts2(buf);
	lcd_pos(1, 0);
	sprintf(buff, "Music Notes");
	lcd_puts2(buff);
}

typedef struct {
	double frequency;
	double duration;
} MusicNote;

#define Wait2 2.0
#define Wait1 1.0
#define Wait2_5 2.5
#define Wait4 4.0

// Notes for Happy Birthday {note_frequency, duration}
MusicNote Happy_Birthday[] = {
	{C,Wait2}, {C,Wait2}, {D,Wait2}, {C,Wait2}, {F,Wait2}, {E,Wait2_5},
	{C,Wait2}, {C,Wait2}, {D,Wait2}, {C,Wait2}, {G,Wait2}, {F,Wait2_5},
	{C,Wait2}, {C,Wait2}, {C1,Wait2}, {A,Wait2}, {F,Wait2}, {E,Wait2}, {D,Wait2_5},
	{As,Wait2}, {As,Wait2}, {A,Wait2}, {F,Wait2}, {G,Wait2}, {F,Wait2_5}
};

// Notes for Twinkle Star {note_frequency, duration}
MusicNote Twinkle_Star[] = {
	{C,Wait2}, {C,Wait2}, {G,Wait2}, {G,Wait2}, {A,Wait2}, {A,Wait2}, {G,Wait2_5},
	{F,Wait2}, {F,Wait2}, {E,Wait2}, {E,Wait2}, {D,Wait2}, {D,Wait2}, {C,Wait2_5},
	{G,Wait2}, {G,Wait2}, {F,Wait2}, {F,Wait2}, {E,Wait2}, {E,Wait2}, {D,Wait2_5},
	{G,Wait2}, {G,Wait2}, {F,Wait2}, {F,Wait2}, {E,Wait2}, {E,Wait2}, {D,Wait2_5},
	{C,Wait2}, {C,Wait2}, {G,Wait2}, {G,Wait2}, {A,Wait2}, {A,Wait2}, {G,Wait2_5},
	{F,Wait2}, {F,Wait2}, {E,Wait2}, {E,Wait2}, {D,Wait2}, {D,Wait2}, {C,Wait2_5}
};

// Notes for Happy and You Know It {note_frequency, duration}
MusicNote Happy_Clap[] = {
	{D,Wait2}, {D,Wait2}, {G,Wait1}, {G,Wait1}, {G,Wait2}, {G,Wait2}, {G,Wait2}, {G,Wait2_5}, {Fs,Wait2}, {G,Wait2}, {A,Wait4},
	{D,Wait2}, {D,Wait2}, {A,Wait1}, {A,Wait1}, {A,Wait2}, {A,Wait2}, {A,Wait2}, {A,Wait2_5}, {G,Wait2}, {A,Wait2}, {B,Wait4},
	{B,Wait2}, {B,Wait2}, {C,Wait1}, {C,Wait1}, {C,Wait2}, {C,Wait2}, {C,Wait2}, {E,Wait2_5}, {C,Wait2}, {C,Wait2}, {B,Wait2}, {B,Wait2}, {B,Wait2}, {B,Wait1}, {A,Wait1}, {G,Wait2}, {G,Wait4},
	{B,Wait2}, {B,Wait2}, {A,Wait1}, {A,Wait1}, {A,Wait2}, {G,Wait2}, {Fs,Wait2}, {Fs,Wait2_5}, {E,Wait2}, {Fs,Wait2}, {G,Wait4},
};

// Notes for Jurassic Theme {note_frequency, duration}
MusicNote Jurassic[] = {
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5}, 
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5}, 
	{As,Wait2}, {A,Wait2}, {A,Wait2_5}, {As,Wait2}, {F,Wait2}, {As,Wait2}, {Gs,Wait2_5},
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2_5}, 
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {F,Wait2}, {Ds,Wait2}, 
	{As,Wait2}, {A,Wait2}, {As,Wait2}, {Ds,Wait2}, {D,Wait2}, {Ds,Wait2_5}, 
	{As,Wait2}, {A,Wait2}, {As,Wait2_5}, {A,Wait2}, {As,Wait2_5}
};

/*
From AVR_C with updated frequency wait period
for better and precise sound quality.
*/
void avr_wait_main(unsigned short msec) {
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.00001);
		SET_BIT(TIFR, TOV0);
		WDR();
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}

// Default setting of volume to 1 (Switched ON)
int volume_button = 1;

void playing_note(const MusicNote* play_note) {
	char key = get_key();
	if(key == '*'){
		// To stop song at any time
		volume_button = 0;
	}
	if(key == '#'){
		// To start song at any time
		volume_button = 1;
	}
	if(volume_button && key != '*'){
		double freq = play_note->frequency;
		// To increase pitch
		if(key == '1'){
			freq += 100.0;
		}
		if(key == '2'){
			freq += 200.0;
		}
		if(freq == '3'){
			freq += 300.0;
		}
		double time = play_note->duration;
		// To increase tempo
		if(key == '4'){
			time += 1.0;
		}
		if(key == '5'){
			time += 2.0;
		}
		if(key == '6'){
			time += 3.0;
		}
		double load = 1/freq;
		int comp = (load/2) * 10000; // convert to ms
		int note = time / load;
		for(int i = 0; i < note; i++){
			// Sets third bit of PORTB to 1
			SET_BIT(PORTB, 3); // Switch Speaker ON
			avr_wait_main(comp); // Time is High
			// Clears third bit of PORTB to 0
			CLR_BIT(PORTB, 3); // Switch Speaker OFF
			avr_wait_main(comp); // Time if Low
		}
	}
}

/*
Loops through every note in the song's array.
If key A, B, C, or D are selected, the song
changes immediately to the chosen one.
*/
void song_function(char key);
void song(const MusicNote music[], int notes) {
	if(volume_button){
		for(int i = 0; i < notes; i++){
			char key = get_key();
			if (key == 'A' || key == 'B' || key == 'C' || key == 'D'){
				song_function(key);
				break;
			}
			playing_note(&music[i]);
		}
	}
}

/*
If a particular song is selected, the name of
the song is printed on the LCD display.
*/
void print_song(char key) {
	char buf1[17];
	lcd_pos(0,0);
	sprintf(buf1, "Song Playing:");
	lcd_puts2(buf1);
	char buf2[17];
	lcd_pos(1,0);
	switch(key){
		case 'A':
		sprintf(buf2, "Happy Birthday!");
		lcd_puts2(buf2);
		break;
		
		case 'B':
		sprintf(buf2, "Twinkle Star!");
		lcd_puts2(buf2);
		break;
		
		case 'C':
		sprintf(buf2, "Happy Clap!");
		lcd_puts2(buf2);
		break;
		
		case 'D':
		sprintf(buf2, "Jurassic!");
		lcd_puts2(buf2);
		break;
	}
}

/*
The song_function checks the key and if valid, then
it calls song(song_array[], length) to play the song
that the key represents.
*/
void song_function(char key) {
	int HB_Length;
	switch(key){
		case 'A':
		print_song('A');
		HB_Length = sizeof(Happy_Birthday)/sizeof(Happy_Birthday[0]);
		song(Happy_Birthday, HB_Length);
		lcd_clr();
		break;
		
		case 'B':
		print_song('B');
		HB_Length = sizeof(Twinkle_Star)/sizeof(Twinkle_Star[0]);
		song(Twinkle_Star, HB_Length);
		lcd_clr();
		break;
		
		case 'C':
		print_song('C');
		HB_Length = sizeof(Happy_Clap)/sizeof(Happy_Clap[0]);
		song(Happy_Clap, HB_Length);
		lcd_clr();
		break;
		
		case 'D':
		print_song('D');
		HB_Length = sizeof(Jurassic)/sizeof(Jurassic[0]);
		song(Jurassic, HB_Length);
		lcd_clr();
		break;
	}
}

int main() {
	avr_init();
	lcd_init();
	print_first();
	avr_wait(500);
	lcd_clr();
	SET_BIT(DDRB, 3);
	while(1){
		avr_wait(1000);
		char key = get_key();
		song_function(key);
	}
	return 0;
}