/*
   codigo baseado no desenvolvido por GameInstance.com 2016-2018 e adaptador por Mirela Tomazini:
   STM32 Digital Oscilloscope
   using the STM32F103C8 MCU and the NT35702 2.4 inch TFT display
   https://www.gameinstance.com/post/80/STM32-Oscilloscope-with-FFT-and-SD-export 
*/

/* a primeira parte do codigo importa as bibliotecas e cria variaveis globais */

#define FREQ200 200000
#define FREQ20 20000
#define BUFFER_SIZE  1200
#define VCC_3_3 3.3
#define CHANNELS_PER_ADC  1                      // number of channels for each ADC. Must match values in ADCx_Sequence array below
#define NUM_SAMPLES       1200                    // number of samples for each ADCx. Each channel will be sampled NUM_SAMPLES/CHANNELS_PER_ADC
#define ADC_SMPR          ADC_SMPR_7_5           // when using dual mode, each pair of channels must have same rate. Here all channels have the same
#define PRE_SCALER        RCC_ADCPRE_PCLK_DIV_6  // Prescaler do ADC
#define FAST_INTERLEAVED  false                   // Fast Interleave Mode Flag. Para "dobrar" taxa de amostragem medindo o mesmo canal dos 2 ADCs.
                                                 // Se 'false', habilita "Regular simultaneous mode". Se 'true', habilita "Fast interleaved mode".
#include "stm32_adc_dual_mode.h"
//#include "adc_proj.h"
#include "dft.h"

int flag_inicial          = 1;
float fpeak200            = 0;
float fpeak20             = 0;
float sample_freq         = 600000;
const float referenceVolts= 3.3;
uint32 adcbuf[NUM_SAMPLES+1];  // buffer to hold samples, ADC1 16bit, ADC2 16 bit
uint16 data16[NUM_SAMPLES+1]; 
uint16 data16cor[NUM_SAMPLES+1];

// O STM32F103 possui 10 pinos do ADC disponíveis:
// pino B0 (PB0) -> 8 (ADC8)
// pino B1 (PB1) -> 9 (ADC9)
uint8 ADC1_Sequence[]={8,0,0,0,0,0};   // ADC1 channels sequence, left to right. Unused values must be 0. Note that these are ADC channels, not pins  
uint8 ADC2_Sequence[]={9,0,0,0,0,0};   // ADC2 channels sequence, left to right. Unused values must be 0



void setup() {
  Serial.begin(115200);
  delay(5000);
  set_adc_dual_channel(PRE_SCALER, ADC_SMPR, CHANNELS_PER_ADC, ADC1_Sequence, ADC2_Sequence, FAST_INTERLEAVED);  // initial ADC1 and ADC2 settings
  Serial.println("Configuracao ok");
}


void loop() {
  // funcao usada para realizar a aquisicao;
  float media               = 0;
  float media_cor           = 0;
  float amplit_v200         = 0;
  float amplit_i200         = 0;
  float amplit_v20          = 0;
  float amplit_i20          = 0;
  float phase_v200          = 0;
  float phase_i200          = 0;
  float phase_v20           = 0;
  float phase_i20           = 0;
  
  start_convertion_dual_channel(adcbuf, NUM_SAMPLES);
  wait_convertion_dual_channel();

  for(int h=0; h<(NUM_SAMPLES); h++){
    data16[h]=(adcbuf[h] & 0xFFFF);
    data16cor[h]= (adcbuf[h] & 0xFFFF0000)>>16;
  }
  media = sinal_medio(data16);
  media_cor = sinal_medio(data16cor);

  if(1==2){
    // imprimindo valores lidos:
    for(int i=0;i<(NUM_SAMPLES);i++) {
      float volts= ((adcbuf[i] & 0xFFFF) / 4095.0)* referenceVolts;
      float voltss=  (((adcbuf[i] & 0xFFFF0000) >>16) / 4095.0)* referenceVolts;
      
      if(FAST_INTERLEAVED){ // Fast interleaved mode
        Serial.print("ADC:");
        Serial.println(voltss); //ADC2 é convertido primeiro... Ver [2], pág 10.
        Serial.print("ADC:");
        Serial.println(volts);
      }
      else{ // Regular simultaneous mode
        Serial.print("ADC1:");
        Serial.print(volts);
        Serial.print("\tADC2:");
        Serial.println(voltss);
      }
    }
    Serial.println();
  }
  
  
  if(flag_inicial){ 
    fpeak200 = search_fpeak_initial_faster (data16, sample_freq, FREQ200);
    Serial.println(fpeak200);
    fpeak20 = search_fpeak_initial_faster (data16, sample_freq, FREQ20);
    Serial.println(fpeak20);
    flag_inicial = 0;
  }
  //fpeak1 = search_fpeak_initial(data16, sample_freq, 10000, 50, fpeak1-240, fpeak1+245,5,BUFFER_SIZE); 
  //fpeak2 = search_fpeak_initial(data16, sample_freq, 10000, 50, fpeak2-20,  fpeak2+25,5,BUFFER_SIZE);


  calc_dft_singfreq_phase(data16,fpeak200,sample_freq,media, amplit_v200, phase_v200);
  calc_dft_singfreq_phase(data16cor,fpeak200,sample_freq,media_cor, amplit_i200, phase_i200);
  calc_dft_singfreq_phase(data16,fpeak20,sample_freq,media, amplit_v20, phase_v20);
  calc_dft_singfreq_phase(data16cor,fpeak20,sample_freq,media_cor, amplit_i20, phase_i20);

  float corrente20  = amplit_i20/(47.0*6.0);
  float corrente200 = amplit_i200/(47.0*6.0);
  float impedancia20 = amplit_v20/corrente20;
  float impedancia200 = amplit_v200/corrente200;

  if(1==1){
    Serial.print(amplit_v200);
    Serial.print("V\t");
    Serial.print(amplit_v20);
    Serial.print("V\t");
    Serial.print(1000.0*corrente200);
    Serial.print("mA\t");
    Serial.print(1000.0*corrente20);
    Serial.print("mA\t");
    float dif200 = phase_v200-phase_i200;
    float dif20 = phase_v20-phase_i20;
    if(dif200>0) dif200=dif200-6.2830;
    if(dif20>0) dif20=dif20-6.2830;
    
    Serial.print(dif200*180/3.1415);
    Serial.print("gr\t");
    Serial.print(dif20*180/3.1415);
    Serial.print("gr\t");
    Serial.print(impedancia200);
    Serial.print("ohm\t");
    Serial.print(impedancia20);
    Serial.println("ohm\t");

  }

}
