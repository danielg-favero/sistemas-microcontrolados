#include <msp430.h> 

void start_p1_p2(void);
void start_timer0A(void);
void start_micro(void);

void main(void) {
    start_micro();
    start_p1_p2();
    start_timer0A();

	do {

	} while(1);
}

void start_p1_p2(void) {
    /*
        PORTA 1
        P1.0 - LED VERMELHO         - sa�da em n�vel baixo;
        P1.1 - N.C                  - sa�da em n�vel baixo;
        P1.2 - N.C                  - sa�da em n�vel baixo;
        P1.3 - S2                   - entrada com resistor de pull-up;
        P1.4 - CANAL A DO ENCODER   - entrada com resistor de pull-up e interrup��o;
        P1.5 - CANAL B DO ENCODER   - entrada com resistor de pull-up;
        P1.6 - LED VERDE            - sa�da em n�vel baixo;
        P1.7 - N.C                  - sa�da em n�vel baixo;
    */
    P1DIR = ~(BIT3 + BIT4 + BIT5);
    P1REN = BIT3 + BIT4 + BIT5;
    P1OUT = BIT3 + BIT4 + BIT5;
    P1IES = BIT4;
    P1IFG = 0;
    P1IE = BIT4;

    /*
        PORTA 2
        P1.0 - N.C                  - sa�da em n�vel baixo;
        P1.1 - N.C                  - sa�da em n�vel baixo;
        P1.2 - N.C                  - sa�da em n�vel baixo;
        P1.3 - N.C                  - sa�da em n�vel baixo;
        P1.4 - N.C                  - sa�da em n�vel baixo;
        P1.5 - N.C                  - sa�da em n�vel baixo;
        P1.6 - XIN                  - fun��o XIN;
        P1.7 - XOUT                 - fun��o XOUT;
    */
    P2DIR = 0xFF;
    P2OUT = 0;
}

void start_timer0A(void) {
    /*
        debounce_time = 25ms

        BLOCO CONTADOR
        - Clock:                  SMCLK ~ 2MHZ
        - Modo contagem:          UP (Inicialmente parado)
        - Interrp��o contador:    N�o utilizado

        M�DULO 0
        - Fun��o:                 Nativa = Compara��o
        - TA0CCR0:                (tempo * SMCLK) / fator_divis�o - 1 = 0.025 * 2000000 / 1 - 1 = 49999
        - Interrup��o:            Habilitada
    */
    TA0CTL = TASSEL1;
    TA0CCTL0 = CCIE;
    TA0CCR0 = 49999;
}

void start_micro(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configura��es do BCS
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

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_RTI(void) {
    // 1 - Desabilitar interrup��o da entrada
    P1IE &= ~BIT4;

    // 2 - Inicia temporizador (sem limpar TASSEL1)
    TA0CTL |= MC0;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void MODULE0_TIMER0_RTI(void) {
    // Parar o temporizador limpando o bit MC0
    TA0CTL &= ~MC0;

    // Verificar o estado da entrada
    // Ser� acionado em borda de descida
    if(~P1IN & BIT4){
        if(P1IN & BIT5){
            P1OUT ^= BIT0; // Alterar Led Vermelho
        } else {
            P1OUT ^= BIT6; // Alterar Led Verde
        }
    }

    P1IFG &= ~BIT4;
    P1IE |= BIT4;
}








