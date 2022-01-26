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
unsigned long timer = 0;
bool timeout,I_Started,I_Received_Something = false;
bool Send_Dif = false; //Como os bits de cada packet são pre-definidos, os bits do packet mudam a cada packet
byte SequenceBit,Prev_SequenceBit = 0; //indica o numero da trama
int Sent_Packets,N_Retrans = 0; //Conta o numero de pacotes enviado sendo que no momento em que o n 100 é enviado, é enviado o pacote que sinaliza o final
int Resent_Packets = 0; //Fica em comentário excepto na verificação do débito
bool Ack_or_Nack = false; //Indica se foi recebido um Ack (true) ou um Nack(false)
bool envia_fim,recebe_fim,receber,nothing_to_send = false;
bool Waiting_Retrans = false;
unsigned long Start_Timer,full_time,Stop_Timer = 0;

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
  radio.startListening();
}

void loop() {

/*if(!I_Started){
   while (!Serial.available());
   if(Serial.available()){
    I_Started = true;
   }
   /* aux_string = Serial.readStringUntil('\n');
    org_string_size = aux_string.length();
    string_size = org_string_size;
    aux_string.toCharArray(Aux_Array, sizeof(Aux_Array));
    if(string_size > 30){
      times_to_send = string_size/30;
      times_to_send = ceil(times_to_send);
    }
    else{
      times_to_send = 1;
    }
   }*/
if(Serial.available()>0){
radio.stopListening();
while(!envia_fim){
  while(Serial.available()){
      Serial.readBytes(Aux_Array,sizeof(Aux_Array));
      counter_receber = sizeof(Aux_Array);
      array_size = counter_receber;
  }

  if(counter_receber > 0){
    while(!Ack_or_Nack){
       memset(Packet,'\0', 30);
      *Packet = CreatePacket(Send_Dif, Aux_Array,array_size);
      envia();
    }
  Ack_or_Nack = false;
  memset(Aux_Array,'\0', sizeof(Aux_Array));
  counter_receber = 0;
  envia_fim = true;
  }
}
}
envia_fim = false;
if(radio.available()){
while(!recebe_fim){
  while(!receber){
    recebe_packet();
  }
  receber = false;
}
}

recebe_fim = false;
Send_Dif = false;
 
}
 /* if(Sent_Packets == total_pacotes){
    debito = ((Sent_Packets-Resent_Packets)*times_to_send*32.0*8.0)/((micros()-Start_Timer)*0.000001);
    //Serial.print("Debito ");
   //Serial.println(debito);
   // Serial.println("FIM");
    while(1);
  }
  else{
      Total_Time = Total_Time + (micros()-Start_Timer);
      posArray = 0;
      string_size = org_string_size;
}*/

//------------------------------------------------------Criar Pacote-------------------------------------------------------------------

