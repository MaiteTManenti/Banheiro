/*
   Projeto Final - Banheiro Inteligente
   
   Autora: Mait� Thomazi Manenti
   
   Descri��o: O projeto possui um chuveiro onde � poss�vel selecionar o tempo em que o mesmo fica ligado e a sele��o da temperatura da �gua,
              e tamb�m automatizar o vaso sanit�rio e a descarga, tendo uma economia no consumo de �gua.

*/

#include <Banheiro.h>
#include <LCD.C>
//Defines para o chuveiro
#define UP PIN_B1
#define DOWN PIN_B2
#define SELECIONAR PIN_B3
#define ON_OFF PIN_B4
#define BUZZER PIN_C1
#define VAL_AGUA_CHUVEIRO PIN_C0
//Defines para o vaso
#define Sensor_TA PIN_B5
#define Sensor_TF PIN_B6
#define Val_Descarga_Agua_Pia PIN_C3
#define Val_Descarga_Agua_Normal PIN_C4
#define Sensor_Presenca PIN_B0
#define Sensor_Agua_Caixinha_Pia PIN_B7
#define Motor PIN_C5

int configuracao;                                                                //3 est�gios - 0:n�o configurando 1-configurando temperatura e 2-configurando tempo
int16 set_temperatura = 30, set_tempo = 10, segundo = 0, tempo_Valv = 0;         //segundo- vari�vel que conta quantas interrup��es teve. Idem para temp_Valv
int1 Banho = 0;                                                                  //vari�vel auxiliar para saber qual o estado do chuveiro
int16 temperaturaD_Atual = 0;                                                    //vari�vel que armazena a temperatura atual lido pelo sensor LM35

#INT_TIMER1                                                                      //interrup��o do timer 1 - 16bits
void trata_int1()
{
   
   if (segundo < (2 * 60 * set_tempo))                                           //f�rmula para contar at� o tempo que a pessoa setou no LCD
   { 
      output_low(BUZZER);                                                        //desativa o buzzer
      segundo = segundo + 1;                                                     //incrementa o contador de interrup��es  segundo
      set_timer1(3036);                                                          //seta timer novamente para pr�xima contagem
      if(segundo == ((2*60*set_tempo) - (2*60)))                                 //Buzzer aciona quando falta 1 minuto
         output_high(BUZZER);                                                    //Buzzer acionado
   }
   else                                                                          //se j� contou at� o tempo setado pelo usu�rio
   {  
      output_low(VAL_AGUA_CHUVEIRO);                                             //desliga valvula de �gua do chuveiro
      output_low(BUZZER);                                                        //desativa o buzzer
      segundo = 0;                                                               //zera vari�vel auxiliar segundo
      set_pwm1_duty(0);                                                          //desabilita sa�da PWM
      Banho = 0;                                                                 //Seta a vari�vel banho com zero, pois o banho acabou
      setup_timer_1(T1_DISABLED);                                                //desabilita o timer1 temporariamente   
   }
}

#INT_TIMER0                                                                      //interrup��o do timer 0 - 8bits
void trata_int0()
{
   if(tempo_Valv < 625)                                                          //conta at� 10segundos
   {
      tempo_Valv = tempo_Valv + 1;                                               //incrementa a vari�vel temp_Valv, a descarga fica acionada por 10s
      set_timer0(131);                                                           //seta o timer novamente para a pr�xima contagem
   }
   else                                                                          //se tempo for 10s
   {
      output_low(Val_Descarga_Agua_Normal);                                      //desabilita as v�lvulas de descargas Normal e Pia
      output_low(Val_Descarga_Agua_Pia);
      tempo_Valv = 0;                                                            //zera a vari�vel temp_Valv
   }     
}

