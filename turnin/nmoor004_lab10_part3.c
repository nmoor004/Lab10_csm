/*	Author: nmoor004
 *  Partner(s) Name: 
 *	Lab Section: 
 *	Assignment: Lab # 10 Exercise # 3
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


///////////////////////////////////////
////////////ThreeLEDsSM//////////////
///////////////////////////////////////

enum 3_States {3_init, 3_Shift} 3_state;

unsigned char shifter;
unsigned short 3_count;

void 3_Tick() {
	switch(3_state) {
		case 3_init:
			3_count = 0;
			shifter = 0x01;
			3_state = 3_Shift;
			break;
		case 3_Shift: 
		if (3_count != 500) {
			3_count++;
		}
		else {
			if (shifter < 0x04) {
			shifter = shifter << 1;
			}
			else {
				shifter = 0x01;
			}
		}
			break;
	}
	
	switch(3_state) {
		case 3_init:
			break;
		case 3_Shift: 
			break;
	}
	
	
}



////////////////////////////////////////////////
//////////////BlinkingLEDSM//////////////////////
//////////////////////////////////////////////

enum B_States {b_init, b_Idle, b_On, b_Off} b_state;
unsigned short b_count = 0x00;
unsigned char b_type = 0x00;
unsigned char B3;
void b_tick() {
	switch(b_state) {
		case b_init:
			B3 = 0x00;
			b_type = 0x00;
			b_state = b_Idle;
			break;
		case b_Idle:
			if (b_count != 150) {
					b_count++;
			}
			else {
				b_count = 0;
				if (b_type == 0) {
					b_state = b_On;
					b_type = 1;
				}
				else if (b_type == 1) {
					b_state = b_Off;
					b_type = 0;
			break;
		case b_On:
			B3 = B3 | 0x08;
			b_state = b_Idle;
			break;
		case b_Off:
			B3 = B3 & 0x07;
			b_state = b_Idle;
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

////////////////////////////////////////////////
//////////////CombineLEDsSM//////////////////////
//////////////////////////////////////////////

enum C_States {c_init, c_combine} c_state;


void c_tick() {
	switch(c_state) {
		case c_init:
			threeLEDs = 0x00;
			c_state = c_combine;
			break;
		case c_combine:
			threeLEDs = (shifter | (B3)) | (PWM_Toggle);
			PORTB = threeLEDs;
			break;

	}
	
	switch(c_state) {
		case c_init:
			break;
		case c_On:
			break;
		case c_Off:
			break;
	}
	
	
}

////////////////////////////////////////////////
//////////////PWMToggleSM//////////////////////
//////////////////////////////////////////////

enum P_States {p_init, p_Idle, p_Toggle_On, p_Toggle_Off) p_state;
unsigned char p_count = 0x00;
unsigned char PWM_Toggle = 0x00;
void p_tick() {
	unsigned char PA2 = (PINA & 0x04) >> 2;
	switch(p_state) {
		case p_init:
			PWM_Toggle = 0x00;
			p_count = 0x00;
			p_state = p_Toggle;
			break;
		case p_Toggle_On:
			if (!PA2) { //Buttons are inverted for some reason
				if (p_count != 2) {
					p_count++;
				}
				else {
					PWM_Toggle = 0x04;
					p_state = p_Toggle_Off;
			break;
			
		case p_Toggle_On:
			if (!PA2) { //Buttons are inverted for some reason
				if (p_count != 2) {
					p_count++;
				}
				else {
					PWM_Toggle = 0x00;
					p_state = p_Toggle_Off;
			break;

	}
	
	switch(p_state) {
		case p_init:
			break;
		case p_Toggle_On:
			break;
		case p_Toggle_Off;
			break;
	}
	
	
}

void main() {
	DDRB = 0xFF; PORTA = 0x00;
	
	b_state = b_init;
	3_state = 3_init;
	c_state = c_combine;
	threeLEDs = 0x00;
	TimerSet(2);
	TimerOn();
	
	while(1) {
		3_Tick();
		b_tick();
		c_tick();
		while (!TimerFlag);
		TimerFlag = 0;
		
	}
	
	
}
