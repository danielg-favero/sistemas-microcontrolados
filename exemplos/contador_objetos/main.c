#include <msp430.h> 

void start_p1(void);
void start_p2(void);

unsigned short int unit = 0;
unsigned short int tenth = 0;

#define     A       BIT0
#define     B       BIT1
#define     C       BIT2
#define     D       BIT3
#define     E       BIT4
#define     F       BIT5
#define     G       BIT6
#define     H       BIT7

#define     ZERO    (A + B + C + D + E + F)
#define     ONE     (B + C)
#define     TWO     (A + B + G + E + D)
#define     THREE   (A + B + C + D + G)
#define     FOUR    (B + C + F + G)
#define     FIVE    (A + C + D + G + F)
#define     SIX     (A + G + C + D + E + F)
#define     SEVEN   (A + B + C)
#define     EIGHT   (A + B + C + D + E + F + G)
#define     NINE    (A + B + C + D + F + G)

void start_p1(void);
void start_p2(void);
void show_digit(char selected_display);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	do {

	} while(1);
}

void start_p1(void){
    /*
        P1.0 - DISP_1 (Unidades) - Saída em nível baixo
        P1.1 - DISP_2 (Dezebas)  - Saída em nível baixo
        P1.2 - N.C               - Saída em nível baixo
        P1.3 - Sout              - Entrada sem configuração do resistor
        P1.4 - N.C               - Saída em nível baixo
        P1.5 - N.C               - Saída em nível baixo
        P1.6 - N.C               - Saída em nível baixo
        P1.7 - N.C               - Saída em nível baixo
    */
    P1DIR = BIT0 + BIT1;
    P1IES = BIT3;
    P1IE = BIT3;
    P1IFG = 0;
}

void start_p2(void){
    /*
       P2.0 - A     - Saída em nível baixo
       P2.1 - B     - Saída em nível baixo
       P2.2 - C     - Saída em nível baixo
       P2.3 - D     - Saída em nível baixo
       P2.4 - E     - Saída em nível baixo
       P2.5 - F     - Saída em nível baixo
       P2.6 - G     - Saída em nível baixo
       P2.7 - N.C   - Saída em nível baixo
    */
    P2SEL &= ~BIT6;   // Alterar a função digital do BIT6
    P2DIR = 0xFF;
    P2OUT = 0;
}

void show_digit(char selected_display){
    unsigned short int digit = -1;

    if(selected_display == '1'){
        digit = unit;
        P1OUT = BIT0;
    } else if (selected_display == '2') {
        digit = tenth;
        P1OUT = BIT1;
    }

    switch(digit){
    case 0:
        P2OUT = ZERO;
        break;
    case 1:
        P2OUT = ONE;
        break;
    case 2:
        P2OUT = TWO;
        break;
    case 3:
        P2OUT = THREE;
        break;
    case 4:
        P2OUT = FOUR;
        break;
    case 5:
        P2OUT = FIVE;
        break;
    case 6:
        P2OUT = SIX;
        break;
    case 7:
        P2OUT = SEVEN;
        break;
    case 8:
        P2OUT = EIGHT;
        break;
    case 9:
        P2OUT = NINE;
        break;
    default:
        P2OUT = ZERO;
        break;
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_RTI(void){
    P1IFG &= ~BIT3;

    if (unit < 9) {
        unit++;
    } else if (tenth < 9) {
        unit = 0;
        tenth++;
    } else {
        unit = 0;
        tenth = 0;
    }
}
