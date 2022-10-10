#include <msp430.h> 

#define QUARTER               0x02
#define HALF                  0x05
#define THREE_QUARTERS        0x0A
#define FULL                  0x0C

#define LED_EMPTY             BIT0
#define LED_QUARTER           BIT1
#define LED_HALF              BIT2
#define LED_THREE_QUARTERS    BIT6
#define LED_FULL              BIT7

unsigned int enconder_output = 0;

void start_p1(void);
void start_p2(void);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	start_p1();
	start_p2();

	do {
	    /*
	        Os bits de entrada são 1, 2, 3 e 4 respectivamente,
	        portanto para fazer a comparação do nível do tanque
	        com o encoder, é preciso deslocar os bits uma casa
	        para a direita

              P1IN = 11111111
	        & 0X1E = 00011110
	                 --------
	                 00011110
	        ÷               2
	                 --------
	                 00001111
	    */
	    enconder_output = (P1IN & 0x1E) / 2;

	    if (enconder_output >= FULL) {
	        P2OUT = LED_EMPTY + LED_QUARTER + LED_HALF + LED_THREE_QUARTERS + LED_FULL;
	    } else if (enconder_output < FULL && enconder_output >= THREE_QUARTERS) {
	        P2OUT = LED_EMPTY + LED_QUARTER + LED_HALF + LED_THREE_QUARTERS;
	    } else if (enconder_output < THREE_QUARTERS && enconder_output >= HALF) {
	        P2OUT = LED_EMPTY + LED_QUARTER + LED_HALF;
	    } else if (enconder_output < HALF && enconder_output >= QUARTER) {
	        P2OUT = LED_EMPTY + LED_QUARTER;
	    } else {
	        P2OUT = LED_EMPTY;
	    }

	} while(1);
}

void start_p1(void) {
    /*
        P1.1 - S0 - Entrada com resistor pull-up
        P1.2 - S1 - Entrada com resistor pull-up
        P1.3 - S2 - Entrada com resistor pull-up
        P1.4 - S3 - Entrada com resistor pull-up
    */
    P1DIR = ~(BIT1 + BIT2 + BIT3 + BIT4);
    P1OUT = BIT1 + BIT2 + BIT3 + BIT4;
    P1REN = BIT1 + BIT2 + BIT3 + BIT4;
    P1IES = BIT1 + BIT2 + BIT3 + BIT4;
}

void start_p2(void) {
    /*
        P2.0 - LED1 - Saída em nível baixo
        P2.1 - LED2 - Saída em nível baixo
        P2.2 - LED3 - Saída em nível baixo
        P2.6 - LED5 - Saída em nível baixo
        P2.7 - LED4 - Saída em nível baixo
    */
    P2SEL = 0;
    P2DIR = 0xFF;
    P2OUT = BIT0 + BIT1 + BIT2 + BIT6 + BIT7;
}
