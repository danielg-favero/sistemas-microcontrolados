#include <msp430.h>
#include <math.h>

void start_micro(void);
void start_p1_p2(void);
void start_usci_b0_spi(void);
void fill_dac(void);
void start_timerA0_spi(void);

unsigned int DAC[100];
unsigned char i = 0;

int main(){
    start_micro();
    start_p1_p2();
    start_usci_b0_spi();

    fill_dac();

    start_timerA0_spi();

    do {

    } while(1);
}

void fill_dac(void){
    unsigned char k = 0;
    float sin_value = 0.0;

    for(k = 0; i < 100; i++){
        sin_value = 2047.0 * (1.0 + sin(6.28318531 * (float) i / 100.0 ));
        DAC[k] = (unsigned int) sin_value;
    }
}

void start_timerA0_spi(void){
    /*
        CONTADOR:
        Clock:              SMCLK ~ 2MHz
        Modo de contagem:   UP
        Interrupção:        Desabilitado

        MÓDULO 0:
        Função:             Comparação
        TA0CCR0:            (1ms * 2MHz / 1 - 1) = 1999
        Interrupção:        Habilitado
    */
    TA0CTL = TASSEL1 + MC0;
    TA0CCTL0 = CCIE;
    TA0CCR0 = 1999;
}

void start_usci_b0_spi(void){
    /*
        Clock:                      SMCLK ~ 2MHz
        Frequência do BitClock:     F_brclock / UCBRx = 2MHz / 1 = 2MHz
    */

    // 1 - Colocar a interface em estado de reset
    UCB0CTL1 |= UCSWRST;

    // 2 - Configurar registradores UCB0CTL0 e UCB0CTL1
    UCB0CTL0 = UCMSB + UCMST + UCSYNC;
    UCB0CTL1 |= UCSSEL1;

    UCB0BR0 = 2;
    UCB0BR1 = 0;

    // 3 - Configurar portas
    P1SEL |= BIT5 + BIT7;
    P1SEL2 |= BIT5 + BIT7;

    // 4 - tirar a interface do estado de reset
    UCB0CTL1 &= ~UCSWRST;
}

void start_p1_p2(void){
    /*
        P1.x - N.A - Saída em nível baixo
    */
    P1DIR = 0xFF;
    P1OUT = BIT0;

    /*
        P2.x - N.A - Saída em nível baixo
    */
    P2DIR = 0xFF;
    P2OUT = 0;
}

void start_micro(void){
    WDTCTL = WDTPW | WDTHOLD;

    // Configurações do BCS (Basic Clock System)
    // MCLK = DCOCLK ~ 16MHz
    // ACLK = LFXT1CLK = 32768Hz
    // SMCLK = DCOCLK / 8 ~ 2MHz
    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS0 + DIVS1;
    BCSCTL3 = XCAP0 + XCAP1;

    while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();
}

// ---------- INTERRUPÇÕES ----------

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_RTI(){
    P1OUT &= ~BIT0;

    if(i >= 100){
        i = 0;
    } else {
        i++;
    }

    // A cada interrupção é colocado uma palavra diferente no buffer de transmissão
    // 1 - Mandar os bits MAIS significativos
    UCB0TXBUF = (unsigned char) DAC[i] >> 8;
    while(UCB0STAT & UCBUSY);

    // 2 - Mandar os bits MENOS significativos
    UCB0TXBUF = (unsigned char) DAC[i];
    while(UCB0STAT & UCBUSY);

    P1OUT |= BIT0;
}