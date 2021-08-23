/*
   codigo baseado no desenvolvido por GameInstance.com 2016-2018 e adaptador por Mirela Tomazini:
   STM32 Digital Oscilloscope
   using the STM32F103C8 MCU and the NT35702 2.4 inch TFT display
   https://www.gameinstance.com/post/80/STM32-Oscilloscope-with-FFT-and-SD-export 
*/

/* a primeira parte do codigo importa as bibliotecas e cria variaveis globais */
#include <STM32ADC.h>                                   //inclui a biblioteca do STM32 para ADC
#include "adc_proj.h"
#include "dft.h"

int cont = 0;
float fpeak = 1100;
float sample_freq=2571000;
static const uint8_t CHANNEL_1 = PB0;                   //define o pino em que sera adquirido o sinal do canal 1

static const uint16_t ADC_RESOLUTION = 4096;            // units - define uma variavel com a resolucao do ADC
static const uint16_t BUFFER_SIZE = 1024;               // bytes - define uma variavel com informacao do tamanho do buffer em bytes
static const float VCC_3_3 = 3.3;                       // volts - define uma variavel com valor de tensao maximo do microcontrolador

const uint8_t DT_PRE[]  = {0,       0,     0,     0,     0,     0,     0,     0,     1}; 
const uint8_t DT_SMPR[] = {0,       1,     2,     3,     4,     5,     6,     7,     7};
const float DT_FS[]     = {2571,    1800,  1384,  878,   667,   529,   429,   143,   71.4};
uint8_t time_base = 7;

//as linhas acima determinam vetores responsaveis por alterar os presets utilizados para velocidade de aquisicao de acordo com o valor de time_base
//as diferentes configuracoes de valores irao refletir em diferentes frequencias apresentadars no vetor DT_FS

uint16_t data16[BUFFER_SIZE];                          // cria um vetor que possui a quantidade de dados igual ao buffer_size 1024

volatile static bool dma1_ch1_Active;                 // variavel boleana que grava a informacao se a coleta do sinal esta ocorrendo ou nao

void setADCs() {
  //determina em qual dos casos deve ser selecionado o DT_PRE a depender do valor inserido na variavel time_base
  switch (DT_PRE[time_base]) {
    case 0: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_2); break;
    case 1: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_4); break;
    case 2: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_6); break;
    case 3: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_8); break;
    default: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_8);
  } 
  
  switch (DT_SMPR[time_base]) {
    // determina em qual dos casos deve ser selecionado o DT_SMPR a depender do valor inserido na variavel time_base
    case 0: adc_set_sample_rate(ADC1, ADC_SMPR_1_5); break;
    case 1: adc_set_sample_rate(ADC1, ADC_SMPR_7_5); break;
    case 2: adc_set_sample_rate(ADC1, ADC_SMPR_13_5); break;
    case 3: adc_set_sample_rate(ADC1, ADC_SMPR_28_5); break;
    case 4: adc_set_sample_rate(ADC1, ADC_SMPR_41_5); break;
    case 5: adc_set_sample_rate(ADC1, ADC_SMPR_55_5); break;
    case 6: adc_set_sample_rate(ADC1, ADC_SMPR_71_5); break;
    case 7: adc_set_sample_rate(ADC1, ADC_SMPR_239_5); break;
    default: adc_set_sample_rate(ADC1, ADC_SMPR_239_5);
  }
  
  adc_set_reg_seqlen(ADC1, 1);
  ADC1->regs->SQR3 = PIN_MAP[CHANNEL_1].adc_channel;
  ADC1->regs->CR2 |= ADC_CR2_CONT; // | ADC_CR2_DMA;    // set continuous mode and DMA
  ADC1->regs->CR2 |= ADC_CR2_SWSTART;
                                                        // realiza as configuracoes finais necessarias para que se inicie a aquisicao de sinal
}

static void DMA1_CH1_Event() {
  dma1_ch1_Active = 0;                                  //variavel que altera o estado para 0, parando a coleta
}

void adc_dma_enable(const adc_dev * dev) {
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 1);
}

void setup() {
  adc_calibrate(ADC1);                                  //funcao que calibra o ADC
}

void DMA_(){

  dma_init(DMA1);
  dma_attach_interrupt(DMA1, DMA_CH1, DMA1_CH1_Event);      //cria uma interrupcao para a transferencia de dados
  adc_dma_enable(ADC1);                                     //habilita dma do ADC
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_16BITS, data16, DMA_SIZE_16BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));
  // configura a tranferencia pelo DMA
  dma_set_num_transfers(DMA1, DMA_CH1, BUFFER_SIZE);        // configura a quantdade de informacao a ser transferida pelo DMA
  
}

void loop() {
  // funcao usada para realizar a aquisicao;
  
  setADCs();                                         // chama a funcao que configura o ADC
  DMA_();                                            // chama a funcao que configura o DMA 
  dma1_ch1_Active = 1;                               // muda o valor da variavel booleana para habilitar a coleta
  dma_enable(DMA1, DMA_CH1);                         // habilita o canal DMA e inicia a transferencia
  
  while (dma1_ch1_Active) {};                        // aguardando a interrupcao mudar o valor da variavel booleana para encerrar a aquisicao
  dma_disable(DMA1, DMA_CH1);                        // finaliza a aquisição 


  int h;                                         //cria uma variavel inteira 
  for (h=0; h<=BUFFER_SIZE; h = h+1){            //cria um laco com o tamanho do buffer para realizar a tranferencia das informacoes         
    //Serial.println(data16[h]*VCC_3_3/4096.0);
  }                                              // printa os valores adquiridos apos realizar a conversao para valores de tensao
    
  delay(1000);                                   //aguarda 1000 ms para iniciar o processo novamente
  
  fpeak = search_fpeak(data16,fpeak, sample_freq);
  //Serial.println(fpeak);
    
  
}
