#include <msp430.h> 

void start_p1_p2(void);
void start_timer0_module0(void);
void start_micro(void);

void main(void) {
    start_micro();
    start_p1_p2();
    start_timer0_module0();

	do {
        _BIS_SR(LPM0_bits + GIE);
	} while(1);
}

void start_p1_p2(void) {
    /*
        P1.0 - Led Vermelho - Saída Digital em nível baixo
        P1.3 - Chave S2     - Entrada digital com resistor pull-up e interrupção habilitada por borda de descida
        P1.X - N.C          - Saídas em nível baixo
    */
    P1DIR = BIT0 + BIT1 + BIT2 + BIT4 + BIT5 + BIT6 + BIT7; // ou.. P1DIR = ~BIT3;
    P1REN = BIT3;
    P1OUT = BIT3;
    P1IES = BIT3; // Interrupção por borda de descida
    P1IFG = 0x00;
    P1IE = BIT3;

    /*
        P2.X - N.C - Saídas em nível baixo
    */
    P1DIR = 0xFF;
    P2OUT = 0;
}

void start_timer0_module0(void) {
    /*
        Tempo: 5ms

        CONTADOR:
            Clock: SMCLK (2MHZ)
            Modo: UP
            Fator de divisão: 1

        MÓDULO 0:
            Função: Comparação (Nativa no MSP430)
            TA0CCR0 = (tempo * frequência_clock / fator_divisão) - 1
                    = 0.005 * 2M / 1 - 1
                    = 9999 < 65535 (valor máximo para TA0CCR0)
            Interrupção: Habilitada
    */
    TA0CTL = TASSEL1;
    TA0CTL0 = CCIE;
    TA0CCR0 = 9999

    /*
        ou...
        TA0CTL = TASSEL1 + MC0;
        TA0CTL0 = CCIE;
        TA0CCR0 = 0

        Quando atribuímos TA0CTL = MC0, o contador do módulo 0 começa a trabalhar,
        mas queremos que isso ocorra apenas quando é pressionado a chave, por isso
        atribuimos TA0CCR0 = 0, para que não o contador não comece contar
    */
}


void start_micro(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configurações do BCS (Sistema Básico de Clock)
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

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_RTI(void) {
    // Limpar flag de interrupção
    PIE &= ~BIT3;

    // Iniciar o temporizador
    TA0CTL |= MC0;
    /*
        ou...
        TA0CCR0 = 9999;
    */
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void MODULE0_TIMER0_RTI(void){
    // Para o temporizador
    TA0CTLz &= ~MC0;
    /*
        ou...
        TA0CCR0 = 0;
    */

    // Verificar se a entrada está em nível baixo
    if ((~PIN) & BIT3){
        // Realizar a ação da chave - inverter o estado do Led Vermelho
        P1OUT ^= BIT0;
    }

    // Limpar a flag de interrupção da entrada
    P1IFG &= ~BIT3;

    // Habilitar a interrupção de P1.3
    P1IE = BIT3;
}