void main()
{
   port_b_pullups(true);                                                         //habilita pullups
   
   setup_wdt(WDT_2304MS);                                                        //configura o tempo de contagem, c�o de guarda
   
   setup_timer_1(T1_DISABLED);                                                   //desabilita o timer temporariamente
   enable_interrupts(INT_TIMER1);                                                //ativa interrup��o no timer 1
   enable_interrupts(INT_TIMER0);                                                //ativa interrup��o no timer 0
   enable_interrupts(GLOBAL);                                                    //ativa interrup��o global
   
   setup_adc_ports(AN0);                                                         //seleciona o pino A0
   setup_adc(ADC_CLOCK_INTERNAL);                                                //seleciona timer interno - temporizador
   
   setup_timer_2(T2_DIV_BY_16,255,1);                                            //setar o timer2 - para o uso de PWM     
                                                                                 //[PR2+1]*4 = 100% do duty (m�ximo valor do duty)
                                                                                 //duty_cicle = value/(4*[PR2+1]) - duty de 0-1, 0%-100%
   setup_ccp1(CCP_PWM);                                                          //setar a porta RC2-CCP1 como modo PWM
   
   output_low(BUZZER);                                                           //desativa Buzzer
   output_low(VAL_AGUA_CHUVEIRO);                                                //desativa v�lvula �gua do chuveiro
   
   configuracao = 0;                                                             //inicia vari�vel configura��o com zero
   
   lcd_init();                                                                   //inicializa o LCD 20x4
   lcd_cursor_on(1);                                                             //habilita o cursor no LCD
   lcd_gotoxy(1,2);                                                              //vai para x = 1 e y = 2 no painel LCD
   printf(LCD_PUTC, "Sel_Tempo: %2li", set_tempo);                               //escreve no LCD o tempo de banho escolhido
   lcd_gotoxy(21,1);                                                             //vai para x = 21 e y = 1 no painel LCD
   printf(LCD_PUTC, "Sel_Temperatura: %2li", set_temperatura);                   //escreve no LCD a temperatura escolhida
   lcd_gotoxy(1,1);                                                              //vai para x = 1 e y = 1 no painel LCD
   printf(LCD_PUTC, "Temp_Atual: %3li", temperaturaD_Atual);                     //escreve no LCD a temperatura atual

   while(TRUE)
   {
      restart_wdt();                                                             //reinicializa a contagem
      lcd_cursor_on(0);                                                          //desativa cursor no LCD
      set_adc_channel(0);                                                        //ativa canal 0 para o conversor AD
      delay_us(10);                                                              //tempo para o conversor AD
      
      temperaturaD_Atual = (5*read_adc()*100.0)/1023.0;                          //f�rmula para converter o valor de sa�da do sensor de temperatura
      lcd_gotoxy(13,1);                                                          //vai para x = 13 e y = 1 no painel LCD
      printf(LCD_PUTC, "%3li", temperaturaD_Atual);                              //escreve o valor da temperatura atual (igual ao sensor) no LCD

      if(Banho == 0)                                                             //com o chuveiro desligado
      {
      
         if(configuracao == 0)                                                   //configura��o do bot�o
         {
            if(input(SELECIONAR) == 0)                                           //se apertar 1 vez, configura��o vai para 1
            {
               delay_ms(400);                                                    //tempo do bot�o ser solto, debounce
               configuracao = 1;
            }
         }
         
         else if(configuracao == 1)                                              //se o bot�o selecionar for apertado uma vez, estou no estado que muda temperatura
         {
            lcd_cursor_on(1);                                                    //aciona cursor;
            lcd_gotoxy(38,1);                                                    //vai para x = 38 e y = 1 no painel LCD
            if(input(SELECIONAR) == 0)                                           //se apertar o bot�o selecionar novamente, configura��o recebe o valor 2
            {
               delay_ms(400);
               configuracao = 2;
            }
            if(input(DOWN) == 0)                                                 //se o bot�o down (diminuir a temperatura) for apertado 
            {
               delay_ms(400);                                                    //tempo para o debounce
               set_temperatura--;                                                //decrementa o valor da temperatura
               printf(LCD_PUTC, "%2li", set_temperatura);                        //printar na tela o valor de temperatura selecionado
            }
            if(input(UP) == 0)                                                   //se o bot�o up (aumentar a temperatura) for apertado
            {
               delay_ms(400);                                                    //tempo para o debounce
               set_temperatura++;                                                //incrementa o valor da temperatura
               printf(LCD_PUTC, "%2li", set_temperatura);                        //printar na tela o valor de temperatura selecionada
            }
         }
               
         else if(configuracao == 2)                                              //se a configura��o for 2, estou no estado que muda o tempo que o chuveiro fica ligado
         {
            lcd_cursor_on(1);                                                    //habilita o cursor
            lcd_gotoxy(12,2);                                                    //vai para x = 12 e y = 2 no painel LCD
            if(input(SELECIONAR) == 0)                                           //se o bot�o de seleionar for apertado novamente, vai para o estado que n�o faz nada
            {
               delay_ms(400);                                                    //tempo para o debounce
               configuracao = 0;                                                 //reseta a vari�vel configura��o
            }  
            if(input(DOWN) == 0)                                                 //se o bot�o de down for apertado
            {
               delay_ms(400);                                                    //tempo para o debounce
               set_tempo--;                                                      //decrementa o valor do tempo
               printf(LCD_PUTC, "%2li", set_tempo);                              //printar no display LCD o tempo selecionado
            }
            if(input(UP) == 0)                                                   //se o bot�o do up for apertado
            {
               delay_ms(400);                                                    //tempo para tirar o dedo do bot�o
               set_tempo++;                                                      //incrementar o tempo com o valor desejado
               printf(LCD_PUTC, "%2li", set_tempo);                              //printar no display LCD o tempo selecionado
            }

         }  
               
         lcd_cursor_on(0);                                                       //desativa o cursor
         if(input(ON_OFF) == 0)                                                  //se o bot�o de ligar o chuveiro for acionado
         {
            Banho = 1;                                                           //setar o valor do banho em 1
            delay_ms(400);                                                       //tempo para o debounce
            output_high(VAL_AGUA_CHUVEIRO);                                      //liga v�lvula de �gua para o chuveiro
            lcd_cursor_on(0);                                                    //desabilita cursor
            setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);                              // ativa o timer1 no clock interno e com pre-escaler de 8,  
                                                                                 // 4Mhz/4 = 1Mhz; 1Mhz / 8 = 125Khz
                                                                                 // 1/125K = 8us;  1s/8us = 125000 pulsos
                                                                                 // 125000pulsos / 2 contagens = 62500 pulsos/contagem    --a cada estouro do time, conta 0.5s
            set_timer1(3036);                                                    //seta o tempo do timer 1 em 62500, -> 65536 - 62500 = 3036 -- 62500*8us = 0.5s
                                                                                 // set_timer(3036) conta 0.5 segundos
         }
      }
      
      else if(Banho == 1)                                                        //Chuveiro Ligado
      {
         if(configuracao == 2)                                                   //se inicialmente a configura��o estiver no estado de mudar o tempo, altero para fazer nada
            configuracao = 0;
            
         if(temperaturaD_Atual > set_temperatura)                                //compara��o de temperaturas, se a temp atual � maior que a temp desejada ent�o a resist�ncia(PWM) n�o � ativado 
            set_pwm1_duty(0);
         else                                                                    //se a temp atual for menor que a temp desejada
            set_pwm1_duty((set_temperatura-temperaturaD_Atual)*20);              //f�rmula do PWM - 1024/50 pois 50 � a m�xima diferen�a de temperatura
                                                                                 //diferen�a de temp for 0, o pwm seja 0 e quando a diferen�a for 50 o pwm seja 1024
                                                                                 //quando for 0 fica 0 * 20 = 0 e quando a diferen�a for 25, 25 * 20 = 500, o PWM fica na metade
        
         if(configuracao == 0)                                                   //se a configura��o for 0
         {
            if(input(SELECIONAR) == 0)                                           //se o bot�o selecionar for apertado
            {
               delay_ms(400);                                                    //tempo para o debounce
               configuracao = 1;                                                 //seta o valor de configura��o para 1
            }
         }
            
         else if(configuracao == 1)                                              //se o valor de configura��o for 1, � poss�vel alterar somente temperatura ap�s o chuveiro ligado
         {
            lcd_cursor_on(1);                                                    //aciona cursor
            lcd_gotoxy(38,1);                                                    //vai para x = 38 e y = 1 no painel LCD
            if(input(SELECIONAR) == 0)                                           //se apertar o bot�o selecionar novamente
            {
               delay_ms(400);                                                    //delay para o debounce
               configuracao = 0;                                                 //reseta o valor da vari�vel configura��o
            }
            if(input(DOWN) == 0)                                                 //se o bot�o DOWN for acionado
            {
               delay_ms(400);                                                    //tempo para o debounce
               set_temperatura--;                                                //decrementa temperatura
               printf(LCD_PUTC, "%2li", set_temperatura);                        //printa na tela o valor da temperatura desejada
            }
            if(input(UP) == 0)                                                   //se o bot�o UP for acionado
            {
               delay_ms(400);                                                    //delay para o debounce
               set_temperatura++;                                                //incrementa a temperatura
               printf(LCD_PUTC, "%2li", set_temperatura);                        //printa no LCD o valor da temperatura desejada
            }
         } 
         
      
         if(input(ON_OFF) == 0)                                                  //se o bot�o de ON/OFF do chuveiro for acionado, ou seja, desligar o chuveiro sem o tempo acabar
         {
            Banho = 0;                                                           //setar o valor do banho em 1, ou seja, desligar o chuveiro depois de ligado
            delay_ms(400);                                                       //tempo para o debounce
            output_low(VAL_AGUA_CHUVEIRO);                                       //desliga v�lvula de �gua para o chuveiro
            lcd_cursor_on(0);                                                    //desabilita cursor
            setup_timer_1(T1_DISABLED);
            set_pwm1_duty(0);
      
         }   
        
      }
         
                                                                                 //Vaso sanit�rio e controle da caixinha de �gua para descarga
      if(input(Sensor_TA) == 0)                                                  //se o sensor de tampa aberta for acionado
      {
         if(input(Sensor_Presenca) == 0)                                         //1-n�o tem ngm, 0-tem algu�m. Se o sensor de presen�a for acionado
         {  
            while(input(Sensor_Presenca) == 0)                                   //ficar aqui enquanto tem uma pessoa utilizando o vaso
            {
               restart_wdt();                                                    //reinicializa a contagem do wdt
            }
            
            output_high(Motor);                                                  //quando o sesor de presenta n�o est� mais acionado, o motor liga
            while(input(Sensor_TF) == 1)                                         //enquanto o sensor de tampa fechada n�o for acionado, o motor continua ligado
            {
               restart_wdt();                                                    //reinicializa a contagem do wdt
            }                                                                                
            output_low(Motor);                                                   //sensor de tampa fechado acionado, desliga o motor
            
            if(input(Sensor_Agua_Caixinha_Pia) == 0)                             //se o sensor(boia) da caixinha d'�gua que sai da pia estiver acionada
            {
               output_high(Val_Descarga_Agua_Pia);                               //ligar a descarga que sai a �gua descartada pela pia
            }
            else                                                                 //se o sensor n�o estiver acionado
            {
               output_high(Val_Descarga_Agua_Normal);                            //a descarga funcionar� normalmente com a �gua que vem da caixa d'�gua - aciona v�lvula de descarga normal
            }
            setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128);                           // ativa o timer0 no clock interno e com pre-escaler de 128,  
                                                                                 // 4Mhz/4 = 1Mhz; 1Mhz / 128 = 7812.5hz
                                                                                 // 1/7812.5 = 128us;  10s/128us = 78125 pulsos
                                                                                 // 78125pulsos / 625 contagens = 125 pulsos/contagem     --precisa ter 625 estouros do time contando 125 pulsos para ter 10s
            set_timer0(131);                                                     //seta o tempo do timer 0 em 256, -> 256 - 125 = 131     --125*128us = 0.016(tempo que leva para estourar timer) * 625 = 10s
                                                                                 
            
         }
                
      }
      
   }

}
