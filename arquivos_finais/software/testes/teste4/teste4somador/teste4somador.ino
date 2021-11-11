/*
   codigo baseado no desenvolvido por GameInstance.com 2016-2018 e adaptador por Mirela Tomazini:
   STM32 Digital Oscilloscope
   using the STM32F103C8 MCU and the NT35702 2.4 inch TFT display
   https://www.gameinstance.com/post/80/STM32-Oscilloscope-with-FFT-and-SD-export 
*/

/* a primeira parte do codigo importa as bibliotecas e cria variaveis globais */

#define FREQ0 200000
#define FREQ1 20000
#define BUFFER_SIZE 60 
#define VCC_3_3 3.3

#include "adc_proj.h"
#include "dft.h"

int flag_inicial          = 1;
int cont                  = 0;
float fpeak1              = 0;
float fpeak2              = 0;
float ampl1              = 0;
float ampl2              = 0;
uint8_t time_base         = 2;
float sample_freq         = DT_FS[time_base];
static const uint8_t CHANNEL = PB0;                   //define o pino em que sera adquirido o sinal do canal 1
uint16_t data16[BUFFER_SIZE];                          // cria um vetor que possui a quantidade de dados igual ao buffer_size 1024


void setup() {
  adc_calibrate(ADC1);                                  //funcao que calibra o ADC
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
}


void loop() {
  // funcao usada para realizar a aquisicao;
  
  setADCs(time_base, CHANNEL);                                         // chama a funcao que configura o ADC
  DMA_(data16);                                            // chama a funcao que configura o DMA 
  dma1_ch1_Active = 1;                               // muda o valor da variavel booleana para habilitar a coleta
  dma_enable(DMA1, DMA_CH1);                         // habilita o canal DMA e inicia a transferencia
  
  while (dma1_ch1_Active) {};                        // aguardando a interrupcao mudar o valor da variavel booleana para encerrar a aquisicao
  dma_disable(DMA1, DMA_CH1);                        // finaliza a aquisição 

 
  
  if(flag_inicial){ 
    fpeak1 = search_fpeak_initial_faster (data16, sample_freq, FREQ0);
    fpeak2 = search_fpeak_initial_faster (data16, sample_freq, FREQ1);
    flag_inicial = 0;
  }
  //fpeak1 = search_fpeak_initial(data16, sample_freq, 10000, 50, fpeak1-240, fpeak1+245,5,BUFFER_SIZE); 
  //fpeak2 = search_fpeak_initial(data16, sample_freq, 10000, 50, fpeak2-20,  fpeak2+25,5,BUFFER_SIZE);

  ampl1 = calc_dft_singfreq(data16,fpeak1,sample_freq,1.7,10000,6,1);
  ampl2 = calc_dft_singfreq(data16,fpeak2,sample_freq,1.7,10000,60,6);
  
  Serial.print(ampl1);
  Serial.print("\t");
  Serial.println(ampl2);

}
