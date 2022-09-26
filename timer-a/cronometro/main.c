#include <msp430.h> 

void start_p1_p2(void);
void start_micro(void);
void start_timer0A(void);
void start_timer1A_debouncer(void);

unsigned long int number_cycles = 0;

void main(void) {
    start_p1_p2();
	start_micro();
	start_timer0A();
	
	do {

	} while(1)
}

void start_timer0A() {
    /*
         TIMER_0 A - CAPTURA

         BLOCO CONTADOR
         - Clock                = ACLK (32768Hz)
         - Fator de divisão     = 1
         - Modo de contagem     = CONTINUO
         - Interrupção contador = HABILITADA

         MÓDULO 0
         - Função               = CAPTURA
         - Modo Captura         = BORDAS DE SUBIDA E DESCIDA
         - Interrupção módulo 0 = HABILITADA
    */
    TA0CTL = TASSEL0 + MC1 + TAIE;
    TA0CCTL0 = CM0 + CM1 + CCIS1 + CAP + CCIE;
}

void start_timer1A_debouncer() {
    /*
         DEBOUNCER DE S2 - td = 20ms

         BLOCO CONTADOR
         - Clock                = SMCLK (2MHz)
         - Fator de divisão     = 1
         - Modo de contagem     = UP (INICIALMENTE PARADO)
         - Interrupção contador = DESABILITADA

         MÓDULO 0
         - Função               = COMPARAÇÃO
         - Valor de TA1CCR0     = (Clock * 0,02 - 1) = 39999
         - Modo Captura         = BORDAS DE SUBIDA E DESCIDA
         - Interrupção módulo 0 = HABILITADA
    */
    TA1CTL = TASSEL1;
    TA1CCTL0 = CCIE;
    TA1CCR0 = 39999;
}

void start_micro() {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configurações do BCS
    // MCLK = DCOCLK ~ 16 MHz
    // ACLK = LFXT1CLK = 32768 Hz
    // SMCLK = DCOCLK / 8 ~ 2 MHz
    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS0 + DIVS1;
    BCSCTL3 = XCAP0 + XCAP1;

    while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();
}

void start_p1_p2() {
    /*
        P1.0 - LED VERMELHO - SAÍDA EM NÍVEL BAIXO
        P1.3 - ENTRADA      - COM RESISTOR DE PULLUP E INTERRUPÇÃO POR BORDA DE DESCIDA
        P1.X - N.C          - SAÍDAS EM NÍVEL BAIXO
    */
    P1DIR = ~BIT3;
    P1REN = BIT3;
    P1OUT = BIT3;
    P1IES = BIT3;
    P1IFG = 0;
    P1IE = BIT3;

    /*
         P2.X - N.C         - SAÍDAS EM NÍVEL BAIXO
         18, 90             - MANTER FUNÇÕES NATIVAS (XIN / XOUT)
    */
    P2DIR = 0xFF;
    P2OUT = 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void RTI_P1(){
    P1IE &= ~BIT3;
    TA1CTL |= MC0;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void RTI_M0_TA1(){
    TA1CTL &= ~MC0;

    if((~P1IN) & BIT3){
        TA0CCTL0 ^= CCIS0;
    }

    P1IFG &= ~BIT3;
    P1IE |= BIT3;
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void RTI_TA0_M12C(){
    switch(TA0IV){
    case 2:
        break;
    case 4:
        break;
    case 10:
        number_cycles++;
        break;
    }
}
