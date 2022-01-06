#include "RF24.h"
#include "nRF24L01.h"
#include <SPI.h>;
#include <CRCx.h>;



RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

#define TIMEOUT_TIME 25000 // 25 milisegundos
//#define Packet_1 0b11111111
//#define Packet_2 0b11100111
//#define Header_Inicial 0b00100000
//#define Header_Normal 0b00010110
#define NPackets 50

byte Adresses[2][6] = {"00001","00002"}; //Define os endereções usados na comunicação
String aux_string;
byte Packet[32]; // Distribuição da trama, 1 byte para header (2 bits de endereço de origem, 2 bits para endereço de chegade, 3 para tipo e 1 para permitir a verificação da repetição do pacote , 30 bytes de data, 1 byte (8bits dedicados a CRC)
byte PacketCRC[30];
byte Ack = 0b000000000;
char Aux_Array[30] = " ";
int string_size = 0;
int org_string_size = 0; //guarda o tamanho original da string
int posArray = 0; //Só é usada caso seja necessário enviar mais que uma trama
int times_to_send = 0; //Vai guardar o numero de tramas a enviar
byte Header_Fim = 0b00101010;
//byte Packet_3 = 0b10000010;
int counter = 0;
int counter_to_error = 0; //Esta variável só é usada para permitir o envio de um erro a cada 4 packets enviados
long Start_Timer = 0;
long timer = 0;
bool timeout = false;
bool I_Started = false;
bool I_Received_Something = false;
bool Send_Dif = false; //Como os bits de cada packet são pre-definidos, os bits do packet mudam a cada packet
byte SequenceBit = 0; //indica o numero da trama
int Sent_Packets = 0; //Conta o numero de pacotes enviado sendo que no momento em que o n 100 é enviado, é enviado o pacote que sinaliza o final
bool Ack_or_Nack = false; //Indica se foi recebido um Ack (true) ou um Nack(false)
bool DividePayload = false;

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
delay(100);
radio.stopListening();
if(!I_Started){
   while (!Serial.available()) {
    Serial.println("Type in the String to be sent");
    delay(1000);
   }
   if(Serial.available()){
    aux_string = Serial.readStringUntil('\n');
    Serial.println(aux_string.length());
    org_string_size = aux_string.length();
    string_size = org_string_size;
    aux_string.toCharArray(Aux_Array, sizeof(Aux_Array));
    //}
   }
   I_Started = true;
  }
  if(string_size > 30){
      times_to_send = string_size/30;
    }
    else{
      times_to_send = 1;
    }

for(counter = times_to_send; counter >= 1; counter--){
if(counter > 1){
    times_to_send--;
    *Packet = CreatePacket(Send_Dif, string_size, Aux_Array, posArray);
    posArray = string_size;
    string_size = string_size-30;
}
else{
*Packet = CreatePacket(Send_Dif, string_size, Aux_Array, posArray);
}

Send_Dif = false;

if(radio.write(&Packet, sizeof(Packet))){
  timer = micros();
  Serial.println("Packet Sent");
  Serial.println(Se nt_Packets);
  if(times_to_send == 1){
  Sent_Packets++;
  }
  counter_to_error++; //Fica em comentário para todos os outros problemas excepto conta_erros
  Start_Timer = micros();
  radio.startListening();
  delay(100);
}

  while(!radio.available()){
    if(micros()-timer >= TIMEOUT_TIME){
      timeout = true;
    }
  }
  if(!timeout){
    radio.read(&Ack, sizeof(Ack));
    Serial.println(Ack);
    Ack_or_Nack = CheckACK(Ack);
    if(Ack_or_Nack){
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
}
 /* if(Sent_Packets == 50){
    radio.write(&Header_Fim ,sizeof(Header_Fim));
    Serial.println("FIM");
    while(1);
  }*/
}

//------------------------------------------------------Criar Pacote-------------------------------------------------------------------

byte CreatePacket(bool SendDif, int StringSize, char* Text,int newpos){
    Packet[0] = 0b00010110; //Endereço emissor (00) Endereço receptor(01) Tipo (111) Numero de trama (0) 
  if(Send_Dif){
    bitWrite(Packet[0], 0 , 1);
    Send_Dif = false;
  }
  for(counter = newpos;counter < newpos+30; counter++){ //Introduzimos os dados a ser enviados no pacoteCRC visto que este vai ser o pacote a sofrer o CRC para os seus dados
      PacketCRC[counter] = Text[counter];
      if(Text[counter] == NULL){
      Text[counter] = '*';
      }
    }
 for(counter = newpos+1; counter < newpos+31; counter ++){
    Packet[counter] = PacketCRC[counter-1]; // Introduzimos na trama, os dados 
 }
 Packet[31] = crcx::crc8(PacketCRC, sizeof(PacketCRC));
 Serial.println(Packet[31]);
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