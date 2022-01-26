#include "RF24.h"
#include "nRF24L01.h"
#include <math.h>
#include <SPI.h>;
#include <CRCx.h>;

RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

#define TIMEOUT_TIME 50000 // 50 milisegundos

byte Adresses[2][6] = {"00001","00002"}; //Define os endereções usados na comunicação
byte Packet[32]; // Distribuição da trama, 1 byte para header (2 bits de endereço de origem, 2 bits para endereço de chegade, 3 para tipo e 1 para permitir a verificação da repetição do pacote , 30 bytes de data, 1 byte (8bits dedicados a CRC)
byte PacketCRC[30];
byte Ack = 0b000000000;
byte Aux_Array[30] = " ";
int array_size = 0;
int posArray = 0; //Só é usada caso seja necessário enviar mais que uma trama
double times_to_send = 0; //Vai guardar o numero de tramas a enviar
byte Header_Fim = 0b00101010;
int counter = 0;
int counter_receber = 0;
unsigned long Start_Timer = 0;
unsigned long Total_Time = 0;
unsigned long timer = 0;
bool timeout = false;
bool I_Started = false;
bool I_Received_Something = false;
bool Send_Dif = false; //Como os bits de cada packet são pre-definidos, os bits do packet mudam a cada packet
byte SequenceBit = 0; //indica o numero da trama
int Sent_Packets = 0; //Conta o numero de pacotes enviado sendo que no momento em que o n 100 é enviado, é enviado o pacote que sinaliza o final
int Resent_Packets = 0; //Fica em comentário excepto na verificação do débito
bool Ack_or_Nack = false; //Indica se foi recebido um Ack (true) ou um Nack(false)
double debito = 0;

void setup() {
  Serial.begin(115200); //baudrate de 115200 bit/sec 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(100); // 2476 Mhz (RF24 channel 77))83 => 2483 Mhz (RF24 channel 84)
  radio.disableCRC();
  radio.openWritingPipe(Adresses[0]);
  radio.openReadingPipe(1,Adresses[1]);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS); //velocidade de transmissão de 1MBPS
  radio.stopListening(); //Para o listening no modulo RF24 que está a enviar

}

void loop() {
radio.stopListening();
if(Serial.available() > 0 ){
    memset(Packet,' ', 30);
    memset(Aux_Array,' ', sizeof(Aux_Array));
    Serial.readBytes(Aux_Array,sizeof(Aux_Array));
    counter_receber = sizeof(Aux_Array);
    array_size = counter_receber;
    *Packet = CreatePacket(Send_Dif, Aux_Array,array_size);
 }

if(counter_receber > 0){
  while(!Ack_or_Nack){
    envia();
  }
  Ack_or_Nack = false;
  counter_receber = 0;
}
}

//------------------------------------------------------Criar Pacote-------------------------------------------------------------------

byte CreatePacket(bool SendDif, byte* Text,int tamanho_array){
  int counter_create =0;
   if(tamanho_array == 30){
     Packet[0] = 0b00010110; //Endereço emissor (00) Endereço receptor(01) Tipo Trama não Final (011) Numero de trama (0) 
   }
   else{
     Packet[0] = Header_Fim; //Endereço emissor (00) Endereço receptor(01) Tipo Final(001) Numero de trama (0) 
   }
  /* if(Sent_Packets == total_pacotes){
     Serial.println("FIM enviado");
     Packet[0] = Header_Fim; // é preciso enviar o final quando o número de pacotes é menos um que o final senão seriam enviado 101 pacotes e não 100
   } */
  if(Send_Dif){
    bitWrite(Packet[0], 0 , 1);
    Send_Dif = false;
  }
  for(counter_create = 0;counter_create < 30; counter_create++){ //Introduzimos os dados a ser enviados no pacoteCRC visto que este vai ser o pacote a sofrer o CRC para os seus dados
      PacketCRC[counter_create] = Text[counter_create];
      if(Text[counter_create] == NULL){
      PacketCRC[counter_create] = ' ';
      }
    }
 for(counter_create = 1; counter_create < 31; counter_create ++){
    Packet[counter_create] = PacketCRC[counter_create-1]; // Introduzimos na trama, os dados 
 }
 Packet[31] = crcx::crc8(PacketCRC, sizeof(PacketCRC));
 return *Packet;
}
//----------------------------------------------------------Envia------------------------------------

void envia(){
  if(radio.write(&Packet, sizeof(Packet))){
  timer = micros();
  //Serial.println("Packet Sent");
  //Serial.println(Sent_Packets+1); // Como a contagem dos pacotes começa em 0 precisamos de indicar desta maneira
  Start_Timer = micros()+100;
  radio.startListening();
  delay(100);
}
  while(!radio.available()){
    if(micros()-(timer+100)>= TIMEOUT_TIME){
      timeout = true;
      break;
    }
  }
  if(!timeout){
    radio.read(&Ack, sizeof(Ack));
    Ack_or_Nack = CheckACK(Ack);
    if(Ack_or_Nack){
      Serial.write(Ack);
      Sent_Packets++;
      Send_Dif = true;
    }
    else{
      Serial.write(Ack);
      Ack_or_Nack = false;
      if(Sent_Packets != 0){
      Resent_Packets++;
      Sent_Packets--;
      }
    }
    Ack = 0b00000000;
  }
  else{
    //Serial.println("-TIME OUT-");
    //counter = ceil(times_to_send);
    timeout = false;
    }
    radio.stopListening();
}

//----------------------------------------------------------Verificação do ACK----------------------------------------------------------
bool CheckACK(byte Ack_Received){
  bool flag1 = false;
  if(bitRead(Ack, 1) == 1 && bitRead(Ack, 2) == 0 && bitRead(Ack, 3) == 0){ // Como o bitRead lê do least significant bit para o MSB, a ordem dos bits é contrária à escrita
    //Serial.println("Recebido com sucesso"); 
    //Serial.println(Ack);
    I_Received_Something = true;
    flag1 = true;
  }
  else if(bitRead(Ack, 1) == 0 && bitRead(Ack, 2) == 1 && bitRead(Ack, 3) == 0){
    //Serial.println(Ack);
    flag1 = false;
    //Serial.println("NACK recebido");
    I_Received_Something = true;
  }
  if(!I_Received_Something){
    //Serial.println("ACK OU NACK PERDIDO");
    flag1=false;
  }
  I_Received_Something = false;
  return flag1;
}
