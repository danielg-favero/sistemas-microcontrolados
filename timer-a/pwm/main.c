/*
    O Timer0 A do microcontrolador MSP430G2553 deve ser configurado no
    Modo PWM para controlar a luminosidade do LED Verde. Um sinal PWM com
    frequência de 100 Hz e com largura de pulso ajustável e 0 a 100% deve ser
    gerado. Cada vez que S2 for pressionada a razão cíclica deve ser aumentada
    de 25%, partindo de 0 a 100%.
*/

#include <msp430.h> 

void start_msp430(void);
void start_p1_p2(void);
void start_timer0_PWM(void);
void start_timerA1_debouncer(void);

void main(void) {
    start_msp430();
    start_p1_p2();
    start_timerA0_PWM();
    start_timerA1_debouncer();

    do{
        _BIS_SR(LPM0_bits + GIE);
    } while(1);
}

void start_timerA1_debouncer(void){
    /*
        TIMER 1 - responsável pelo debounce da chave S2

        Tempo de debounce:      3 ms
    
        CONTADOR:
        Clock:                  SMCLK ~ 2 MHz
        Fator de divisão:       1
        Modo de contagem:       UP
        Interrupções:           Desabilitada.
    
        MODULO 0:
        Funcao:                 comparacao
        Interrupções:           Habilitada
        TA1CCR0:                inicialmente com ZERO
    */
    TA1CTL = TASSEL1 + MC0;
    TA1CCTL0 = CCIE;
    TA1CCR0 = 0;
}

void start_timer0_PWM(void){
    /*
        TIMER 0 - responsável por gerar PWM que controla LED

        Frequência: 100Hz
        Período:    10ms
        Razão Cíclica: inicialmente 0%
    
        CONTADOR:
        Clock:                  ACLK = 32768 Hz
        Fator de divisão:       1
        Interrupção:            Desabilitada
        Modo de contagem:       UP

        MODULO 0 - Responsável pela frequência do sinal PWM:
        Funcao:                 Comparação
        Intettupção:            Desabilitada
        TA0CCR0:                (32768 * (0,01 / 1)) -1 = 327

        MODULO 1 - Responsável pela geração do sinal PWM:
        Funcao:                 Comparação
        Interrupções:           Desabilitada
        Modo de saída:          7 - reset / set
        TA0CCR1:                0
    */
    TA0CTL = TASSEL0 + MC0;
    TA0CCTL1 = OUTMOD0 + OUTMOD1 + OUTMOD2 + OUT;
    TA0CCR0 = 327;
    TA0CCR1 = 0;
}


void start_p1_p2(void){
    /*
        P1.0 - LED VM   - Saida em nivel baixo
        P1.3 - S2       - Entrada com resistor de pull-up e interrupção por borda de descida.
        P1.6 - TA0.1    - Mudar função para saída de sinal PWM
    */
    P1DIR = ~BIT3;
    P1REN = BIT3;
    P1OUT = BIT3;

    P1SEL |= BIT6;

    P1IES = BIT3;
    P1IFG = 0;
    P1IE = BIT3;

    /*
        P2.X - N.C.     - Saída em nivel baixo.
    */
    P2DIR = 0xFF;
    P2OUT = 0;
}

void start_msp430(void) {
    WDTCTL = WDTPW | WDTHOLD;

    // Configuracoes do BCS
    // MCLK = DCOCLK ~ 16 MHz
    // ACLK = LFXT1CLK = 32768 Hz
    // SMCLK = DCOCLK / 8 ~ 2 MHz

    DCOCTL = CALDCO_16MHZ;    // Frequência calibrada para 16 MHz
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS0 + DIVS1;  // Fator de divisão = 8 para SMCLK
    BCSCTL3 = XCAP0 + XCAP1;  // Capacitor do cristal ~12.5 pF

    while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();
}

// ---------- INTERRUPÇÕES ----------

// RTI da Porta 1 - Chave S2
#pragma vector=PORT1_VECTOR
__interrupt void RTI_da_Porta_1(void){

    // Passo 1: desabilita interrupção
    P1IE &= ~BIT3;

    // Passo 2: inicia TIMER 1 para debounce
    TA1CCR0 = 6000; // Para t ~ 3 ms
}

// RTI do Modulo 0 do Timer 1
#pragma vector=TIMER1_A0_VECTOR
__interrupt void RTI_do_M0_do_Timer1(void){

    // Passo 3: Parar o Timer
    TA1CCR0 = 0;

    // Passo 4: Verifica se S2 foi pressionada
    if( (~P1IN) & BIT3 ){
        if(TA0CCR1 >= 328){
            // Resetar a razão cíclica do sinal PWM
            TA0CCR1 = 0;
        }else{
            // Aumentar em 25% da razão cíclica do Sinal
            TA0CCR1 += 82;
        }
    }

    // Passo 5: habilita novamente a interrupção da chave S2
    P1IFG &= ~BIT3;
    P1IE = BIT3;
}
