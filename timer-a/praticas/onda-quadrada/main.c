/*
    3) Utilize o modo contínuo do contador e os módulos 0, 1 e 2 do Timer1 A para gerar 3 sinais do tipo
    onda quadrada (razão cíclica de 50%), com frequências f0 = 25 Hz, f1 = 50 Hz e f2 = 75 Hz. Os sinais
    devem ser gerados pelos respectivos módulos! Apresente os sinais no osciloscópio
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
        P1.0 - Entrada 1 do Encoder - Resistor de pull-up e interrupção
        P1.1 - Entrada 2 do Encoder - Resistor de pull-up
        P1.X - N.C                  - Saída em nível baixo
    */
    P1DIR = ~(BIT0 + BIT1);
    P1OUT = BIT0 + BIT1;
    P2REN = BIT0 + BIT1;
    P1IES = BIT0;
    P1IFG = 0;
    P2IE = BIT0;

    /*
        P2.x - N.C - Saída em nível baixo:
    */
    P2DIR = 0xFF;
    P2OUT = 0;
}

// Inicializar o Timer0 para geração do sinal PWM
void start_TA0_PWM(void) {
    /*
        CONTADOR:
        Clock:                          ACLK = 32768Hz
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
    TA0CTL = TASSEL0 + MC0;
    TA0CCTL1 = OUTMOD0 + OUTMOD1 + OUTMOD2 + OUT;
    TA0CCR0 = 655;
    TA0CCR1 = 33;
}

// Iniciar o Timer1 para debouncer (Encoder Rotativo)
void start_TA1_Debouncer(void) {
    /*
        TIMER1 - Responsável pelo debounce do encoder

        Tempo de Debounce:      5ms

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
    TA1CCTL0 = CCIE;
    TA1CCR0 = 0;
}

// ---------- INTERRUPÇÕES ----------

// Debouncer
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_RTI(void){
    // 1 - Parar o temporizador
    TA1CTL &= ~MC0;

    // 2- Verificar se P1.0 está em nível baixo
    if ((~P1IN) & BIT0){
        // Verificar o sentido de rotação do passo
        if(P1IN & BIT1){
            if(TA0CCR1 >= 655){
                TA0CCR1 = 655;
            } else {
                // Incrementar a razão cíclica com passo de 0,5%
                TA0CCR1 += 4;
            }
        } else {
            if(TA0CCR1 <= 33) {
                TA0CCR1 = 33;
            } else {
                TA0CCR1 -= 4;
            }
        }
    }

    // 3 - Habilitar a interrupção do encoder novamente
    P1IFG &= ~BIT0;
    P1IE = BIT0;
}

