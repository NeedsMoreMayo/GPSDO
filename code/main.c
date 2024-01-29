//F_CPU defined in makefile
//#define F_CPU 10000000ul

#include <avr/io.h>
#include <util/delay.h>		//temporary sin
#include <avr/interrupt.h>
//#include <math.h>

#include "uart.h"
#include "adc.h"


//PORT A
#define uC_CLK_PIN		0
#define LCD_SS_PIN		1
#define LCD_RESET_PIN	2
#define LCD_DCRS_PIN	3
#define LCD_MOSI_PIN	4
#define LCD_MISO_PIN	5
#define LCD_SCK_PIN		6
#define TOUCH_SS_PIN	7

//PORT C
#define DAC_MOSI_PIN	0
#define TOUCH_MOSI_PIN	0
#define TOUCH_MISO_PIN	1
#define DAC_SCK_PIN		2
#define TOUCH_SCK_PIN	2
#define TOUCH_IRQ_PIN	3

//PORT D
#define IPOL_PIN		1	//ADC0 AIN1
#define DAC_SS_PIN		2
#define LED_PIN			3
#define CUSTOM_OUT_PIN	4
#define ICP1_PIN		5
#define UNUSED_PIN		6
#define VREF_3V3_PIN	7

//PORT F
#define GPS_TXD_PIN		0
#define GPS_RXD_PIN		1
#define RESET_PIN		6

void PORTS_init(void);
void SPI0_init(void);
void SPI1_init(void);
void DAC_init(void);
void LCD_init(void);
void TOUCH_and_DAC_SPI_init(void);
void TOUCH_IRQ_init(void);
void TOGGLE_LED(void);

extern uint8_t ADC_result_available;

int main(void){
	cli(); //disable interrupts when enabling interrupt handlers
	PORTS_init();
	ADC_init();
	TOUCH_IRQ_init();
	USART2_init();
	sei();//enable interrupts
    while (1){
		TOGGLE_LED();
		_delay_ms(1000);
		if(ADC_result_available){
			//process sample
			
		}
    }
}

ISR(PORTC_PORT_vect){
	if(PORTC_INTFLAGS & (1 << TOUCH_IRQ_PIN)){
		//react
		PORTC.INTFLAGS = 1 << TOUCH_IRQ_PIN;
	}
}


void TOGGLE_LED(){
	PORTD.OUTTGL = 1 << LED_PIN;
}

void TOUCH_IRQ_init(){
	//PORTC pin 3
	//no pull-up, interrupt on rising edge (TODO: check display datasheet)
	PORTC.PIN3CTRL = PORT_ISC_RISING_gc;
}

void PORTS_init(){
	//LCD_SS, LCD_RESET, LCD_DCRS, LCD_MOSI, LCD_SCK and TOUCH_SS are output on PORTA
	PORTA.DIR =   (1 << LCD_SS_PIN)
				| (1 << LCD_RESET_PIN)
				| (1 << LCD_DCRS_PIN)
				| (1 << LCD_MOSI_PIN)
				| (1 << LCD_SCK_PIN)
				| (1 << TOUCH_SS_PIN);
	PORTA.OUT = 0x00;
	
	//DAC_MOSI (TOUCH_MOSI), DAC_SCK (TOUCH_SCK) are output on PORTC
	PORTC.DIR =   (1 << DAC_MOSI_PIN)
				| (1 << DAC_SCK_PIN);
	PORTC.OUT = 0x00;
	
	//LED, DAC_SS and CUSTOM_OUT are output on PORTD
	PORTD.DIR =   (1 << LED_PIN)
				| (1 << DAC_SS_PIN)
				| (1 << CUSTOM_OUT_PIN);
	PORTD.OUT = 0x00;
	
	//GPS_TXD is output on PORTD
	PORTF.DIR = 1 << GPS_TXD_PIN;
	PORTF.OUT = 0x00;
}