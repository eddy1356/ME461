/******************************************************************************
MSP430F2272 Project Creator 4.0

ME 461 - S. R. Platt
Fall 2010

Updated for CCSv4.2 Rick Rekoske 8/10/2011

Written by: Steve Keres
College of Engineering Control Systems Lab
University of Illinois at Urbana-Champaign
*******************************************************************************/

#include "msp430x22x2.h"
#include "UART.h"

char newprint = 0;
unsigned int timecnt = 0;
unsigned int fastcnt = 0;
char SW = 0;
unsigned int switchcnt0 = 0;
unsigned int switchcnt1 = 0;
unsigned int switchcnt2 = 0;
unsigned int switchcnt3 = 0;
unsigned int LED = 4;
// Create your global variables here:

char readSW(void);
void blinkOctagram(unsigned int);

void main(void) {
	
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT
	
	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);
	                                             
	DCOCTL  = CALDCO_16MHZ; // Set uC to run at approximately 16 Mhz
	BCSCTL1 = CALBC1_16MHZ; 

	//P1IN 		    Port 1 register used to read Port 1 pins setup as Inputs
	P1SEL &= ~0xFF; // Set all Port 1 pins to Port I/O function
	P1REN &= ~0xFF; // Disable internal resistor for all Port 1 pins
	P1DIR = 0xFF;   // Set Port 1 Pins as an output.
	P1OUT &= ~0xFF; // Initially set all Port 1 pins set as Outputs to zero

	//P2IN          Port 2 register used to read Port 2 pins setup as Inputs
	P2SEL &= ~0xFF; // Set all Port 2 pins to Port I/O function
	P2REN = 0xF0; // Enable internal resistor for port 2 pin 4 - 7.
	P2DIR = 0x00;   // Set Port 2 Pin to inputs.
	P2OUT = 0x30; // Initially set Port 2 pin 4 and 5 as pull-up and rest as pull-down.

	//P2 INTERRUPT
	P2IES = 0x30;
	P2IE = 0xF0;
	P2IFG &= ~0xF0;

	// Timer A Config
	TACCTL0 = CCIE;              // Enable Timer A interrupt
	TACCR0  = 16000;             // period = 1ms
	TACTL   = TASSEL_2 + MC_1;   // source SMCLK, up mode

	Init_UART(9600, 1);	// Initialize UART for 9600 baud serial communication

	_BIS_SR(GIE); 	    // Enable global interrupt

	while(1) {

	    if(newmsg) {
	        //my_scanf(rxbuff,&var1,&var2,&var3,&var4);
			newmsg = 0;
		}
		if (newprint)  { 
//		    blinkOctagram(LED);
		    SW = readSW();
		    if(SW == 0) {
		        P1OUT &= 0xF0; // CLEARS 0xF0
		        P1OUT ^= 0xF0; // Blink LED
		    } else {
		        P1OUT &= SW; // clears non blinking LEDs
		        P1OUT ^= SW; //Blinks the LED
		    }

 			UART_printf( "%d %d %d %d \n\r", switchcnt0 , switchcnt1,  switchcnt2, switchcnt3 ); //  %d int, %ld long, %c char, %x hex form, %.3f float 3 decimal place, %s null terminated character array
			// UART_send(1,(float)timecnt);
			
			timecnt++;  // Just incrementing this integer for default print out.
			newprint = 0;
		}

	}
}


// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if((P2IFG & 0x80) == 0x80) {
        switchcnt3++;
        P2IFG &= ~0x80;
    }
    if((P2IFG & 0x40) == 0x40) {
        switchcnt2++;
        P2IFG &= ~0x40;
    }
    if((P2IFG & 0x20) == 0x20) {
        switchcnt1++;
        P2IFG &= ~0x20;
    }
    if((P2IFG & 0x10) == 0x10) {
        switchcnt0++;
        P2IFG &= ~0x10;
    }
}

char readSW(void) {
    char sw = P2IN;
    char val = 0;

    if((~sw & 0x10) == 0x10) {
        val += 0x01;
    }
    if((~sw & 0x20) == 0x20) {
        val += 0x02;
    }
    if((sw & 0x40) == 0x40) {
        val += 0x04;
    }
    if((sw & 0x80) == 0x80) {
        val += 0x08;
    }

    return val;
}

