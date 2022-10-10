# TIMER A

## Sistema básico de Clock (BCS)

### Clocks básicos

#### ACLK (Auxiliary Clock)
- Clock auxiliar
- Baixa frequência

![Diagrama de blocos do clock ACLK](assets/ACLK.png)<br><br>

#### MCLK (Main Clock)
- Clock principal
- Interage diretamente com a CPU

![Diagrama de blocos do clock MCLK](assets/MCLK.png)<br><br>

#### SMCLK (Sub Main Clock)
- Clock secundário
- Alimenta os periféricos

![Diagrama de blocos do clock SMCLK](assets/SMCLK.png)<br><br>
<br>

### Osciladores

#### VLOCLK
- Baixo consumo de energia
- Baixa frequência (10KHz ~ 20KHz)
- Baseado em carga e descarga de capacitor
<br><br>

#### LFXT1CLK
- Usa um cristal de quartzo como referência **(32.768Hz)**
- É possível entrar com sinal de clock externo
<br><br>

#### XT2CLK
- Alta frequência
<br><br>

#### DCOCLK (Digital Configurable Oscilator Clock)
- Oscilador digital configurável
- Clock ajustável
- Geralmente utilizado pela CPU
- Configuração inicial:
    - RSELx = 7
    - DCOx = 3
    - MODx = 0
    - Resultando em uma frequência 0.8MHz ~ 1.5MHz 

![Configuração inicial do DCO](assets/DCO_config.png)

O DCO possui frequências pré-calibradas pelo fabricante. Os valores para essas frequências estão armazenadas em **memória flash de 256 Bytes** que contém as sequintes frequências: *1MHz, 8MHz, 12MHz e 16MHz*. Essas frequências podem ser configuradas pelos registradores **DCOCTL** e **BCSCTL1**. Ex:

```C
// [FREQ] = 1MHZ ou 8MHZ ou 12MHZ ou 16MHZ
// [FREQ] deve ser igual para os dois registradores
DCOCTL = CALDCO_[FREQ];
BCSCTL1 = CALBC1_[FREQ];
```
<br>

### Registradores

#### BCSCTL1 (Basic Clock System Control 1)

![Bits do registrador BCSCTL1](assets/BCSCTL1_bits.png)
> rw-1 = é possível ler e escrever o bit e seu valor inicial é igual a 1

|        	|                                            Descrição                                            	|                    Descrição dos BIT                    	|                    Descrição dos BIT                    	|
|:------:	|:-----------------------------------------------------------------------------------------------:	|:-----------------------------------------:	|:-----------------------------------------:	|
| XT2OFF 	|                                      Ligar ou desligar XT2                                      	|             1 - XT2 Desligado             	|               0 - XT2 Ligado              	|
|   XTS  	|                                   Modo de frequência do LFXT1                                   	|        1 - Modo de alta frequência        	|        0 - Modo de baixa frequência       	|
|  DIVAx 	|                              Fator de divisão para o clock auxiliar                             	| 00 - Divisão por 1<br> 01 - Divisão por 2 	| 10 - Divisão por 4<br> 11 - Divisão por 8 	|
|  RSELx 	| Selecionar a frequência do clock (pré-definido pelo fabricante nos registradores CALBC1_[FREQ]) 	|                                           	|                                           	|
<br><br>

#### BCSCTL2 (Basic Clock System Control 2)
Usado para configurar o clock principal e secundário

![Bits do registrador BCSCTL2](assets/BCSCTL2_bits.png)

