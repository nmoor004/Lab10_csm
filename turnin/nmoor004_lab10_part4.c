/*	Author: nmoor004
 *  Partner(s) Name: 
 *	Lab Section: 
 *	Assignment: Lab # 10 Exercise # 4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

////////////////////////////////////////////////////
////////////////TIMER FUNCTIONS
//////////////////////////////////////////////////

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C Programmer should clear to 0.
unsigned long _avr_timer_M = 1;	       // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
unsigned short count = 0x00;
void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;     // bit3 = 0: CTC mode (clear timer on compare)
		          // bit2bit1bit0 = 011: pre-scaler /64
			 // 00001011: 0x0B
			// SO, 8 MHz clock or 8,000,000 / 64 = 125,000 ticks/s
		       // Thus, TCNT1 register will count at 125,000 tick/s
	// AVR output compare register OCR1A.
	OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
		       // We want a 1 ms tick. 0.001s * 125,000 ticks/s = 125
		      // So when TCNT1 register equals 125,
		     // 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt
	// Initialize avr counter
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	// Enable globla interrupts
	SREG |= 0x80; // 0x80: 1000000
}
void TimerOff() {
	TCCR1B = 0x00; /// bit3bit1bit0 = 000: timer off
}
void TimerISR() {
	TimerFlag = 1;
}
// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); 	      // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}



//GLOBAL SHARED VARIABLE
unsigned char threeLEDs = 0x00;

/////////////////////////////////////////
/////////////globalFreqSM//////
/////////////////////////////////////////

enum F_States {f_init, f_around, f_up, f_down} f_state;
unsigned short frequency;
unsigned char PA_0;
unsigned char PA_1;

void f_uck() {
	PA_0 = (PINA & 0x01);
	PA_1 = (PINA & 0x02) >> 1;
	switch(f_state) {
		case f_init:
			frequency = 0x00;
			f_state = f_around;
		break;
		case f_around:
			if (!PA_0 && PA_1) {
				f_state = f_up;
			}
			else if (PA_0 && !PA_1) {
				f_state = f_down;
			}
		break;
		case f_up:
			if (frequency < 500) {
				frequency++;			
			}
			f_state = f_around;
		break;
		case f_down:
			if (frequency > 1) {
				frequency--;
			}
			f_state = f_around;
		break;
	}	

	switch(f_state) {
		case f_init:
		break;
		case f_around:
		break;
		case f_up:
		break;
		case f_down:
		break;
	}	
}



///////////////////////////////////////
////////////ThreeLEDsSM//////////////
///////////////////////////////////////

enum T_States {t_init, t_Shift} t_state;

unsigned char shifter;
unsigned char t_count;

void t_tick() {
	switch(t_state) {
		case t_init:
			shifter = 0x04;
			//PORTB = shifter;
			t_state = t_Shift;
			t_count = 0x00;
			break;
		case t_Shift: 
			if (t_count != 300) {
				t_count++;
			}
			else {
				if (shifter < 0x04) {
					shifter = shifter << 1;
				}
				else {
					shifter = 0x01;
				}
				t_count = 0;
			}

			//PORTB = shifter;
			break;
	}
	
	switch(t_state) {
		case t_init:
			break;
		case t_Shift: 
			break;
	}
	
	
}



////////////////////////////////////////////////
//////////////BlinkingLEDSM//////////////////////
//////////////////////////////////////////////

enum B_States {b_init, b_On, b_Off} b_state;

unsigned char B3;
unsigned short b_count = 0x00;

void b_tick() {
	switch(b_state) {
		case b_init:
			B3 = 0x00;
			b_state = b_On;
			break;
		case b_On:

			PORTC = 0x01;
			B3 = B3 | 0x08;
			if (b_count != 1000) {
				b_count++;
			}
			else {
				b_count = 0;
				b_state = b_Off;
			}

			break;
		case b_Off:
			PORTC = 0x02;
			B3 = B3 & 0x07;
			if (b_count != 1000) {
				b_count++;
			}
			else {
				b_count = 0;
				b_state = b_On;
			}
			break;
	}
	
	switch(b_state) {
		case b_init:
			break;
		case b_On:
			break;
		case b_Off:
			break;
	}
	
	
}




/////////////////////////////////////////////
///////////SoundSM/////////////////////
////////////////////////////////////////////

enum P_States {p_init, p_Idle, p_Toggle_On, p_Toggle_Off} p_state;
unsigned char p_count;
unsigned char p_Toggle;
unsigned char PA_2;

void p_tick() {
	PA_2 = (PINA & 0x04) >> 2;
	switch(p_state) {
		case p_init:
			p_Toggle = 0x00;
			p_count = 0x00;
			p_state = p_Toggle_Off;
			break;
		case p_Idle:
			if (!PA_2) {
				p_state = p_Toggle_On;
			}
			else {
				p_state = p_Idle;
			}

			break;
		case p_Toggle_On:
			if (!PA_2) { //Buttons are inverted for some reason
				if (p_count != frequency) {
					p_count++;
				}
				else {
					p_Toggle = 0x10;
					p_state = p_Toggle_Off;
					p_count = 0;
				}
			}
			break;
			
		case p_Toggle_Off:
			if (!PA_2) { //Buttons are inverted for some reason
				if (p_count != frequency) {
					p_count++;
				}
				else {
					p_Toggle = 0x00;
					p_state = p_Toggle_On;
					p_count = 0;
				}
			}
			else {	
				p_state = p_Idle;
			}
			break;

	}
	
	switch(p_state) {
		case p_init:
			break;
		case p_Idle:
			break;
		case p_Toggle_On:
			break;
		case p_Toggle_Off:
			break;
	}
	
	
}

////////////////////////////////////////////////
//////////////CombineLEDsSM//////////////////////
//////////////////////////////////////////////

enum C_States {c_init, c_combine} c_state;


void c_tick() {
	switch(c_state) {
		case c_init:
			c_state = c_combine;
			break;
		case c_combine:
			threeLEDs = shifter | B3 | p_Toggle ;
			PORTB = threeLEDs;
			break;

	}
	
	switch(c_state) {
		case c_init:
			break;
		case c_combine:
			break;
	}
	
	
}



int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;  // LED output
	DDRC = 0xFF; PORTC = 0x00; // DEBUG output
	
	b_state = b_init;
	t_state = t_init;
	c_state = c_init;
	p_state = p_init;
	f_state = f_init;
	threeLEDs = 0x00;
	TimerSet(1);
	TimerOn();
	
	while(1) {
		t_tick();
		b_tick();
		c_tick();
		p_tick();
		f_uck();
		while (!TimerFlag);
		TimerFlag = 0;
		
	}
	
	
}
