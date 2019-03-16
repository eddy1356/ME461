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

// Create your global variables here:
long value = 0;
long voltage = 0;
long RPM = 0;
long angle = 0;
char SW = 0;
long desiredAngle = 0;
long DC = 0 ;
char readSW(void);

void main(void) {
	
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT
	
	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF) while(1);
	                                             
	DCOCTL  = CALDCO_16MHZ; // Set uC to run at approximately 16 Mhz
	BCSCTL1 = CALBC1_16MHZ; 
		
	//P1IN 		    Port 1 register used to read Port 1 pins setup as Inputs
	P1SEL &= ~0xFF; // Set all Port 1 pins to Port I/O function
	P1REN &= ~0xFF; // Disable internal resistor for all Port 1 pins
	P1DIR |= 0x1;   // Set Port 1 Pin 0 (P1.0) as an output.  Leaves Port1 pin 1 through 7 unchanged
	P1OUT &= ~0xFF; // Initially set all Port 1 pins set as Outputs to zero

    //P2IN          Port 2 register used to read Port 2 pins setup as Inputs
    P2SEL &= ~0xFF; // Set all Port 2 pins to Port I/O function
    P2REN = 0xF0; // Enable internal resistor for port 2 pin 4 - 7.
    P2DIR = 0x00;   // Set Port 2 Pin to inputs.
    P2OUT = 0x30; // Initially set Port 2 pin 4 and 5 as pull-up and rest as pull-down.

	// Timer A Config
	TACCTL0 = CCIE;              // Enable Timer A interrupt
	TACCR0  = 16000;             // period = 1ms   
	TACTL   = TASSEL_2 + MC_1;   // source SMCLK, up mode

	// Timer B Config
	TBCCR0 = 800;
	TBCTL   = TBSSEL_2 + MC_1; // source SMCLK, up mode
	TBCCTL1 = OUTMOD_7;

	// P4.1/TB1 Setting
	P4DIR = 0x82;
	P4SEL = 0x02;

	// ADC10 Config
	ADC10CTL0 |= SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;
    ADC10AE0 |= 0x02;
    ADC10CTL1 |= INCH_2;
	
    Init_UART(9600, 1);	// Initialize UART for 9600 baud serial communication

	_BIS_SR(GIE); 	    // Enable global interrupt

	while(1) {

		if(newmsg) {
			//my_scanf(rxbuff,&var1,&var2,&var3,&var4);
			newmsg = 0;
		}

		if (newprint)  { 
//			P1OUT ^= 0x1; // Blink LED
 			char sw_sense = readSW();
// 		    if((sw_sense&0x04) == 0){
// 		        UART_printf("%ld %ld\n\r",angle,desiredAngle); //  %d int, %ld long, %c char, %x hex form, %.3f float 3 decimal place, %s null terminated character array
// 		    }
// 		    if((sw_sense&0x04) == 0x04) {
// 		        angle = -1*angle;
// 		        voltage = -1*voltage;
// 		        UART_printf("%ld  %ld \n\r",angle,desiredAngle ); //  %d int, %ld long, %c char, %x hex form, %.3f float 3 decimal place, %s null terminated character array
// 		    }
		    UART_printf("%ld %ld\n\r",angle,desiredAngle); //  %d int, %ld long, %c char, %x hex form, %.3f float 3 decimal place, %s null terminated character array
			// UART_send(1,(float)timecnt);

			timecnt++;  // Just incrementing this integer for default print out.
			newprint = 0;
		}

	}
}
char readSW(void) {
    char sw = P2IN;
    char val = 0;

    if((~sw & 0x10) == 0x10) { //4
        val += 0x01;
    }
    if((~sw & 0x20) == 0x20) { //5
        val += 0x02;
    }
    if((sw & 0x40) == 0x40) { //6
        val += 0x04;
    }
    if((sw & 0x80) == 0x80) { //7
        val += 0x08;
    }

    return val;
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
	fastcnt++; // Keep track of time for main while loop. 
	if (fastcnt == 250) {
		fastcnt = 0;
		newprint = 1;  // flag main while loop that .5 seconds have gone by.  
	}


	// ADC10 TRIGGER
	ADC10CTL0 |= ADC10SC + ENC;
}



// ADC 10 ISR - Called when a sequence of conversions (A7-A0) have completed
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    value = (long)ADC10MEM;
  voltage = (value*3600L)/1023L;  //voltage in mV
  angle = (-93L*voltage)/1000 + 179;
//    voltage = ((value*3600L)/1023L);
//    angle = (385L*voltage)/10000L;
    //RPM = ((-voltage*2509L)/1000L) + 3753;
    // Put your Timer_B code here:
    SW = readSW();

    if((SW&0x03) == 0) {
        desiredAngle = 0;
//        TBCCR1 = 0;
    }
    if((SW&0x03) == 0x01) {
        desiredAngle = 10;
//        TBCCR1 = 91;
    }
    if((SW&0x03) == 0x02) {
        desiredAngle = 20;
//        TBCCR1 = 182;
    }
    if((SW&0x03) == 0x03) {
        desiredAngle = 30;
//        TBCCR1 = 274;
    }
    DC = ((desiredAngle*114L) / 100L);
    if((SW&0x04) == 0x04){
        desiredAngle *= -1;
        P4OUT = 0x80;
    }
    if((SW&0x04) == 0x00) {
        P4OUT = 0x00;
    }
    TBCCR1 = (DC *TBCCR0) / 100L;
}


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



