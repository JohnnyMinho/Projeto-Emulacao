#include "RF24.h"
#include "nRF24L01.h"
#include <SPI.h>;
#include <printf.h>;
#include "CRCx.h";


RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

#define TIMEOUT_TIME 50000 // 25 milisegundos

byte Adresses[2][6] = {"00001","00002"}; //Define os endereções usados na comunicação
//String aux_string;
byte Packet[32]; // Distribuição da trama, 1 byte para header , 30 bytes de data, 1 byte (8bits dedicados a CRC)
byte Aux_Packet[30];
char Aux_Array[30];
byte crc = 0b0000000;
byte Ack_toSend = 0b01000010;
byte Nack_toSend = 0b01000100;
bool Ack_OR_Nack = true; //Se o crc for igual ao do enviado, enviamos uma ack ou seja true, caso o contrário se verifique é enviado um Nack, logo o valor passa a false
int counter = 0;
unsigned long Start_Timer = 0;
unsigned long Stop_Timer = 0;
unsigned long full_time = 0;
byte SequenceBit = 0;
byte Prev_SequenceBit = 0;
int NTramas = 0; //Como este programa tem código reutilizado das fases anterior este NTramas é na verdade o numero total de tramas recebidas
int NTramas_Retrans = 0;//Ver comentário acima
bool Start_of_program = true; //Apenas usado para garantir que o timer não leva reset a cada iteração do loop
bool Waiting_Retrans = false;

void setup() {
  Serial.begin(115200); //baudrate de 115200 bit/sec 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(100); // 2476 Mhz (RF24 channel 77))83 => 2483 Mhz (RF24 channel 84)
  radio.disableCRC();
  radio.openWritingPipe(Adresses[1]);
  radio.openReadingPipe(0,Adresses[0]);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS); //velocidade de transmissão de 1MBPS
  radio.startListening(); //Para o listening no modulo RF24 que está a enviar
}

void loop() {
while(!radio.available());

if(radio.available()){
  Serial.println("Received");
}
if(Start_of_program){ // Quando o programa receber um texto pela primeira vez ai é que associamos o run-time do programa à variável
  Start_Timer = micros(); //visto que assim evitamos erros associados a excesso de tempo visto que o programa começa a contar os ticks a partir do momento
  Start_of_program = false; // em que é inicializado
  }
memset(Packet,' ',30);
//Serial.println(NTramas);
radio.read(&Packet, sizeof(Packet)); 
CheckPacket();
}

//------------------------------------------------------Verifica a Receção do Pacote-------------------------------------------------------------------

void CheckPacket(){
   if(SequenceBit == Prev_SequenceBit && Waiting_Retrans){
    //Serial.println("PACKET REPETIDO");
    Waiting_Retrans = false;
  }
  if(bitRead(Packet[0], 3) == 0 && bitRead(Packet[0], 2) == 1 && bitRead(Packet[0], 1) == 1 || bitRead(Packet[0], 3) == 0 && bitRead(Packet[0], 2) == 0 && bitRead(Packet[0], 1) == 1 || bitRead(Packet[0], 3) == 1 && bitRead(Packet[0], 2) == 0 && bitRead(Packet[0], 1) == 1 ){
  for(counter = 0; counter<30;counter++){ // Precisamos de incluir o caso de quando estamos na trama final do pacote final 
    Aux_Packet[counter] = Packet[counter+1];
    Serial.println((char)Aux_Packet[counter]);
    //Serial.println(System.Text.Encoding.ASCII.GetString(Aux_Packet[counter]).ToCharArray();System.Text.Encoding.ASCII.GetString(Aux_Packet[counter]).ToCharArray());
  }
  crc = crcx::crc8(Aux_Packet, sizeof(Aux_Packet));
  if(crc == Packet[31]){
    Serial.println("Sending ACK");
   /*for(counter = 0; counter<30;counter++){
   Serial.write(Aux_Packet[counter]);
   }*/
    Ack_OR_Nack = true;// Ack_OR_Nack a verdadeiro, logo um ACK vai ser enviado
    Prev_SequenceBit = SequenceBit;
    NTramas++;
  }
  if(crc != Packet[31]){
    Serial.println("Sending Nack");
    Ack_OR_Nack = false; // Ack_OR_Nack a falso, logo um NACK vai ser enviado
    Waiting_Retrans = true;
    NTramas_Retrans++;
    NTramas++;
  }
}
  if(bitRead(Packet[0], 3) == 1 && bitRead(Packet[0], 2) == 0 && bitRead(Packet[0], 1) == 1){
     radio.stopListening();
     Stop_Timer = micros();
     full_time = Stop_Timer - Start_Timer;
     /*Serial.println("PACOTE FINAL RECEBIDO");
     Serial.println(NTramas);*/
     radio.write(&Ack_toSend, sizeof(Ack_toSend));
     full_time = Stop_Timer - Start_Timer;
     //END();
  }
//Aux_Packet.toCharArray(Aux_Array, sizeof(Aux_Array));

Prev_SequenceBit = SequenceBit;
radio.stopListening();
SendAck();
}
//--------------------------------------------------------------------------------------------------------------------------------------
void END(){
  int counter_envio_fim = 0;
  byte byte_fim[3] ={0b01000110, 0b01001001, 0b01001101 };
  for(counter_envio_fim = 0 ; counter_envio_fim < 30; counter_envio_fim++){
    if(counter_envio_fim < 3){
    Serial.write(byte_fim[counter_envio_fim]);
    }
    else{
      Serial.write(" ");
    }
  }
}
//----------------------------------------------------------Envia o ACK----------------------------------------------------------
void SendAck(){
    if(Ack_OR_Nack){
      radio.write(&Ack_toSend, sizeof(Ack_toSend));
    }
    if(!Ack_OR_Nack){
      radio.write(&Nack_toSend, sizeof(Nack_toSend));
    }
    radio.startListening();
  }
