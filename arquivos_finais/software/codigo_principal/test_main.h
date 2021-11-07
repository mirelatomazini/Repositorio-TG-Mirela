/*
    dft.h
    Versão 1 
    Autora: Mirela de Oliveira Tomazini
    Última atualização: 19 de agosto de 2021
    Descrição: O objetivo desse código e o desenvolvimento de funcoes que envolvam a transformada discreta de Fourier
*/

//#include <math.h>

//#define _USE_MATH_DEFINES - talvez tenha que usar para funcaionar M_PI
//#include <cmath> - talvez tenha que usar para funcaionar M_PI

double calc_dft_singfreq(float data[],float freq, float sample_freq, int factor_z=1){
  // funcao calcula a dft de uma unica frequencia

  
  float Freal     = 0;        
  float Fimag     = 0;
  float amplit    = 0;
  float phase     = 0;
  int nreal       = lenght(data[]);                           //definindo o tamanho do vetor, ou seja, quantidade de dados 
  int n           = nreal*factor;                   
  float df        = sample_freq/n;                            //definindo a df entre 2 amostras subsequentes (em frequencia)
  int k           = round(freq/sample_freq);                  // definindo k em termos de frequencia de amostragem e frequencia do sinal
  
  for (int m = 0; m<nreal; m++){
    Freal = Freal + data[m]*cos(k*m*2*M_PI/n);
    Fimag = Fimag + data[m]*sin(k*m*2*M_PI/n);
  }
  amplit = sqrt(pow(Freal,2)+pow(Fimag,2))/(n/2);
  phase = atan2(Fimag,Freal);

  return amplit;

}

float search_fpeak (float data[],float f_peak, float sample_freq, int factor = 10000, float faixa = 0.1, int n_p = 50){
  // funcao procura pelo frequencia de pico 

  int sttep       = (f_peak*faixa)/(n_p/2);
  int num         = ((f_peak*(1+faixa))-(f_peak*(1-faixa)))/sttep;
  
  float freq_value[num];
  float freq_datadft[num];
  
  float f_peak_real = 0;
  float peak        = 0;

  

  for(int cont=0; cont<num; cont++){
    freq_value[cont]    = (f_peak(1-faixa)) + cont*sttep;
    freq_datadft[cont]  = calc_dft_singfreq(data[],freq_value[cont],sample_freq, factor);
  }
  
  for(int cont=0; cont<num; cont++){
    if(freq_datadft[cont]>peak){
      peak          = freq_datadft[cont];
      f_peak_real   = freq_value[cont];
    }
  }

  return f_peak_real;

}


float search_fpeak_initial (float data[], float sample_freq, int factor = 10000, int n_p = 50, float f_peak_i = 1000, float f_peak_f =20000, int sttep = 10){
  // funcao procura pelo frequencia de pico na inicialização do programa

  int num         = (f_peak_f-f_peak_i)/sttep;
  float f_peak_real = 0;
  float peak        = 0;

  float freq_value[num];
  float freq_datadft[num];
  

  for(int cont=0; cont<num; cont++){
    freq_value[cont]    = f_peak_i + cont*sttep;
    freq_datadft[cont]  = calc_dft_singfreq(data[],freq_value[cont],sample_freq, factor);
  }
  
  for(int cont=0; cont<num; cont++){
    if(freq_datadft[cont]>peak){
      peak          = freq_datadft[cont];
      f_peak_real   = freq_value[cont];
    }
  }

  return f_peak_real;

}
