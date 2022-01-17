#include "RF24.h"
#include "nRF24L01.h"
#include <math.h>
#include <SPI.h>;
#include <CRCx.h>;



RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

#define TIMEOUT_TIME 50000 // 50 milisegundos

byte Adresses[2][6] = {"00001","00002"}; //Define os endereções usados na comunicação
String aux_string;
byte Packet[32]; // Distribuição da trama, 1 byte para header (2 bits de endereço de origem, 2 bits para endereço de chegade, 3 para tipo e 1 para permitir a verificação da repetição do pacote , 30 bytes de data, 1 byte (8bits dedicados a CRC)
byte PacketCRC[30];
byte Ack = 0b000000000;
char Aux_Array[256] = " ";
double string_size = 0;
int org_string_size = 0; //guarda o tamanho original da string
int posArray = 0; //Só é usada caso seja necessário enviar mais que uma trama
double times_to_send = 0; //Vai guardar o numero de tramas a enviar
byte Header_Fim = 0b00101010;
int counter = 0;
int counter_to_error = 0; //Esta variável só é usada para permitir o envio de um erro a cada 4 packets enviados
unsigned long Start_Timer = 0;
unsigned long timer = 0;
bool timeout = false;
bool I_Started = false;
bool I_Received_Something = false;
bool Send_Dif = false; //Como os bits de cada packet são pre-definidos, os bits do packet mudam a cada packet
byte SequenceBit = 0; //indica o numero da trama
int Sent_Packets = 0; //Conta o numero de pacotes enviado sendo que no momento em que o n 100 é enviado, é enviado o pacote que sinaliza o final
bool Ack_or_Nack = false; //Indica se foi recebido um Ack (true) ou um Nack(false)

void setup() {
  Serial.begin(115200); //baudrate de 115200 bit/sec 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(76); // 2476 Mhz (RF24 channel 77))83 => 2483 Mhz (RF24 channel 84)
  radio.disableCRC();
  radio.openWritingPipe(Adresses[0]);
  radio.openReadingPipe(1,Adresses[1]);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS); //velocidade de transmissão de 1MBPS
  radio.stopListening(); //Para o listening no modulo RF24 que está a enviar

}

void loop() {

   while (!Serial.available();
   if(Serial.available()){
    I_Started = true;
    aux_string = Serial.readStringUntil('\n');
    string_size = aux_string.length();
    aux_string.toCharArray(Aux_Array, sizeof(Aux_Array));
    if(string_size > 30){
      times_to_send = string_size/30;
      times_to_send = ceil(times_to_send);
    }
    else{
      times_to_send = 1;
    }
   }
  
for(counter = ceil(times_to_send); counter >= 1; counter--){
radio.stopListening();
*Packet = CreatePacket(Send_Dif, string_size, Aux_Array, posArray);
Send_Dif = false;

if(radio.write(&Packet, sizeof(Packet))){
  timer = micros();
  Serial.println("Packet Sent");
  Serial.println(Sent_Packets);
  Start_Timer = micros();
  radio.startListening();
  delay(100);
}

  while(!radio.available()){
    if(micros()-(timer+100)>= TIMEOUT_TIME){
      timeout = true;
    }
  }
  if(!timeout){
    radio.read(&Ack, sizeof(Ack));
    Ack_or_Nack = CheckACK(Ack);
    if(Ack_or_Nack){
      if(counter == 1){
        Sent_Packets++;
        counter_to_error++; //Fica em comentário para todos os outros problemas excepto conta_erros, visto que é só introduz um erro para testes
      }
      posArray = posArray+30;
      string_size = string_size-30;
      Send_Dif = true;
      Ack_or_Nack = false;
    }
    else{
      Sent_Packets--;
    }
    Ack = 0b00000000;
  }
  else{
    Serial.println("-TIME OUT-");
    timeout = false;
    }
  if(Sent_Packets == 100){
    radio.stopListening();
    Serial.println(Sent_Packets);
    radio.write(&Header_Fim ,sizeof(Header_Fim));
    Serial.println("FIM");
    while(1);
  }
  else{
      posArray = 0;
      string_size = org_string_size;
}
}
}

//------------------------------------------------------Criar Pacote-------------------------------------------------------------------

byte CreatePacket(bool SendDif, int StringSize, char* Text,int newpos){
  int counter_create =0;
   if(StringSize > 30){
     Packet[0] = 0b00010110; //Endereço emissor (00) Endereço receptor(01) Tipo Trama não Final (011) Numero de trama (0) 
   }
   else{
     Packet[0] = 0b00010010; //Endereço emissor (00) Endereço receptor(01) Tipo Final(001) Numero de trama (0) 
   }
  if(Send_Dif){
    bitWrite(Packet[0], 0 , 1);
    Send_Dif = false;
  }
  for(counter_create = 0;counter_create < 30; counter_create++){ //Introduzimos os dados a ser enviados no pacoteCRC visto que este vai ser o pacote a sofrer o CRC para os seus dados
      PacketCRC[counter_create] = Text[newpos+counter_create];
      if(Text[newpos+counter_create] == NULL){
      Text[counter_create] = '*';
      }
    }
 for(counter_create = 1; counter_create < 31; counter_create ++){
    Packet[counter_create] = PacketCRC[counter_create-1]; // Introduzimos na trama, os dados 
 }
 Packet[31] = crcx::crc8(PacketCRC, sizeof(PacketCRC));
 //Serial.println(Packet[31]);
 if((counter_to_error) == 4){
    counter_to_error = 0;
    Packet[1] = 'I';
 }
 return *Packet;
}

//-----------------------------------Esta função só é usada quando o numero de tramas é > 1-----------------------------------------------------------------------------------

//----------------------------------------------------------Verificação do ACK----------------------------------------------------------
bool CheckACK(byte Ack_Received){
  bool flag1 = false;
  if(bitRead(Ack, 1) == 1 && bitRead(Ack, 2) == 0 && bitRead(Ack, 3) == 0){ // Como o bitRead lê do least significant bit para o MSB, a ordem dos bits é contrária à escrita
    Serial.println("Recebido com sucesso"); 
    Serial.println(Ack);
    I_Received_Something = true;
    flag1 = true;
  }
  else if(bitRead(Ack, 1) == 0 && bitRead(Ack, 2) == 1 && bitRead(Ack, 3) == 0){
    Serial.println(Ack);
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
