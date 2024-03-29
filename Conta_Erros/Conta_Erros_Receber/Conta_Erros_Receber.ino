 #include "RF24.h"
#include "nRF24L01.h"
#include <SPI.h>;
#include <printf.h>;
#include "CRCx.h";


RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

#define TIMEOUT_TIME 25000 // 25 milisegundos

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
int Npackets = 0;
int Npackets_Retrans = 0;
bool Start_of_program = false; //Apenas usado para garantir que o timer não leva reset a cada iteração do loop
bool Waiting_Retrans = false;

void setup() {
  Serial.begin(115200); //baudrate de 115200 bit/sec 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(76); // 2476 Mhz (RF24 channel 77))83 => 2483 Mhz (RF24 channel 84)
  radio.disableCRC();
  radio.openWritingPipe(Adresses[1]);
  radio.openReadingPipe(0,Adresses[0]);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS); //velocidade de transmissão de 1MBPS
  radio.startListening(); //Para o listening no modulo RF24 que está a enviar


}
void loop() {
radio.startListening();
while(!radio.available());
if(radio.available()){
  Serial.println("Received");
}
if(Start_of_program){
  Start_of_program = false;
  Start_Timer = micros();
}
radio.read(&Packet, sizeof(Packet)); 
CheckPacket();
}

//------------------------------------------------------Verifica a Receção do Pacote-------------------------------------------------------------------

void CheckPacket(){
   if(SequenceBit == Prev_SequenceBit && Waiting_Retrans){
    Serial.println("PACKET REPETIDO");
    Waiting_Retrans = false;
    Npackets--;
  }
  if(bitRead(Packet[0], 3) == 0 && bitRead(Packet[0], 2) == 1 && bitRead(Packet[0], 1) == 1 || bitRead(Packet[0], 3) == 0 && bitRead(Packet[0], 2) == 0 && bitRead(Packet[0], 1) == 1){
  for(counter = 0; counter<30;counter++){
    Aux_Packet[counter] = Packet[counter+1];
    Serial.println((char)Aux_Packet[counter]);
    //Serial.println(System.Text.Encoding.ASCII.GetString(Aux_Packet[counter]).ToCharArray();System.Text.Encoding.ASCII.GetString(Aux_Packet[counter]).ToCharArray());
  }
  crc = crcx::crc8(Aux_Packet, sizeof(Aux_Packet));
  Serial.println(crc);
  if(crc == Packet[31]){
    Serial.println("Sending ACK");
    Ack_OR_Nack = true;// Ack_OR_Nack a verdadeiro, logo um ACK vai ser enviado
    Prev_SequenceBit = SequenceBit;
    if(bitRead(Packet[0], 3) == 0 && bitRead(Packet[0], 2) == 0 && bitRead(Packet[0], 1) == 1){
    Npackets++;
    }
  }
  else{
    Serial.println("Sending Nack");
    Ack_OR_Nack = false; // Ack_OR_Nack a falso, logo um NACK vai ser enviado
    Waiting_Retrans = true;
    Npackets_Retrans++;
  }
}
  if(bitRead(Packet[0], 3) == 1 && bitRead(Packet[0], 2) == 0 && bitRead(Packet[0], 1) == 1){
     radio.stopListening();
     Stop_Timer = micros();
     Serial.println("PACOTE FINAL RECEBIDO");
     Serial.println(Npackets);
     radio.write(&Ack_toSend, sizeof(Ack_toSend));
     full_time = Start_Timer - Stop_Timer;
     END(Npackets,Npackets_Retrans,Stop_Timer,full_time);
  }
//Aux_Packet.toCharArray(Aux_Array, sizeof(Aux_Array));

Prev_SequenceBit = SequenceBit;
radio.stopListening();
SendAck();
}
//--------------------------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------Envia o ACK----------------------------------------------------------
void SendAck(){
    if(Ack_OR_Nack){
      radio.write(&Ack_toSend, sizeof(Ack_toSend));
    }
    if(!Ack_OR_Nack){
      radio.write(&Nack_toSend, sizeof(Nack_toSend));
    }
  }

void END(int Pacote_Recebido, int Pacote_Ret, long tempofim, long tempototal){
  int Bytes_Recebidos = Pacote_Recebido*32;
  double per_retrans = 0;
  per_retrans = (Pacote_Ret/Pacote_Recebido);
  Serial.println("Pacotes Recebidos");
  Serial.println(Pacote_Recebido);
  Serial.println("Pacote Retransmitidos");
  Serial.println(Pacote_Ret);
  Serial.println("Tempo Usado");
  Serial.println(full_time);
  Serial.println("Numero de Bytes recebidos");
  Serial.println(Bytes_Recebidos);
  Serial.println("Percentagem de Pacotes Retransmitidos");
  Serial.print(per_retrans);
  Serial.println("%");
}
