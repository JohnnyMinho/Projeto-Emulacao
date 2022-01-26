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
String aux_string;
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
bool sucess = false;
bool enviar = false;

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
  radio.startListening();
  while(!radio.available());
  if(radio.available()){
    radio.read(Aux_Array, sizeof(Aux_Array));
    Serial.println(Aux_Array[0]);
    radio.stopListening();
    enviar = true;
    delay(100);
  }
  while(enviar){
      if(radio.write(Aux_Array, sizeof(Aux_Array))){
        sucess = true;
      }
    }
    if(sucess){
      sucess = false;
      enviar = false;
      memset(Aux_Array,0,30);
      delay(50);
    }
  }
  
  
