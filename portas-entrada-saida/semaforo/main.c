#include <msp430.h> 

#define LD1_Green       BIT0
#define LD1_Yellow      BIT1
#define LD1_Red         BIT2

#define LD2_Green       BIT3
#define LD2_Yellow      BIT4
#define LD2_Red         BIT5

#define LDP1_Green      BIT7
#define LDP1_Red        BIT6

#define LDP2_Green      BIT6
#define LDP2_Red        BIT7

void start_p1_p2(void);

unsigned char traffic_light_state = 0;
unsigned long i = 0;
unsigned int switch1_green_time = 8;
unsigned int switch2_green_time = 8;

void main(void)
{
    unsigned int t = 0;

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	__enable_interrupt();

	// Iniciar as portas do MSP430
	start_p1_p2();

	// Máquina de estados para o semáforo
	do {
	    switch(traffic_light_state){
	    case 0:
	        P2OUT = LD1_Green + LD2_Red + LDP2_Red;
	        P1OUT = LDP1_Green;

	        if(t >= switch1_green_time) {
	            t = 0;
	            traffic_light_state = 1;
	            switch1_green_time = 8;
	        }

	        break;
	    case 1:
	        P2OUT = LD1_Yellow + LD2_Red + LDP2_Red;
	        P1OUT = LDP1_Red;

	        if(t >= 2){
	            t = 0;
	            traffic_light_state = 2;
	        }

	        break;
	    case 2:
	        P2OUT = LD1_Red + LD2_Green + LDP2_Green;
	        P1OUT = LDP1_Red;

	        if(t >= switch2_green_time){
	            t = 0;
	            traffic_light_state = 3;
	            switch2_green_time = 8;
	        }

	        break;
	    case 3:
	        P2OUT = LD1_Red + LD2_Yellow + LDP2_Red;
	        P1OUT = LDP1_Red;

	        if(t >= 2){
	            t = 0;
	            traffic_light_state = 0;
            }

	        break;
	    }

	    // Incrementar o tempo
	    for(i = 0; i < 80000; i++);
	    t++;
	} while(1);
}

// Interrupção do botão de pedestre
#pragma vector = PORT1_VECTOR
__interrupt void Port1_RTI(void) {
    switch(P1IFG & (BIT1 + BIT2)){
    case BIT1:
        P1IFG &= ~BIT1;

        if(traffic_light_state == 0){
            switch1_green_time = 2;
        }

        break;
    case BIT2:
        P1IFG &= ~BIT2;

        if(traffic_light_state == 2){
            switch2_green_time = 2;
        }

        break;
    default:
        P1IFG &= ~(BIT1 + BIT2);
        break;
    }
}

void start_p1_p2(void){
    /* Inicialização da porta 1
       P1.0 - N.C           - Saída em nível baixo
       P1.1 - SW1           - Entrada como resistor pull-up
       P1.2 - SW2           - Entrada como resistor pull-up
       P1.3 - N.C           - Saída em nível baixo
       P1.4 - N.C           - Saída em nível baixo
       P1.5 - N.C           - Saída em nível baixo
       P1.6 - LDP1_Green    - Saída em nível baixo
       P1.7 - LDP1_Red      - Saída em nível baixo
    */


    P1DIR = ~(BIT1 + BIT2);     // ou P1DIR = 0xF9
    P1OUT = BIT1 + BIT2;        // ou P1OUT = 0x06
    P1REN = BIT1 + BIT2;        // ou P1REN = 0x06
    P1IES = BIT1 + BIT2;        // ou P1IES = 0x06
    P1IFG = 0;
    P1IE = BIT1 + BIT2;         // ou P1IE = 0x06

    /* Inicialização da porta 1
       P2.0 - LD1_Green     - Saída em nível baixo
       P2.1 - LD1_Yellow    - Saída em nível baixo
       P2.2 - LD1_Red       - Saída em nível baixo
       P2.3 - LD2_Green     - Saída em nível baixo
       P2.4 - LD2_Yellow    - Saída em nível baixo
       P2.5 - LD2_Red       - Saída em nível baixo
       P2.6 - LDP2_Green    - Saída em nível baixo
       P2.7 - LDP2_Red      - Saída em nível baixo
    */

    P2SEL = 0;                  // Limpar bits
    P2SEL2 = 0;
    P2DIR = 0xFF;
    P2OUT = 0x00;
}

