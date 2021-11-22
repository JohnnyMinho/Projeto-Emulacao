#include "RF24.h"
#include "nRF24L01.h"
#include <SPI.h>;


RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

byte Adresses[2][6] = {"00001","00002"}; //Define os endereções usados na comunicação
int n_packets = 500;
int counter = 0;
char packet_message[32] = "Hello";
char packet_message_endsign[32] = "END";
bool Stopped_Sending = false;
float Debit_Recept = 0;
bool I_Started = false;


void setup() {
  Serial.begin(115200); //baudrate de 115200 bit/sec 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(111); // 2476 Mhz (RF24 channel 77))83 => 2483 Mhz (RF24 channel 84)
  radio.openWritingPipe(Adresses[0]);
  radio.openReadingPipe(1,Adresses[1]);
  //radio.openReadingPipe(1,Adresses[0]);
  radio.disableCRC();
  radio.setDataRate(RF24_1MBPS); //velocidade de transmissão de 1MBPS
  radio.setAutoAck(false);
  radio.startListening();
  radio.stopListening();
}

void loop() {
  
 if(!I_Started){
  
  while (!Serial.available()) {
    Serial.println(F("Press any key to start"));
    delay(1000);
  }
  }
  if(counter<n_packets){
    radio.write(&packet_message, sizeof(packet_message));
    Serial.println("Packet n:");
    Serial.println(counter);
    counter++;
    delay(150);
  }
  /*if(counter<=n_packets){
    radio.write(&packet_message, sizeof(packet_message));
    Serial.println("Packet n:");
    Serial.println(counter);
    counter++;
    delay(200);
  }*/
  else{
    if(!Stopped_Sending){
    Serial.println("Sent all packets");
    radio.write(&packet_message_endsign,sizeof(packet_message_endsign));
    Serial.println(packet_message_endsign);
    Stopped_Sending = true;
    }
  }
    if(Stopped_Sending){
    radio.startListening();
    ListenforDebit();
    }
}

void ListenforDebit(){
    
    while(!radio.available());
    while(radio.available()){
    radio.read(&Debit_Recept, sizeof(float));
    Serial.print("Debit:");
    Serial.println(Debit_Recept);
    while(1);
    }
}
