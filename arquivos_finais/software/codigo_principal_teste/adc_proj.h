#include <STM32ADC.h>                                   //inclui a biblioteca do STM32 para ADC


static const uint16_t ADC_RESOLUTION = 4096;            // units - define uma variavel com a resolucao do ADC


const uint8_t DT_PRE[]  = {2,         3,        2,        3,          3,        2,        2,        3,       2}; 
const uint8_t DT_SMPR[] = {0,         0,        1,        1,          2,        3,        6,        6,       7};
const float DT_FS[]     = {857143,    642857,   600000,    450000,     346154,   292683,   142857,   107143,  47619};

volatile static bool dma1_ch1_Active;                 // variavel boleana que grava a informacao se a coleta do sinal esta ocorrendo ou nao


void setADCs(uint8_t time_base2, uint8_t CHANNEL_1) {

  //determina em qual dos casos deve ser selecionado o DT_PRE a depender do valor inserido na variavel time_base
  switch (DT_PRE[time_base2]) {
    case 0: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_2); break;
    case 1: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_4); break;
    case 2: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_6); break;
    case 3: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_8); break;
    default: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_8);
  } 
  
  switch (DT_SMPR[time_base2]) {
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

void DMA_(uint16_t *data16){

  dma_init(DMA1);
  dma_attach_interrupt(DMA1, DMA_CH1, DMA1_CH1_Event);      //cria uma interrupcao para a transferencia de dados
  adc_dma_enable(ADC1);                                     //habilita dma do ADC
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_16BITS, data16, DMA_SIZE_16BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));
  // configura a tranferencia pelo DMA
  dma_set_num_transfers(DMA1, DMA_CH1, BUFFER_SIZE);        // configura a quantdade de informacao a ser transferida pelo DMA
  
}
