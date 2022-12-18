#include <Encoder.h>

//pulsos por rotação do encoder
#define ppr 20

//frequencia de amostragem
#define sampleRate 100 

//clock da placa
#define clk 16000000 

//define os pinos 
#define A_in 2
#define B_in 4
#define pinoPWM 3
#define A_out 11
#define B_out 12

//valor para zerar o contador do timer 1 
unsigned int ocr = clk / (64 * sampleRate); 

//variáveis auxiliares para o recebimento de mensagens
int recebido=0;
int recebidoAntigo=0;

//variável do valor do duty cycle (de 0 a 255)
unsigned char d = 0;

//inicia o encoder com os pinos de leitura
Encoder myEnc(A_in, B_in);

//variáveis para o cálculo da velocidade
long oldPosition = 0;
long newPosition = 0;
float velocidade = 0;

//interrupção periódica 
ISR(TIMER1_COMPA_vect) {

  //leitura da posição do encoder
  newPosition = myEnc.read();
  
  //cálculo da velocidade
  velocidade = ((newPosition - oldPosition)*sampleRate)/ppr;

  //atualização da variável de posição
  oldPosition = newPosition;
  
  //lógica para receber dados 

  //se existir algum dado disponível no buffer de comunicação
  if (Serial.available() > 0) {

    //salva o valor recebido
    recebido = Serial.parseInt ();

    // se o valor recebido for maior que zero, o pino 11 recebe 0 e o 12 recebe 1, se for menor o contrário
    if (recebido > 0){
      digitalWrite(A_out, LOW);
      digitalWrite(B_out, HIGH);  
      d = recebido;
      
    }else if(recebido == 0 && recebidoAntigo == 0){
      d=0;
    }else if(recebido < 0){      
      digitalWrite(B_out, LOW);
      digitalWrite(A_out, HIGH); 
      d = recebido*(-1);
    }
    recebidoAntigo=recebido;
  }

  //aplica o valor recebido no duty cycle do PWM
  analogWrite(pinoPWM,d);

  //envia os dados captados pela comunicação serial
  Serial.print(d);
  Serial.print(",");
  Serial.print(velocidade);
  Serial.print(",");
  Serial.println(newPosition);
}

void setup() {
  //Configura o baud rate da comunicação serial 
  Serial.begin(115200);

  //Habilita as interrupções globais
  SREG |= (1 << 7);

  //Configura os pinos como entrada ou saída
  pinMode(A_in, INPUT);
  pinMode(B_in, INPUT);
  pinMode(pinoPWM, OUTPUT);
  pinMode(A_out, OUTPUT);
  pinMode(B_out, OUTPUT);

  //Inicia os pinos 11 e 12 com 1 e 0 respectivamente
  digitalWrite(A_out, HIGH);
  digitalWrite(B_out, LOW);

  //modo normal de operação
  TCCR1A = 0b00000000;

  //OCR1a como limite prescaller:1/64 modo CTC
  TCCR1B = 0b00001011; 
  /* tbela para o TIMER0 e TIMER1
    0 0 1 clkI/O/1 (No prescaling)
    0 1 0 clkI/O/8 (From prescaler)
    0 1 1 clkI/O/64 (From prescaler)
    1 0 0 clkI/O/256 (From prescaler)
    1 0 1 clkI/O/1024 (From prescaler)
  */

  // Compare Interrupt Enable. Interrupção por comparação ativada.
  TIMSK1 = 0x02; 

  //valor para zerar o contador do timer 1  
  OCR1A = ocr; 
}

void loop() {
  //vazio, pois toto o código é feito na interrupção
}
