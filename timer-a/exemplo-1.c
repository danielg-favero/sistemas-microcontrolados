/*
    Timer A configurado para gerar interrupções a cada 1s e na RTI mudar o estado do LED Vermelho (P1.0),
    usando como fonte de clock o sinal ACLK, proveniente de LFXT1 (cristal de 32.768Hz)
*/

#include <msp430.h> 

void start_p1_p2(void);
void start_TA0(void);
void start_micro(void);

void main(void) {
    start_micro();
    start_p1_p2();
    start_TA0();

	do {
        // Não é preciso fazer nada, pois a interrupção irá realizar toda a lógica
	} while(1);
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

void start_p1_p2(void){
    // Todos os pinos da porta 1 como saída
    P1DIR = 0xFF;
    P1OUT = 0;

    // Todos os pinos da porta 2 como saída, exceto P2.6 e P2.7
    P2DIR = 0xFF;
    P2OUT = 0;
}

void start_TA0(void) {
    /*
        TASSELx = 01 -> Selecionar a fonte de clock auxiliar ACLK
        MCx     = 01 -> Selecionar modo de contagem UP
        IDx     = 00 -> Fator de divisão 1
    */
    TA0CTL = TASSEL0 + MC0;

    /*
        CCIE = 1 -> Habilitar interrupção do modo de captura / comparação
    */
    TA0CCTL0 = CCIE;

    /*
        TAxCCRx = (tempo * (Fclk / Fdiv) - 1)
        TAxCCRx = (1 * (32768 / 1) - 1)
        TAxCCRx = 32767
    */
    TA0CCR0 = 32767 
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0CCRO_RTI(void){
    // Alternar o estado do LED para ligado / desligado
    P1OUT ^= BIT0;
}