byte CreatePacket(bool SendDif, byte* Text,int tamanho_array){
  int counter_create =0;
   if(tamanho_array > 30){
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
//---------------------------------------------------------ENVIA-------------------------------------------------
void envia(){
  if(radio.write(&Packet, sizeof(Packet))){
  timer = micros();
  //Serial.println("Packet Sent");
  //Serial.println(Sent_Packets+1); // Como a contagem dos pacotes começa em 0 precisamos de indicar desta maneira
  Start_Timer = micros()+100;
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
     /* if(counter == 1){
        Sent_Packets++;
      }*/
      /*else{
      //posArray = posArray+30;
      //string_size = string_size-30;
      }*/
      Send_Dif = true;
      //Ack_or_Nack = false;
    }
   /* else{
      if(Sent_Packets != 0){
      Resent_Packets++;
      Sent_Packets--;
      }
    }*/
    Ack = 0b00000000;
  }
  else{
    //Serial.println("-TIME OUT-");
    //counter = ceil(times_to_send);
    timeout = false;
    }
    radio.startListening();
}
}
//----------------------------------------------------------RECEBER----------------------------------------------
 void recebe_packet(){
 byte Packet_Receiver[32]; // Criamos um array próprio para receceber o Pacote vindo do chat
 byte Packet_Payload[30];
 byte crc;
 byte Ack_receptor;
  while (!radio.available());
  radio.read(Packet_Receiver, sizeof(Packet_Receiver));
  Serial.println("Li Packet");
  if(bitRead(Packet_Receiver[0], 3) == 0 && bitRead(Packet_Receiver[0], 2) == 1 && bitRead(Packet_Receiver[0], 1) == 1 || bitRead(Packet_Receiver[0], 3) == 0 && bitRead(Packet_Receiver[0], 2) == 0 && bitRead(Packet_Receiver[0], 1) == 1 || bitRead(Packet_Receiver[0], 3) == 1 && bitRead(Packet_Receiver[0], 2) == 0 && bitRead(Packet_Receiver[0], 1) == 1 ){
  for(counter = 0; counter<30;counter++){ // Precisamos de incluir o caso de quando estamos na trama final do pacote final 
    Packet_Payload[counter] = Packet_Receiver[counter+1];
    Serial.println((char)Packet_Payload[counter]);
    //Serial.println(System.Text.Encoding.ASCII.GetString(Aux_Packet[counter]).ToCharArray();System.Text.Encoding.ASCII.GetString(Aux_Packet[counter]).ToCharArray());
  }
  crc = crcx::crc8(Packet_Payload, sizeof(Packet_Payload));
  SequenceBit = bitRead(Packet_Receiver[0],0);
  if(SequenceBit == Prev_SequenceBit && Waiting_Retrans){
    Sent_Packets--;
    Waiting_Retrans = false;
  }
  else{
    Serial.write(Packet_Payload, sizeof(Packet_Payload));
    receber = true;
  }
  if(crc == Packet_Receiver[31]){
    radio.stopListening();
    Ack_receptor = 0b01000010;
    radio.write(&Ack_receptor, sizeof(Ack_receptor));
    Serial.println("Sending ACK");
    //Ack_OR_Nack = true;// Ack_OR_Nack a verdadeiro, logo um ACK vai ser enviado
    Prev_SequenceBit = SequenceBit;
    Sent_Packets++;
  }
  if(crc != Packet_Receiver[31]){
    radio.stopListening();
    Ack_receptor = 0b01000100;
    Serial.println("Sending Nack");
    radio.write(&Ack_receptor, sizeof(Ack_receptor));
    //Ack_OR_Nack = false; // Ack_OR_Nack a falso, logo um NACK vai ser enviado
    Waiting_Retrans = true;
    N_Retrans++;
  }
}
  if(bitRead(Packet[0], 3) == 1 && bitRead(Packet[0], 2) == 0 && bitRead(Packet[0], 1) == 1){
     radio.stopListening();
     Ack_receptor = 0b01000010;
     recebe_fim = true;
     Stop_Timer = micros();
     full_time = Stop_Timer - Start_Timer;
     Serial.println("PACOTE FINAL RECEBIDO");
    /* Serial.println(NTramas);*/
     radio.write(&Ack_receptor, sizeof(Ack_receptor));
     full_time = Stop_Timer - Start_Timer;
  }
  
//Aux_Packet.toCharArray(Aux_Array, sizeof(Aux_Array));
Prev_SequenceBit = SequenceBit;
 }
//----------------------------------------------------------Verificação do ACK----------------------------------------------------------
bool CheckACK(byte Ack_Received){
  bool flag1 = false;
  if(bitRead(Ack, 1) == 1 && bitRead(Ack, 2) == 0 && bitRead(Ack, 3) == 0){ // Como o bitRead lê do least significant bit para o MSB, a ordem dos bits é contrária à escrita
    Serial.println("Recebido com sucesso"); 
    //Serial.println(Ack);
    I_Received_Something = true;
    flag1 = true;
  }
  else if(bitRead(Ack, 1) == 0 && bitRead(Ack, 2) == 1 && bitRead(Ack, 3) == 0){
    //Serial.println(Ack);
    flag1 = false;
    Serial.println("NACK recebido");
    I_Received_Something = true;
  }
  if(!I_Received_Something){
    Serial.println("ACK OU NACK PERDIDO");
  }
  I_Received_Something = false;
  return flag1;
}