void blinkOctagram(unsigned int get) {
    if(get == 1) {
        P1OUT &= 0x01; // CLEAR
        P1OUT ^= 0x01; // BLINK LED 1
        LED = 6;
    }
    if(get == 2) {
        P1OUT &= 0x02; // CLEAR
        P1OUT ^= 0x02; // BLINK LED 2
        LED = 7;
    }
    if(get == 3) {
        P1OUT &= 0x04; // CLEAR
        P1OUT ^= 0x04; // BLINK LED 3
        LED = 8;
    }
    if(get == 4) {
        P1OUT &= 0x08; // CLEAR
        P1OUT ^= 0x08; // BLINK LED 4
        LED = 1;
    }
    if(get == 5) {
        P1OUT &= 0x10; // CLEAR
        P1OUT ^= 0x10; // BLINK LED 5
        LED = 2;
    }
    if(get == 6) {
        P1OUT &= 0x20; // CLEAR
        P1OUT ^= 0x20; // BLINK LED 6
        LED = 3;
    }
    if(get == 7) {
        P1OUT &= 0x40; // CLEAR
        P1OUT ^= 0x40; // BLINK LED 7
        LED = 4;
    }
    if(get == 8) {
        P1OUT &= 0x80; // CLEAR
        P1OUT ^= 0x80; // BLINK LED 8
        LED = 5;
    }
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	fastcnt++; // Keep track of time for main while loop. 
	if (fastcnt == 500) {
		fastcnt = 0;
		newprint = 1;  // flag main while loop that .5 seconds have gone by.  
	}

	// Put your Timer_A code here:
    char sw = P2IN;

//    fastcnt2++;
//    if (fastcnt2 == LEDperiod){
//        blinkOctagon(LED);
//    }

    if((~sw & 0x20) == 0x20) {
        TACCR0 += 100;
    }
    if((~sw & 0x10) == 0x10) {
        TACCR0 -= 100;
    }
    if(TACCR0 > 48000) {
        TACCR0 = 48000;
    }
    if(TACCR0 < 4000){
        TACCR0 = 4000;
    }
}


/*
// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
	
}
*/


// USCI Transmit ISR - Called when TXBUF is empty (ready to accept another character)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {
  
	if((IFG2&UCA0TXIFG) && (IE2&UCA0TXIE)) { // USCI_A0 requested TX interrupt
		if(printf_flag) {
			if (currentindex == txcount) {
				senddone = 1;
				printf_flag = 0;
				IFG2 &= ~UCA0TXIFG;
			} else {
			UCA0TXBUF = printbuff[currentindex];
			currentindex++;
			}
		} else if(UART_flag) {
			if(!donesending) {
				UCA0TXBUF = txbuff[txindex];
				if(txbuff[txindex] == 255) {
					donesending = 1;
					txindex = 0;
				} else {
					txindex++;
				}
			}
		}
	    
		IFG2 &= ~UCA0TXIFG;
	}

	if((IFG2&UCB0TXIFG) && (IE2&UCB0TXIE)) { // USCI_B0 requested TX interrupt (UCB0TXBUF is empty)
		
		IFG2 &= ~UCB0TXIFG;   // clear IFG
	}
}


// USCI Receive ISR - Called when shift register has been transferred to RXBUF
// Indicates completion of TX/RX operation
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  
	if((IFG2&UCA0RXIFG) && (IE2&UCA0RXIE)) { // USCI_A0 requested RX interrupt (UCA0RXBUF is full)
  	
		if(!started) {	// Haven't started a message yet
			if(UCA0RXBUF == 253) {
				started = 1;
				newmsg = 0;
			}
		} else { // In process of receiving a message		
			if((UCA0RXBUF != 255) && (msgindex < (MAX_NUM_FLOATS*5))) {
				rxbuff[msgindex] = UCA0RXBUF;
				
				msgindex++;
			} else { // Stop char received or too much data received
				if(UCA0RXBUF == 255) { // Message completed
					newmsg = 1;
					rxbuff[msgindex] = 255;	// "Null"-terminate the array
				}
				started = 0;
				msgindex = 0;
			}
		}
		IFG2 &= ~UCA0RXIFG;
	}
	
	if((IFG2&UCB0RXIFG) && (IE2&UCB0RXIE)) { // USCI_B0 requested RX interrupt (UCB0RXBUF is full)

		IFG2 &= ~UCB0RXIFG; // clear IFG
	}
  
}



