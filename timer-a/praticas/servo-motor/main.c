/*
    1) Use o kit MSP430 Launchpad para controlar um servo motor. O Timer0 A do
    microcontrolador MSP430G2553 deve ser configurado para gerar um sinal
    PWM com frequência de 50 Hz e razão cíclica entre 5% e 10% (largura de pulso
    entre 1 ms e 2 ms), a qual será ajustada através de um encoder rotativo (com
    debouncer via Timer1 A). A cada passo do encoder no sentido horário, a razão
    cíclica deve ser incrementada de 0,5%, partindo de 5% até atingir 10%. No
    sentido anti-horário, deve haver decremento de 0,5%. O servo motor deve ser
    alimentado com 5V obtido na USB do kit (via TP1), sendo conectado da seguinte
    forma: 5V da USB no fio vermelho, PWM no branco e GND no preto.
*/

#include <msp430.h> 

void start_msp430(void);
void start_p1_p2(void);
void start_TA0_PWM(void);
void start_TA1_Debouncer(void);

int main(void) {
    start_msp430();
    start_p1_p2();
    start_TA0_PWM();
    start_TA1_Debouncer();

    do {

    } while(1);
}

void start_msp430(void) {
    WDTCTL = WDTPW | WDTHOLD;

    /*
       OSCILADORES:
       VLO:                 Não utilizado
       LXFT1CLK:            32.768 Hz
       DCOCLK:              ~ 16 MHz (via dados de calibrado do fab.)

       SAÍDAS DE CLOCK:
       ACLK = LFXT1CLK:     32.768 Hz
       MCLK = DCOCLK:       ~ 16 MHz
       SMCLK = DCOCLK/8:    ~ 2 MHz
    */
    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS0 + DIVS1;
    BCSCTL3 = XCAP0 + XCAP1;

    while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();
}

void start_p1_p2(void) {
    /*
        P2.0 - Entrada 1 do Encoder - Resistor de pull-up e interrupção
        P2.1 - Entrada 2 do Encoder - Resistor de pull-up
        P2.X - N.C                  - Saída em nível baixo
    */
    P2DIR = ~(BIT0 + BIT1);
    P2OUT = BIT0 + BIT1;
    P2REN = BIT0 + BIT1;
    P2IES = BIT0;
    P2IFG = 0;
    P2IE = BIT0;

    P2SEL = BIT6;
    P2SEL2 = 0;

    /*
        P1.0 - LED VERMELHO - Saída em nível baixo
        P1.x - N.C          - Saída em nível baixo
    */
    P1SEL |= (BIT1 + BIT2);
    P1SEL2 &= ~(BIT1 + BIT2);
    P1DIR = 0xFF;
    P1OUT = 0;
}

// Inicializar o Timer0 para geração do sinal PWM
void start_TA0_PWM(void) {
    /*
        CONTADOR:
        Clock:                          ACLK = 32768
        Fator de divisão:               1
        Modo de contagem:               UP
        Interrução:                     Desabilitado

        MÓDULO 0 - Responsável pela frequência do sinal PWM:
        Função:                         Comparação
        Interrupção:                    Desabilitada
        Frequência do PWM (Fpwm):       50Hz
        Período do PWM (Tpwm):          1 / Fpwm = 20ms
        TA0CCR0:                        Tpwm * (Fclk / Fdiv) - 1 ~ 655

        MÓDULO 1 - Responsável pela geração do sinal PWM:
        Função:                         Comparação
        Interrupção:                    Desabilitada
        Modo de saída:                  7 - Reset / Set
        TA0CCR1:                        33 -> Inicial de 5% de razão cíclica
    */
    TA0CTL = TASSEL1 + MC0;
    TA0CCTL1 = OUTMOD0 + OUTMOD1 + OUTMOD2 + OUT;
    TA0CCR0 = 39999;
    TA0CCR1 = 1999;
}

// Iniciar o Timer1 para debouncer (Encoder Rotativo)
void start_TA1_Debouncer(void) {
    /*
        TIMER1 - Responsável pelo debounce do encoder

        Tempo de Debounce:      10ms

        CONTADOR:
        Clock:                  SMCLK ~ 2MHz
        Fator de divisão:       1
        Modo de contagem:       UP
        Interrupção:            Habilitado

        MÓDULO 0:
        Função:                 Comparação
        Interrupção:            Habilitada
    */
    TA1CTL = TASSEL1 + MC0;
    TA1CCTL0 = CCIE + CM1;
    TA1CCR0 = 0;
}

// ---------- INTERRUPÇÕES ----------

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_RTI(void){
    P2IE &= ~BIT0;
    TA1CCR0 = 1200;
}

// Debouncer
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_RTI(void){
    // 1 - Parar o temporizador
    TA1CCR0 = 0;

    // 2- Verificar se P2.0 está em nível baixo
    if (P2IFG & BIT0){
        if ((~P2IN) & BIT0){
            // Verificar o sentido de rotação do passo
            if((~P2IN) & BIT1){
                P1OUT ^= BIT6;
                if(TA0CCR1 >= 3999){
                    TA0CCR1 = 3999;
                } else {
                    // Incrementar a razão cíclica com passo de 0,5%
                    TA0CCR1 += 199;
                }
            } else {
                P1OUT ^= BIT0;
                if(TA0CCR1 <= 1999) {
                    TA0CCR1 = 1999;
                } else {
                    TA0CCR1 -= 199;
                }
            }
        }

        // 3 - Habilitar a interrupção do encoder novamente
        P2IFG &= ~BIT0;
        P2IE = BIT0;
    }
}

