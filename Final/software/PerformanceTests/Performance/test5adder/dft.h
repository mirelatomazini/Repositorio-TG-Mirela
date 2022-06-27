/*
    dft.h
    Versão 1 
    Autora: Mirela de Oliveira Tomazini
    Última atualização: 19 de agosto de 2021
    Descrição: O objetivo desse código e o desenvolvimento de funcoes que envolvam a transformada discreta de Fourier
*/


float sinal_medio (uint16_t *data){
  float media = 0;
  for (int i=0; i<BUFFER_SIZE; i++){
    media = media + data[i];
  }
  media = media/BUFFER_SIZE;
  return media;
}

void calc_dft_singfreq_phase(uint16_t *data,float freq, float sample_freq, float media, float &amplit, float &phase, int factor_z=1000, float pontos=BUFFER_SIZE){
  
  double Freal     = 0;        
  double Fimag     = 0;
  float nreal     = pontos;                           //definindo o tamanho do vetor, ou seja, quantidade de dados 
  float n         = nreal*factor_z;                   
  float df        = sample_freq/n;                            //definindo a df entre 2 amostras subsequentes (em frequencia)
  float k         = round(freq/df);                  // definindo k em termos de frequencia de amostragem e frequencia do sinal

  for (int m = 0; m<nreal; m++){
    Freal = Freal + (data[m]-media)*cos(k*m*2.0*M_PI/n);
    Fimag = Fimag + (data[m]-media)*sin(k*m*2.0*M_PI/n);
  }
  amplit = (sqrt(pow(Freal,2)+pow(Fimag,2))/(nreal/2.0))*3.3/4096.0;
  phase = atan2(Fimag,Freal);

  //Serial.print(freq);
  //Serial.print(" ");
  //Serial.println(amplit, 7);

}


double calc_dft_singfreq(uint16_t *data,float freq, float sample_freq, float media, int factor_z=1000, float pontos=BUFFER_SIZE){
  
  double Freal     = 0;        
  double Fimag     = 0;
  double amplit    = 0;
  float phase     = 0;
  float nreal     = pontos;                           //definindo o tamanho do vetor, ou seja, quantidade de dados 
  float n         = nreal*factor_z;                   
  float df        = sample_freq/n;                            //definindo a df entre 2 amostras subsequentes (em frequencia)
  float k         = round(freq/df);                  // definindo k em termos de frequencia de amostragem e frequencia do sinal

  for (int m = 0; m<nreal; m++){
    Freal = Freal + (data[m]-media)*cos(k*m*2.0*M_PI/n);
    Fimag = Fimag + (data[m]-media)*sin(k*m*2.0*M_PI/n);
  }
  amplit = (sqrt(pow(Freal,2)+pow(Fimag,2))/(nreal/2.0))*3.3/4096.0;
  phase = atan2(Fimag,Freal);

  //Serial.print(freq);
  //Serial.print(" ");
  //Serial.println(amplit, 7);
  
  return amplit;

}

float search_fpeak (uint16_t *data,float f_peak, float sample_freq, int factor = 10000, float faixa = 0.1, int n_p = 50, int pontos = BUFFER_SIZE){
  // funcao procura pelo frequencia de pico 
  float media = 0;
  media = sinal_medio(data);
  
  int sttep       = (f_peak*faixa)/(n_p/2);
  int num         = ((f_peak*(1+faixa))-(f_peak*(1-faixa)))/sttep;
  
  float freq_value[num];
  float freq_datadft[num];
  
  float f_peak_real = 0;
  float peak        = 0;

  

  for(int cont=0; cont<num; cont++){
    freq_value[cont]    = (f_peak*(1-faixa)) + cont*sttep;
    freq_datadft[cont]  = calc_dft_singfreq(data,freq_value[cont],sample_freq,media,factor, pontos);
  }
  
  for(int cont=0; cont<num; cont++){
    if(freq_datadft[cont]>peak){
      peak          = freq_datadft[cont];
      f_peak_real   = freq_value[cont];
    }
  }

  return f_peak_real;

}


float search_fpeak_initial (uint16_t *data, float sample_freq, int factor = 10000, int n_p = 50, float f_peak_i = 100, float f_peak_f =20000,float sttep = 100, float pontos=BUFFER_SIZE){
  float media = 0;
  media = sinal_medio(data);
  int num         = (f_peak_f-f_peak_i)/sttep;
  float f_peak_real = 0;
  float peak        = 0;

  float freq_value[num];
  float freq_datadft[num];
  

  for(int cont=0; cont<num; cont++){
    freq_value[cont]    = f_peak_i + cont*sttep;
    freq_datadft[cont]  = calc_dft_singfreq(data,freq_value[cont],sample_freq,media,factor,pontos);
  }
  
  for(int cont=0; cont<num; cont++){
    if(freq_datadft[cont]>peak){
      peak          = freq_datadft[cont];
      f_peak_real   = freq_value[cont];
    }
    
  }

  if (1==1){
    Serial.print(peak, 6);
    Serial.print(" ******** ");
    Serial.print(f_peak_real, 2);
    Serial.println(" ");
  }
 
  if(1==1){
    for (int i=0; i<BUFFER_SIZE; i++){
      Serial.print(data[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
  return f_peak_real;
}


float search_fpeak_initial_faster (uint16_t *data, float sample_freq, float fpeak){
  // funcao procura pelo frequencia de pico na inicialização do programa

  float minim       = 0;

  if (1==1){
    
    if((fpeak-40000)>0){
      minim = fpeak-40000;
    }
    
    fpeak = search_fpeak_initial (data, sample_freq, 10000, 50, minim, fpeak+40000,10000); //realiza 8 calculos
    
    if((fpeak-10000)>0){
      minim = fpeak-10000;
    }
    
    fpeak = search_fpeak_initial (data, sample_freq, 10000, 50, minim, fpeak+10000,1000); //realiza 10 calculos

  }
  if((fpeak-1000)>0){
    minim = fpeak-1000;
  }
  fpeak = search_fpeak_initial (data, sample_freq, 10000, 50, minim, fpeak+1000,100); //realiza 10 calculos
  
  if((fpeak-100)>0){
    minim = fpeak-100;
  }
  fpeak = search_fpeak_initial (data, sample_freq, 10000, 50, minim, fpeak+100,10); //realiza 10 calculos
  
  if((fpeak-10)>0){
    minim = fpeak-10;
  }
  fpeak = search_fpeak_initial (data, sample_freq, 10000, 50, minim, fpeak+10,1); //realiza 10 calculos

  return fpeak;
}
