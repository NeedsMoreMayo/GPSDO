

#include <avr/io.h>
#include <util/delay.h>		//temporary sin
#include <avr/interrupt.h>

#include "adc.h"

volatile uint8_t ADC_result_available = 0;
volatile adc_result_t ADC_result = 0;


ISR(ADC0_RESRDY_vect){
    // Store the ADC result and notify the main loop
    ADC_result = ADC0.RES;
    ADC_result_available  = 1;
    // The Interrupt flag has to be cleared manually
	// According to 33.5.12 the flag is already cleared by reading the ADC result (?)
    ADC0.INTFLAGS = ADC_RESRDY_bm;
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
	//Enable ADC in 12-bit mode
	ADC0.CTRLA= ADC_ENABLE_bm | ADC_RESSEL_12BIT_gc;
}

void ADC_start(){
	//enable interrupt
	ADC0.INTCTRL = ADC_RESRDY_bm;
	//start conversion
	ADC0.COMMAND = ADC_STCONV_bm;
	//TODO: change pin to digital output and write 0 to discharge the capacitor
}