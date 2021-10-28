/*
 * GccApplication1.c
 *
 * Created: 27.02.2021 10:45:18
 * Author : mszoz
 */
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>

#define LED_ON PORTD = PORTD | (1<<7);
#define LED_OFF PORTD = PORTD & ~(1<<7);

#define BUZZER_ON PORTD = PORTD | (1<<3);
#define BUZZER_OFF PORTD = PORTD & ~(1<<3);

#define STB_HIGH PORTB = PORTB | 1;
#define STB_LOW PORTB = PORTB & 0;

#define CLK_HIGH PORTB = PORTB | (1<<1);
#define CLK_LOW	 PORTB = PORTB & ~(1<<1);

#define DATA_HIGH PORTB = PORTB | (1<<2);
#define DATA_LOW PORTB = PORTB & ~(1<<2);

#define HP_HIGH PORTC = PORTC | 0b00000111;
#define HP_LOW	PORTC = PORTC | 0b00000000;

volatile bool led_state=0;
volatile bool buzzer_state=0;
volatile bool add_second=0;
volatile bool alarm_mode=0;
volatile bool turn_on_lamps=1;
volatile bool alarm_flag=0;
volatile uint8_t phase=0;

volatile uint8_t hours=13;
volatile uint8_t minutes=22;
volatile uint8_t seconds=30;

volatile uint8_t alarm_hours=13;
volatile uint8_t alarm_minutes=23;


uint32_t lamp1[] = {0x0101dd,0x010090,0x010156,0x010196,0x01009a,0x01018e,0x0101ce,0x010094,0x0101df,0x01019f}; //0 1 2 3 4 5 6 7 8 9
uint32_t lamp2[] = {0x0081fd,0x0080b0,0x008176,0x0081b6,0x0080ba,0x0081ae,0x0081ee,0x0080b4,0x0081ff,0x0081bf}; //0 1 2 3 4 5 6 7 8 9
uint32_t lamp3[] = {0x0005dd,0x000490,0x000556,0x000596,0x00049a,0x00058e,0x0005ce,0x000494,0x0005df,0x00059f}; //0 1 2 3 4 5 6 7 8 9
uint32_t lamp4[] = {0x0009dd,0x000890,0x000956,0x000996,0x00089a,0x00098e,0x0009ce,0x000894,0x0009df,0x00099f};	//0 1 2 3 4 5 6 7 8 9

void timer0_init(){
TCCR0B = (0<<CS02) | (1<<CS01) | (1<<CS00);
TIMSK0 |= (1<<TOIE0);
}

void timer2_init(){ 
	TCCR2A = 0;
	//TCCR2B = (1<<CS02) | (0<<CS01) | (1<<CS00); // tick every second
	TCCR2B = (1<<CS02) | (0<<CS01) | (0<<CS00); // tick every 0.5 second
	TIMSK2 = (1<<TOIE2);
	ASSR = (1<<AS2);
}

void data_set(uint8_t d) {
	if (d)
		DATA_HIGH
	else
		DATA_LOW
}

void send_byte(uint8_t d) {
	for(uint8_t i=128;i>0;i>>=1) {
		CLK_LOW
		data_set(d&i);
		CLK_HIGH
	}
}

void beep(){
	BUZZER_ON
	_delay_ms(50);
	BUZZER_OFF
}

void toggle_led(){
	if(led_state==0) {
		LED_ON
		led_state=1;
	}
	else {
		LED_OFF
		led_state=0;
	}	
}

int main(void)
{
DDRD = 0b10001000; //LED 13 OUTPUT BUZZER OUTPUT
DDRB = 0b00000111; // Strobe/CLK/Data OUTPUT
DDRC = 0b00011111; //PC 0 1 2 3 4 5 OUTPUT


HP_HIGH
HP_LOW
//guzik1
//EICRA |= (1<<ISC01) | (0<<ISC00);
//EIMSK |= (1<<INT0); 

PCMSK2=0b01110100;
PCICR |= (1<<PCIE2);

timer0_init();
timer2_init();
sei();

    while (1) 
    {	
		if(alarm_hours==hours && alarm_minutes==minutes && alarm_flag==1 && alarm_mode==0)
			beep();
		_delay_ms(500);
	
	}
}
//ISR(INT0_vect){ //guzik1
//	if(led_state==0) {
//		LED_ON
//		led_state=1;
//	}
//	else {
//		LED_OFF
//		led_state=0;
//	}
//}
ISR(PCINT2_vect){
	if(!(PIND & (1<<PORTD2) )) //falling edge
	{
		//toggle_led();	
		alarm_mode=1;
		if(alarm_flag) {
			alarm_flag=0;
			alarm_mode=0;
		}
			
	}
	
	if(!(PIND & (1<<PORTD4) )) //falling edge
	{
		//toggle_led();
		if(alarm_mode==1) {
			alarm_flag=1;
			alarm_mode=0;
			turn_on_lamps=1;
		}
		
	}
		
	
	if(!(PIND & (1<<PORTD5) )) //falling edge
	{
		//toggle_led();
		if(alarm_mode){
			alarm_hours++;
		}else{
			hours++;
		}
	}
	
	if(!(PIND & (1<<PORTD6) )) //falling edge
	{
		//toggle_led();
		if(alarm_mode){
			alarm_minutes++;
		}else{
			minutes++;
		}
	}	
}
ISR(TIMER2_OVF_vect) {
	if(alarm_flag)
		LED_ON
	else
		LED_OFF
	if(add_second){
		add_second=0;
		seconds++;
	} else {
		add_second=1;
	}		
	
	if(alarm_mode) {
		turn_on_lamps = !turn_on_lamps;
	}
				
	if(seconds>=60){
		seconds=0;
		minutes++;
	}
	if(minutes>=60){
		minutes=0;
		hours++;
	}
	if(hours>=24){
		hours=0;
	}

	if(hours==23 && minutes==59 && seconds==59) {
		seconds=0;
		minutes=0;
		hours=0;
	}	
}
ISR(TIMER0_OVF_vect){

	if(alarm_hours>23) {
		alarm_hours=0;
	}
	if(alarm_minutes>59){
		alarm_minutes=0;
	}
	
	uint8_t v;
	if(turn_on_lamps) {
		if(phase==0){
			if(alarm_mode) {
				v=alarm_hours/10;
			}else {
				v=hours/10;
			}
				send_byte(lamp1[v]>>16);
				send_byte(lamp1[v]>>8);
				send_byte(lamp1[v]);
				STB_HIGH
				STB_LOW
				phase=1;				
		}else if(phase==1){
			if(alarm_mode){
				v=alarm_hours%10;
			}else{
				v=hours%10;
			}
				send_byte(lamp2[v]>>16);
				send_byte(lamp2[v]>>8);
				send_byte(lamp2[v]);
				STB_HIGH
				STB_LOW
				phase=2;
		}else if(phase==2){
			if(alarm_mode){
				v=alarm_minutes/10;
			}else {
				v=minutes/10;
			}
				send_byte(lamp3[v]>>16);
				send_byte(lamp3[v]>>8);
				send_byte(lamp3[v]);
				STB_HIGH
				STB_LOW
				phase=3;
		}else if(phase==3){
			if(alarm_mode){
				v=alarm_minutes%10;
			}	else {
				v=minutes%10;
			}
			send_byte(lamp4[v]>>16);
			send_byte(lamp4[v]>>8);
			send_byte(lamp4[v]);
			STB_HIGH
			STB_LOW
			phase=0;
		}
	}else {
		send_byte(0x00);
		send_byte(0x00);
		send_byte(0x00);
		STB_HIGH
		STB_LOW
	}
					

}
