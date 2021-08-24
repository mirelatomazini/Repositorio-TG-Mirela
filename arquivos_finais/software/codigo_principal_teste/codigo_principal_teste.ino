/*
   codigo baseado no desenvolvido por GameInstance.com 2016-2018 e adaptador por Mirela Tomazini:
   STM32 Digital Oscilloscope
   using the STM32F103C8 MCU and the NT35702 2.4 inch TFT display
   https://www.gameinstance.com/post/80/STM32-Oscilloscope-with-FFT-and-SD-export 
*/

/* a primeira parte do codigo importa as bibliotecas e cria variaveis globais */
#include "adc_proj.h"
#include "dft.h"

int cont                  = 0;
float fpeak               = 1100;
uint8_t time_base         = 6;
float sample_freq         = DT_FS[time_base]*1000;
static const uint8_t CHANNEL = PB0;                   //define o pino em que sera adquirido o sinal do canal 1

static const uint16_t BUFFER_SIZE = 1024;               // bytes - define uma variavel com informacao do tamanho do buffer em bytes
static const float VCC_3_3 = 3.3;                       // volts - define uma variavel com valor de tensao maximo do microcontrolador


uint16_t data16[BUFFER_SIZE];                          // cria um vetor que possui a quantidade de dados igual ao buffer_size 1024


void setup() {
  adc_calibrate(ADC1);                                  //funcao que calibra o ADC
}


void loop() {
  // funcao usada para realizar a aquisicao;
  
  setADCs(time_base, CHANNEL);                                         // chama a funcao que configura o ADC
  DMA_(data16,BUFFER_SIZE);                                            // chama a funcao que configura o DMA 
  dma1_ch1_Active = 1;                               // muda o valor da variavel booleana para habilitar a coleta
  dma_enable(DMA1, DMA_CH1);                         // habilita o canal DMA e inicia a transferencia
  
  while (dma1_ch1_Active) {};                        // aguardando a interrupcao mudar o valor da variavel booleana para encerrar a aquisicao
  dma_disable(DMA1, DMA_CH1);                        // finaliza a aquisição 


  int h;                                         //cria uma variavel inteira 
  for (h=0; h<BUFFER_SIZE; h = h+1){            //cria um laco com o tamanho do buffer para realizar a tranferencia das informacoes         
    Serial.print(data16[h]*VCC_3_3/4096.0);
    Serial.print(" ");
    Serial.println(data16[h]*VCC_3_3/4096.0);

  }                                              // printa os valores adquiridos apos realizar a conversao para valores de tensao
    
  delay(1000);                                   //aguarda 1000 ms para iniciar o processo novamente
  
  fpeak = search_fpeak_initial (data16, sample_freq);
    
  
}
