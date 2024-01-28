//F_CPU defined in makefile
//#define F_CPU 10000000ul

#include <avr/io.h>
#include <util/delay.h>		//temporary sin
#include <avr/interrupt.h>
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
#define IPOL_PIN		1	//ADC0 AIN1
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

#define adc_result_t	uint16_t


void PORTS_init(void);
void USART2_init(void);
void SPI0_init(void);
void SPI1_init(void);
void ADC_init(void);
void ADC_start(void);
void DAC_init(void);
void LCD_init(void);
void TOUCH_init(void);
uint16_t adc_sample(void);
void TOGGLE_LED(void);

volatile uint8_t ADC_result_available = 0;
volatile adc_result_t ADC_result = 0;

int main(void){
	PORTS_init();
	ADC_init();
    while (1){
		TOGGLE_LED();
		_delay_ms(1000);
		if(ADC_result_available){
			//process sample
			
		}
    }
}

ISR(ADC0_RESRDY_vect){
    // Store the ADC result and notify the main loop
    ADC_result = ADC0.RESL;
    ADC_result_available  = 1;

    // The Interrupt flag has to be cleared manually
    ADC0.INTFLAGS = ADC_RESRDY_bm;
}

void TOGGLE_LED(){
	PORTD.OUTTGL = 1 << LED_PIN;
}

void ADC_init(){
	//PORTD1 is Time Interpolation pin
    //Disable digital input buffer .. 18.3.2.1 and 18.3.2.3
	//to reduce noise
    PORTD.PIN1CTRL &= ~PORT_ISC_gm; //INTDISABLE
    PORTD.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc; //INPUT_DISABLE
    PORTD.PIN1CTRL &= ~PORT_PULLUPEN_bm; //Disable pull-up resistor
	//tau = 150 * 680 ~= 100ns
	//V = 5V, so at tau we reach max 0.63*5 = 3.15V
	//The board has the footprint for an external Vref at PD7
	//If populated, the ADC ref V is:
	VREF.ADC0REF = VREF_REFSEL_VREFA_gc;
	//If not populated, the best ADC ref V is 4.096V, the diff is 0.946V
	//which is just below 25% of our max value, so we lose 0.5 bits precision:
	//VREF.ADC0REF = VREF_REFSEL_4V096_gc;
	//
	//We measure time IPOL at pin 7, ADC0 AIN1
	ADC0.MUXPOS = ADC_MUXPOS_AIN1_gc;
	//TODO: figure out CLK_PER
	ADC0.CTRLC = ADC_PRESC_DIV4_gc;
	//Just 1 sample, no accumulation
	ADC0.CTRLB= ADC_WINCM_NONE_gc;
	//ENable ADC in 12-bit mode
	ADC0.CTRLA= ADC_ENABLE_bm | ADC_RESSEL_12BIT_gc;
}

void ADC_start(){
	//start conversion
	ADC0.COMMAND = ADC_STCONV_bm;
	//TODO: change pin to digital output and write 0 to discharge the capacitor
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

