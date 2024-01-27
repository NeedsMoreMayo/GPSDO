#define F_CPU 10000000ul

#include <avr/io.h>
#include <util/delay.h>		//temporary sin
//#include <math.h>


//PORT A
#define uC_CLK_PIN		0
#define LCD_SS_PIN		1
#define LCD_RESET_PIN	2
#define LCD_DCRS_PIN	3
#define LCD_MOSI_PIN	4
#define LCD_MISO_PIN	5
#define LCD_SCK_PIN		6
#define TOUCH_IRQ_PIN	7

//PORT C
#define DAC_MOSI_PIN	0
#define TOUCH_MOSI_PIN	0
#define TOUCH_MISO_PIN	1
#define DAC_SCK_PIN		2
#define TOUCH_SCK_PIN	2
#define TOUCH_SS_PIN	3

//PORT D
#define IPOL_PIN		1	//ADC
#define DAC_SS_PIN		2
#define LED_PIN			3
#define CUSTOM_OUT_PIN	4
#define ICP1_PIN		5
#define FF_SET_PIN		6
#define FF_RESET_PIN	7

//PORT F
#define GPS_TXD_PIN		0
#define GPS_RXD_PIN		1
#define RESET_PIN		6



void PORTS_init(void);
void USART2_init(void);
void SPI0_init(void);
void SPI1_init(void);
void ADC_init(void);
void DAC_init(void);
void LCD_init(void);
void TOUCH_init(void);
uint16_t adc_sample(void);
void TOGGLE_LED(void);


int main(void){
	PORTS_init();
	ADC_init();
    while (1){
		TOGGLE_LED();
		_delay_ms(1000);
    }
}

uint16_t adc_sample(){
	uint16_t res;
	ADC0.COMMAND = ADC_STCONV_bm;
	while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
	/*Right shift result by 4 due to 16 over samples*/
	res=ADC0.RES>>4;
	return res;
}

void TOGGLE_LED(){
	PORTD.OUTTGL = 1 << LED_PIN;	//out toggle
}

void ADC_init(){
	//PORTD1 is Time Interpolation pin
	//tau = 150 * 680 ~= 100ns
	//V = 5V, so at tau we reach max 0.63*5 = 3.15V
	//set max resolution to 4.096V, the diff is 0.946V
	//which is just below 25% of our max value, so we lose 0.5 bits precision
	VREF.ADC0REF = VREF_REFSEL_4V096_gc;
	ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;
	ADC0.CTRLB= ADC_SAMPNUM_ACC16_gc;
	ADC0.CTRLA= ADC_ENABLE_bm;
}

void PORTS_init(){
	//LCD_SS, LCD_RESET, LCD_DCRS, LCD_MOSI, LCD_SCK are output on PORTA
	PORTA.DIR = 1 << LCD_SS_PIN | 1 << LCD_RESET_PIN | 1 << LCD_DCRS_PIN | 1 << LCD_MOSI_PIN | 1 << LCD_SCK_PIN;
	PORTA.OUT = 0x00;
	
	//DAC_MOSI (TOUCH_MOSI), DAC_SCK (TOUCH_SCK), DAC_SCK are output on PORTC
	PORTC.DIR = 1 << DAC_MOSI_PIN | 1 << DAC_SCK_PIN | 1 << DAC_SCK_PIN;
	PORTC.OUT = 0x00;
	
	//LED, DAC_SS, CUSTOM_OUT, FF_SET, FF_RESET are output on PORTD
	PORTD.DIR = 1 << LED_PIN | 1 << DAC_SS_PIN | 1 << CUSTOM_OUT_PIN | 1 << FF_SET_PIN | 1 << FF_RESET_PIN;
	PORTD.OUT = 0x00;
	
	//GPS_TXD is output on PORTD
	PORTF.DIR = 1 << GPS_TXD_PIN;
	PORTF.OUT = 0x00;
}

