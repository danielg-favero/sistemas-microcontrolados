/*
    2) Implemente um controlador para motor de passo usando o kit
    MSP430 Launchpad. O Timer1 A do microcontrolador MSP430G2553
    deve ser configurado para gerar uma interrupção em um intervalo de
    tempo t variável (usar Timer0 A), o qual determinará o tempo de
    mudança do passo do motor, permitindo que a velocidade de giro seja
    alterada. O motor deve ser conectado no módulo de acionamento. O
    microcontrolador, por meio de 4 pinos de I/O (Porta 2) configurados
    como saída, deve acionar uma das bobinas do motor por vez,
    colocando nível alto (3,6V) em uma das entradas do módulo (IN1, IN2,
    IN3, IN4). O giro do motor é dado pelo acionamento sequencial das
    bobinas (IN1→IN2→IN3 →IN4→IN1→IN2....) a cada intervalo de tempo
    t. Inicialmente, faça t igual a 100 ms. Alimente o módulo com +5V da USB do kit (via TP1). Utilize um
    encoder (debouncer via Timer0 A) para aumentar/diminuir a velocidade do motor
*/

#include <msp430.h> 

void start_msp430(void);
void start_p1_p2(void);
void start_timer0_debouncer(void);
void start_timer1_counter(void);

unsigned short int count = 100;

void main(void){
    start_msp430();
    start_p1_p2();
    start_timer0_debouncer();
    start_timer1_counter();

    do {

    } while(1);
}

void start_timer0_debouncer(void){
    /* Tempo debouncer: 9 ms
     *
     * CONTADOR:
     *      - Clock: SMCLK ~ 2 MHz
     *          - FDiv: 1
     *      - Modo: UP
     *      - Int.: desabilitada
     *
     * MODULO 0:
     *      - Funcao: comparacao
     *      - Int.: Habilitada
     *      - Valor para TA0CCR0:
     *          TA0CCR0 = 0;
     *
     */
    TA0CTL = TASSEL1 + MC0;
    TA0CCTL0 = CCIE + CM1;
    TA0CCR0 = 0; // Contador fica parado!
}

void start_timer1_counter(void) {
    /*
            CONTADOR:
            Clock:                          SMCLK = 2MHz
            Fator de divisão:               1
            Modo de contagem:               UP
            Interrução:                     Desabilitado

            MÓDULO 0:
            Função:                         Comparação
            Interrupção:                    Habilitada
            TA1CCR0:                        t * (Fclk / Fdiv) - 1 ~ 24999
    */
    TA1CTL = TASSEL1 + MC0;
    TA1CCTL0 = CCIE;
    TA1CCR0 = 24999;
}

void start_msp430(void){
    WDTCTL = WDTPW | WDTHOLD;   // Para o contador do watchdog timer

    // Configuracoes do BCS
    // MCLK = DCOCLK ~ 16 MHz
    // ACLK = LFXT1CLK = 32768 Hz
    // SMCLK = DCOCLK / 8 ~ 2 MHz

    DCOCTL = CALDCO_16MHZ;    // Freq. Calibrada de 16 MHz
    BCSCTL1 = CALBC1_16MHZ;
    BCSCTL2 = DIVS0 + DIVS1;  // Fator divisao = 8 para SMCLK
    BCSCTL3 = XCAP0 + XCAP1;  // Capacitor do cristal ~12.5 pF

    while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();  // seta o bit GIE - permite geracao de interrupcoes
}

void start_p1_p2(void){
    /*
        P1.0 - Bobina 1 - Saída em nível baixo
        P1.1 - Bobina 2 - Saída em nível baixo
        P1.2 - Bobina 3 - Saída em nível baixo
        P1.3 - Bobina 4 - Saída em nível baixo
     */
     P1DIR = 0xFF;
     P1OUT = 0;

    /*
        P2.0 - Sa do encoder    - Entrada resistor de pull-up com interrupcao por borda de descida.
        P2.1 - Sb do encoder    - Entrada com resistor de pull-up.
        P2.X - N.C.             - Saidas em nivel baixo.
     */
    P2DIR = ~(BIT0 + BIT1);
    P2REN = BIT0 + BIT1;
    P2OUT = BIT0 + BIT1;

    P2IES = BIT0;
    P2IFG = 0;
    P2IE = BIT0;
}

// ---------- INTERRUPÇÕES ----------

// Chave do encoder
#pragma vector=PORT2_VECTOR
__interrupt void PORT2_RTI(void) {
    // Passo 1: Desabilita int. de Sa
    P2IE &= ~BIT0;

    // Passo 2: Inicia Timer0
    TA0CCR0 = 1200;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_RTI(void){
    switch(P1OUT){
        case BIT3:
            P1OUT = BIT2;
            break;
        case BIT2:
            P1OUT = BIT1;
            break;
        case BIT1:
            P1OUT = BIT0;
            break;
        default :
            P1OUT = BIT3;
    }
}

// Debouncer
#pragma vector=TIMER0_A0_VECTOR
__interrupt void  RTI_do_Timer0(void){
    // Passo 3: Parar Timer0
    TA0CCR0 = 0;

    // Passo 4: Verifica Chaves
    if((P2IFG) & BIT0){
        if((~P2IN) & BIT0){
                if((~P2IN) & BIT1) {
                    if(count >= 0 && count <= 6553){
                        count--;
                        TA1CCR0 -= 1250;
                    }
                } else {
                    if(count >= 0 && count <= 6553){
                        count++;
                        TA1CCR0 += 1250;
                    }
                }
            }
        }
        P2IFG &= ~BIT0;
        P2IE = BIT0;
    }
