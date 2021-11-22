#include "RF24.h"
#include "nRF24L01.h"
#include <SPI.h>;


RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32 para a RF24)

byte Adresses[6] = "00001"; //Define os endereções usados na comunicaç
char To_Send[32]= "";
char To_Receive[32] = "";
String aux_string = "";
String User = " ";
int first_start = 0; //verifica se o programa está a ser corrido pela primeira vez de modo a associar um numero para facilitar na leitura dos dados enviados
char Test[32] = "Teste";

void setup() {
  Serial.begin(115200); //baudrate de 115200 bit/sec 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(128); // 2476 Mhz (RF24 channel 77))83 => 2483 Mhz (RF24 channel 84)
  radio.disableCRC(); // Desligar os protocolos de correção de erros
  radio.openWritingPipe(Adresses);
  radio.openReadingPipe(0,Adresses);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS); //velocidade de transmissão de 2MBPS
}


void loop() {
 /* if(first_start == 0){
    while(User==" "){
    Serial.println("Escreva um numero para associar ao port a ser usado");
    User = Serial.readStringUntil('\n');
    delay(10000);
    }
    first_start = 1;
  } */
    radio.startListening();
   if(radio.available()){
      //Chat_Receive();
    radio.read(&To_Receive, sizeof(To_Receive));
   //  Serial.printf("User: %c", User);
      Serial.println("User_Getting:");
     Serial.println(To_Receive);
   }
   if(Serial.available()){
   radio.stopListening();
   //Chat_Write();
   aux_string = Serial.readStringUntil('\n');
   //Serial.printf("User: %c", User);
   Serial.println("User_Sending:");
   Serial.println(aux_string);
   aux_string.toCharArray(To_Send, sizeof(To_Send));
   radio.write(&To_Send, sizeof(To_Send));
   memset(To_Send, 0, sizeof(To_Send));
   radio.startListening();
  }
}
/*
void Chat_Receive(){
  radio.read(&To_Receive, sizeof(To_Receive));
   //  Serial.printf("User: %c", User);
      Serial.println("User_Getting:");
     Serial.println(To_Receive);
}

void Chat_Write(){
  aux_string = Serial.readStringUntil('\n');
   //Serial.printf("User: %c", User);
   Serial.println("User_Sending:");
   Serial.println(aux_string);
   aux_string.toCharArray(To_Send, sizeof(To_Send));
   radio.write(&To_Send, sizeof(To_Send));
   memset(To_Send, 0, sizeof(To_Send));
}*/