|           	|                   Descrição                   	|                    Descrição dos BIT                    	|                                                  Descrição dos BIT                                                  	|
|:---------:	|:---------------------------------------------:	|:-----------------------------------------:	|:-----------------------------------------------------------------------------------------------------:	|
| **SELMx** 	|  Seleciona a fonte do clock principal (MCLK)  	|          00 - DCOCLK<br> 01 - DCOCLK          	| 10 - XT2CLK (Apenas quando o oscilador XT2 está presente no microcontrolador)<br> 11 - LFXT1CLK ou VLOCLK 	|
| **DIVMx** 	|     Fator de divisão para clock principal     	| 00 - Divisão por 1<br> 01 - Divisão por 2 	|                               10 - Divisão por 4<br> 11 - Divisão por 8                               	|
|  **SELS** 	| Seleciona a fonte do clock secundário (SMCLK) 	|                 0 - DCOCLK                	|                1 - LFXT1CLK ou VLOCLK (apenas quando o oscilador XT2 não está presente)               	|
| **DIVSx** 	|    Fator de divisão para o clock secundário   	| 00 - Divisão por 1<br> 01 - Divisão por 2 	|                               10 - Divisão por 4<br> 11 - Divisão por 8                               	|
| **DCOR**  	| Seletor do resistor do DCO                    	| 0 - Resistor interno                      	| 0 - Resistor externo                                                                                  	|
<br>

#### BCSCTL3 (Basic Clock System Control 3)
Usado para configurar as faixas de frequência e as capacitâncias conectadas ao cristal

![Bits do registrador BCSCTL3](assets/BCSCTL3_bits.png)

|             	|                                     Descrição                                    	|                                      Descrição do BIT                                     	|                                              Descrição do BIT                                              	|
|:-----------:	|:--------------------------------------------------------------------------------:	|:-----------------------------------------------------------------------------------------:	|:----------------------------------------------------------------------------------------------------------:	|
|  **XT2Sx**  	|                        Seletor da faixa de frequência XT2                        	| 00 - 0.4MHz - 1MHz do cristal ou ressonador<br> 01 - 1MHz - 3MHz do cristal ou ressonador 	| 10 - 3MHz - 16MHz do cristal ou ressonador<br> 11 - Fonte digital externa do clock primário 0.4MHz - 16MHz 	|
| **LFXT1Sx** 	| Seletor de clock de baixa frequência e selecionar a faixa de frequência do LFXT1 	|                              00 - 32.768Hz<br> 01 - Reservado                             	|                               10 - VLOCLK 11 - Fonte digital de clock externa                              	|
|  **XCAPx**  	|                         Seleção do capacitor do oscilador                        	|                                  00 - ~1pF<br> 01 - ~6pF                                  	|                                         10 - ~10pF<br> 11 - ~12.5pF                                        	|
| **XT2OF**   	| Verifica condição de falha no oscilador XT2                                      	| 0 - Sem condição de falha                                                                 	| 1 - Condição de falha presente                                                                             	|
| **LFXT1OF** 	| Verifica condição de falha no oscilador LFXT1                                    	| 0 - Sem condição de falha                                                                 	| 1 - Condição de falha presente                                                                             	|
> O oscilador LFXT1 não começa com frequência de 32.768Hz no tempo 0. O oscilador aplica pulsos no cristal até que chegue a frequência do cristal
<br><br>

### Exemplo de Configuração

![Exemplo de configuração do sistema de clock básico](assets/basic_clock_config.png)

```C
void start_micro(){
    WDTCTL = WDTPW | WDTHOLD;

    DCOCTL = CALDCO_16MHZ;

    /*
        XT2OFF          1000 0000
        + CALBC1_16MHZ    0000 xxxx
                        1000 xxxx
    */
    BCSCTL1 = XT2OFF + CALBC1_16MHZ;

    // DIVSx = 11 - Divisão por 8
    BCSCTL2 = DIVS0 + DIVS1;

    // XCAPx = 11 ~12.5pF
    BCSCTL3 = XCAP0 + XCAP1;

    // Verificar se o oscilador LFXT1 está na frequência do cristal
    // Executar o loop vazio enquanto LFXT1OF não esteja estável
    // Quando LFXT1OF for estável, ele é limpo pela CPU
    while(BCSCTL3 & LFXT1OF);

    __enable_interrupt();
}
```