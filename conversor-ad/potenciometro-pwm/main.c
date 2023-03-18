/*
    Um sistema de geração de sinal PWM possui um potenciômetro para ajuste de razão cíclica de um sinal PWM
    - O potenciômetro conectado em A0, ajusta a razão cíclica do sinal de 5% a 95%
    - A tensão de saída dos potenciômetros deve ser lida a cada 100ms
*/

#include <msp430.h>

void start_micro();
void start_p1_p2();
void start_timerA0_ADC10();
void start_timerA1_PWM();
void start_ADC10();

unsigned int ADC10[4];
unsigned int sum;
unsigned int avarage;
unsigned char i = 0;

int main(){
    start_micro();
    start_p1_p2();
    start_timerA0_ADC10();
    start_timerA1_PWM();
    start_ADC10();
    
    do {

    } while(1);

}

void start_ADC10(){
    /*
        Taxa de amostragem:     200ksps
        Vref:                   Vcc
        Tsample:                
            - Ts:               (Rs + Ri) * Ci * ln(2^11) = 7k * 27p * ln(2^11) ~ 1,44us
            - ADC10OSC:         3,7Mhz a 6,3Mhz
                - No pior caso: tclk = 1 / 6,3M = 158,73ns
                - nº Ciclos:    1,44u / 158,73n = 9,07 (arredonda-se para o maior valor -> 16)
        Buffer:                 N.A
        DTC:                    4 transferências (4 conversões consecutivas)
    */ 
    ADC10CTL0 = ADC10SHT1 + MSC + ADC10ON + ADC10IE;
    ADC10CTL1 = SHS0 + CONSEQ1;

    ADC10AE0 = BIT0;

    ADC10DTC0 = 0;
    ADC10DTC1 = 4; // Transferências a serem realizadas

    ADC10SA = &ADC10[0];
    ADC10CTL0 |= ENC;
}

void start_timerA1_PWM(){
    /*
        Frequência de amostragem (fa) = 20Khz
        Tempo de amostragem (ta) = 1 / fa = 50us

        CONTADOR:
        Clock:              SMCLK ~ 2Mhz
        Fator de Divisão:   1
        Modo:               UP
        Interrupção:        Desabilitada

        MÓDULO 0:
        Função:             Comparação
        TA1CCR0             (50u * 2M / 1) - 1 = 99
        Interrupção:        Desabilitada

        MÓDULO 1:
        Função:             Comparação
        TA1CCR1:            0 (Razão cíclica de 0%)
        Modo saída:         Reset / Set (7)
        Interrupção:        Desabilitada
    */
    TA1CTL = TASSEL1 + MC0;
    TA1CCTL = OUTMOD0 + OUTMOD1 + OUTMOD2;
    TA1CCR0 = 99;
    TA1CCR1 = 0;
}

void start_timerA0_ADC10(){
    /*
        Frequência de amostragem (fa) = 5Khz
        Tempo de amostragem (ta) =  1 / fa = 200us

        CONTADOR:
        Clock:              SMCLK ~ 2Mhz
        Fator de Divisão:   1
        Modo:               UP
        Interrupção:        Desabilitada

        MÓDULO 0:
        Função:             Comparação
        TA0CCR0:            (200u * 2M / 1) - 1 = 399
        Interrupção:        Desabilitade

        MÓDULO 1:
        Função:             Comparação
        TA0CCR1:            199 (Razão cíclica 50%)
        Modo saída:         Reset / Set (7)
        Interrupção:        Desabilitada
    */

    TA0CTL = TASSEL1 + MC0;
    TA0CCTL1 = OUTMOD0 + OUTMOD1 + OUTMOD2;
    TA0CCR0 = 399;
    TA0CCR1 = 199;
}

void start_p1_p2(){
    /*
        PORTA 1:
        P1.x - N.C - Saída em nível baixo
    */
    P1DIR = 0xFF;
    P1OUT = 0;

    /*
        PORTA 2:
        P2.2 - Saída PWM TA1.1 - Mudar função do pino
        P2.x - N.C             - Saída em nível baixo
    */
    P2SEL &= ~(BIT6 + BIT7);
    P2SEL |= BIT2;
    P2DIR = 0xFF:
    P2OUT = 0;
}

void start_micro(){
    WDTCTL = WDTPW | WDTHOLD;

    // Configurações do BCS
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
#pragma VECTOR=ADC10_VECTOR
__interrupt void ADC10_RTI(void) {
    // Parar as conversões consecutivas
    ADC10CTL0 &= ~ENC;

    soma = 0;
    for(i = 0; i < 4; i++) {
        soma += ADC10[i];
    }
    // Fazer divisão por quatro deslocando dois bits a direita
    media = soma >> 2;

    TA1CCR1 = soma / 10;

    // Habilitar novamente a interrupção
    ADC10SA = &ADC10[0];
    ADC10CTL0 |= ENC;
}