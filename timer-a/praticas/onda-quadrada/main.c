/*
    3) Utilize o modo contínuo do contador e os módulos 0, 1 e 2 do Timer1 A para gerar 3 sinais do tipo
    onda quadrada (razão cíclica de 50%), com frequências f0 = 25 Hz, f1 = 50 Hz e f2 = 75 Hz. Os sinais
    devem ser gerados pelos respectivos módulos! Apresente os sinais no osciloscópio.
*/

#include <msp430.h> 

void start_msp430(void);
void start_p1_p2(void);
void start_timer1A_PWM(void);

int main(void) {
    start_msp430();
    start_p1_p2();
    start_timer1A_PWM();

    do {

    } while(1);
}

void start_msp430(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Configurações do BCS
    // MCLK = DCOCLK ~ 16MHz
    // ACLK = LFXT1CLK = 32768 Hz
    // SMCLK = DCOCLK / 8 ~ 2MHz

    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS0 + DIVS1;
    BCSCTL3 = XCAP0 + XCAP1;

    while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();
}

void start_p1_p2(void) {
    /*
        P1.x - N.C - Saídas em nível baixo
    */
    P1DIR = 0xFF;
    P1OUT = 0;

    /*
        P2.x - N.C - Saídas em nível baixo
    */
    P2SEL |= (BIT0 + BIT1 + BIT4);
    P2SEL2 &= ~(BIT0 + BIT1 + BIT4);
    P2DIR = 0xFF;
    P2OUT = 0;
}

void start_timer1A_PWM (void) {
    /*
        CONTADOR:
        Clock:                          SMCLK = 2MHz
        Fator de divisão:               8
        Modo de contagem:               Contínuo
        Interrução:                     Habilitada

        MÓDULO 0:
        Função:                         Comparação
        Interrupção:                    Habilitada
        Frequência do PWM (Fpwm):       25Hz
        Período do PWM (Tpwm):          1 / 2 * Fpwm = 20ms
        TA0CCR0:                        Tpwm * (Fclk / Fdiv) - 1 ~ 4999

        MÓDULO 1:
        Função:                         Comparação
        Interrupção:                    Habilitada
        Frequência do PWM (Fpwm):       50Hz
        Período do PWM (Tpwm):          1 / 2 * Fpwm = 10ms
        TA0CCR1:                        Tpwm * (Fclk / Fdiv) - 1 ~ 2499

        MÓDULO 2:
        Função:                         Comparação
        Interrupção:                    Habilitada
        Frequência do PWM (Fpwm):       50Hz
        Período do PWM (Tpwm):          1 / 2 * Fpwm = 6,66ms
        TA0CCR2:                        Tpwm * (Fclk / Fdiv) - 1 ~ 1666
    */
    TA1CTL = TASSEL_2 + ID_3 + MC_2 + TAIE;

    TA1CCTL0 = CCIE;
    TA1CCTL1 = CCIE;
    TA1CCTL2 = CCIE;

    TA1CCR0 = 4999; // 25Hz
    TA1CCR1 = 2499; // 50Hz
    TA1CCR2 = 1666; // 75Hz
}

// ---------- INTERRUPÇÕES ----------

#pragma vector=TIMER1_A0_VECTOR
__interrupt void RTI_M0_Timer1(void){
    //TA1CCR0
    TA1CCR0 += 4999;
    P2OUT ^= BIT5;
}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void RTI_M1_M2_Timer1(void){
    switch(TA1IV) {
        // TA1CCR1
        case 2: { 
            TA1CCR1 += 2499;
            P2OUT ^= BIT2;
            break;
        }
        // TA1CCR2
        case 4:{ 
            TA1CCR2 += 1666;
            P2OUT ^= BIT3;
        }
        // Overflow - TA1IFG
        case 10: {
            break;
        }
    }
}
