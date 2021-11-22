#include "RF24.h"
#include "nRF24L01.h"
#include <SPI.h>;
#include "printf.h"

RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

byte Adresses[2][6] = {"00001","00002"}; //Define os endereções usados na comunicação
int n_packets = 0;
int n_bytes = 0;
int n_noncorrupt_packets = 0; // O numero de pacotes recevidos não corrompidos
int n_corruptpackets = 0; //O numero de pacotes corrompidos
bool stop_sign = false;
long Start_Timer,Stop_Timer = 0;
char Packet_Store[32] = ""; //Vai guardar o packet recebido momentaneamente 
char Packet_Check[32] = "Hello";
char Packet_Stop[32] = "END";
int counter = 0;
float Debit = 0;
bool corrupted_packet = false;


void setup() {
  Serial.begin(115200); //baudrate de 115200 bit/sec 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(111); // 2476 Mhz (RF24 channel 77))83 => 2483 Mhz (RF24 channel 84)
  radio.openWritingPipe(Adresses[1]);
  radio.openReadingPipe(0,Adresses[0]);
  radio.setDataRate(RF24_1MBPS); //velocidade de transmissão de 1MBPS
  radio.setAutoAck(false);
  radio.disableCRC(); // Desligar
  radio.startListening();
}

void loop(){
  if(radio.available() && !stop_sign /*&& n_packets <= 10*/){ //Caso seja usado o packet de paragem acho que não é necessário o uso do n_packets como condição de paragem
    Start_Timer = millis();
    radio.read(&Packet_Store , sizeof(Packet_Store));
    if(strcmp(Packet_Store, Packet_Stop) == 0){
       Stop_Program();
       stop_sign = true;
       Stop_Timer = millis();
    }
    if(!stop_sign){
    for(counter = 0; counter != 32 ; counter++){
      if(Packet_Store[counter] != Packet_Check[counter]){
      corrupted_packet = true;
    }
    }
      if(corrupted_packet){
        corrupted_packet = false;
        n_corruptpackets++;
    }
    Serial.println(n_packets+1);
    n_packets++;
    memset(Packet_Store,0,sizeof(Packet_Store));
    }
  }
}

void Stop_Program(){
      int lost_packets;
      long total_time = -((Stop_Timer-Start_Timer)+75000)/1000;
      n_bytes = (n_packets*32);
      lost_packets = 500-n_packets;
      float Losses = ((n_corruptpackets+(lost_packets))/500.0)*100.0;
      Serial.println(total_time);
      Debit = ((n_bytes*8.0)/(total_time));
      radio.stopListening();
      radio.write(&Debit, sizeof(float));
      Serial.println(Packet_Store);
      Serial.printf("Number of packets : %d \n", n_packets);
      Serial.printf("Number of Corrupt : %d \n", n_corruptpackets);
      Serial.printf("Number of bytes used: %d \n", n_bytes);
      Serial.printf("Total Time: %d \n", -(Stop_Timer-Start_Timer));
      Serial.printf("Lost Packets: ");
      Serial.println(lost_packets);
      Serial.printf("Losses: ");
      Serial.println(Losses);
      Serial.printf("Debit %f \n", Debit);
      while(1);
      delay(10000);
      
}
