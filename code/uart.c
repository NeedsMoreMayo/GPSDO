#include <avr/io.h>
#include <util/delay.h>		//temporary sin
#include <avr/interrupt.h>

#include "uart.h"

#define USART2_BAUD_RATE(BAUD_RATE)     ((float)(64 * F_CPU / (16 * (float)BAUD_RATE)) + 0.5) //TODO: check CLK settings for this calc


#define RXBUF_len 256
volatile uint8_t RXBUF[RXBUF_len];
volatile uint16_t RXBUF_idx = 0;

#define TXBUF_len 512
volatile uint8_t TXBUF[TXBUF_len];
volatile uint16_t TXBUF_idx = 0;
volatile uint8_t transmitting = 0;



void USART2_init(void){
	//PF0 = TXD
	//PF1 = RXD
	PORTMUX.USARTROUTEA &= ~(3 << PORTMUX_USART2_0_bp);	//clr bits 4 and 5
    USART2.BAUD = (uint16_t)(USART2_BAUD_RATE(9600));
	USART2.CTRLC = USART_CMODE_ASYNCHRONOUS_gc	//Async mode
				 | USART_PMODE_DISABLED_gc		//no parity bit
				 | USART_SBMODE_1BIT_gc			//1 stop bit
				 | USART_CHSIZE_8BIT_gc; 		//8 bits/char
	//17.3.3  Default pins for TXD RXD .. PF2 and PF3 not on device
    USART2.CTRLB = (1 << USART_RXEN_bp)		// USART_RXEN = RX enable
				 | (0 << USART_TXEN_bp)		// USART_TXEN = TX enable ... disabled
				 | (1 << USART_RXMODE_0_bp);// USART_RXMODE_0_bm = normal speed mode
    USART2.CTRLA = (1 << USART_RXCIE_bp)	// RX Complete Interrupt Enable
				 | (0 << USART_TXCIE_bp)	// TX Complete Interrupt Enable ... disabled
				 | (1 << USART_DREIE_bp);	// Data Register Empty Interrupt Enable
	USART2.STATUS = 1 << USART_TXCIF_bp; //clear Transmit Complete Interrupt Flag
}


//Data Register Empty interrupt
//The entire frame in the Transmit Shift register has been shifted out and there
//are no new data in the transmit buffer (TXCIE)
ISR(USART2_DRE_vect){
	if(transmitting == 0){
		//previous char must've been last one
		USART2_disable_TX();
	}
	uint8_t character = TXBUF[TXBUF_idx++];
	if (TXBUF_idx >= TXBUF_len) TXBUF_idx = 0;
	USART1.TXDATAL = character;
	if(character == '\0'){
		//end of transmission
		transmitting = 0;
	}
}

//Receive Complete interrupt
//There is unread data in the receive buffer (RXCIE)
//Receive of Start-of-Frame detected (RXSIE)
//Auto-Baud Error/ISFIF flag set (ABEIE)
ISR(USART2_RXC_vect){
	//The NEO-M8N sends data as strings, terminated by 0Dh,0Ah and beginning with $G
	
	//USART2.STATUS = 1 << USART_RXCIF_bp; //clear RX Complete Interrupt Flag
	//RXCIF is read-only and is cleared by reading the RXDATA register
	RXBUF[RXBUF_idx++] = USART0.RXDATAL;
	if (RXBUF_idx >= RXBUF_len) RXBUF_idx = 0;
}

//Transmit Complete interrupt
//The entire frame in the Transmit Shift register has been shifted out and there
//are no new data in the transmit buffer (TXCIE)
/*
ISR(USART2_TXC_vect){
	USART2.STATUS = 1 << USART_TXCIF_bp; //clear TX Complete Interrupt Flag
}
*/

void USART2_disable_TX(){
    USART2.CTRLB &= ~(1 << USART_TXEN_bp);	// TX enable OFF
    USART2.CTRLA &= ~(1 << USART_DREIE_bp);	// Data Register Empty Interrupt Enable OFF
}

void USART2_enable_TX(){
    USART2.CTRLB |= (1 << USART_TXEN_bp);	// TX enable ON
    USART2.CTRLA |= (1 << USART_DREIE_bp);	// Data Register Empty Interrupt Enable ON
}

void USART2_send_string(uint8_t *string, uint16_t index){
	USART2_enable_TX();
	//wait for Data Register to become ready 27.5.3
	while(!(USART2.STATUS & (1 << USART_DREIF_bp))){;}
	USART1.TXDATAL = string[index++];// send first char, interrupts deal with the rest
}