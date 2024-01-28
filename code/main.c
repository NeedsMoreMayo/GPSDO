//F_CPU defined in makefile
//#define F_CPU 10000000ul
#define USART2_BAUD_RATE(BAUD_RATE)     ((float)(64 * F_CPU / (16 * (float)BAUD_RATE)) + 0.5) //TODO: check CLK settings for this calc

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

#define adc_result_t	uint16_t


void PORTS_init(void);
void USART2_init(void);
void SPI0_init(void);
void SPI1_init(void);
void ADC_init(void);
void ADC_start(void);
void DAC_init(void);
void LCD_init(void);
void TOUCH_and_DAC_SPI_init(void);
void TOUCH_IRQ_init(void);
uint16_t adc_sample(void);
void TOGGLE_LED(void);

volatile uint8_t ADC_result_available = 0;
volatile adc_result_t ADC_result = 0;
#define RXBUF_len 256
volatile uint8_t RXBUF[RXBUF_len];
volatile uint16_t RXBUF_idx = 0;

int main(void){
	cli(); //disable interrupts when enabling interrupt handlers
	PORTS_init();
	ADC_init();
	TOUCH_IRQ_init();
	sei();//enable interrupts
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
	// According to 33.5.12 the flag is already cleared by reading the ADC result (?)
    ADC0.INTFLAGS = ADC_RESRDY_bm;
}

ISR(PORTC_PORT_vect){
	if(PORTC_INTFLAGS & (1 << TOUCH_IRQ_PIN)){
		//react
		PORTC.INTFLAGS = 1 << TOUCH_IRQ_PIN;
	}
}

ISR(USART0_RXC_vect){
	//USART2.STATUS = 1 << USART_RXCIF_bp; //clear RX Complete Interrupt Flag
	//RXCIF is read-only and is cleared by reading the RXDATA register
	RXBUF[RXBUF_idx++] = USART0.RXDATAL;
		if (RXBUF_idx >= RXBUF_len) RXBUF_idx =0;
}

ISR(USART0_TXC_vect){
	USART2.STATUS = 1 << USART_TXCIF_bp; //clear TX Complete Interrupt Flag
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
	//enable interrupt
	ADC0.INTCTRL = ADC_RESRDY_bm;
	//start conversion
	ADC0.COMMAND = ADC_STCONV_bm;
	//TODO: change pin to digital output and write 0 to discharge the capacitor
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

void USART2_init(void){
	//PF0 = TXD
	//PF1 = RXD
	//9600 baud, 8 bits, no parity bit, 1 stop bit
	// CMODE=Async, PMODE=Parity Disabled, SBMODE=1 stop bit, CHSIZE=8 bits/char
	PORTMUX.USARTROUTEA &= ~(3 << PORTMUX_USART2_0_bp);	//clr bits 4 and 5
    USART2.CTRLA = (1 << USART_RXCIE_bp) | (1 << USART_TXCIE_bp); //  RX Complete Interrupt Enable,  TX Complete Interrupt Enable
    USART2.CTRLB = (1 << USART_RXEN_bp) | (1 << USART_TXEN_bp) | (1 << USART_RXMODE_0_bp); // USART_RXEN = RX enable, USART_TXEN = TX enable, USART_RXMODE_0_bm = normal speed mode
	USART2.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc | USART_CHSIZE_8BIT_gc; 
	//17.3.3  Default pins for TXD RXD .. PF2 and PF3 not on device
    USART2.BAUD = (uint16_t)(USART2_BAUD_RATE(9600));
	USART2.STATUS = 1 << USART_TXCIF_bp; //clear Transmit Complete Interrupt Flag
}