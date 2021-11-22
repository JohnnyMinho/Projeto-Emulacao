#include "RF24.h"
#include "nRF24L01.h"
#include <SPI.h>;


RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

byte Adresses[2][6] = {"00001","00002"}; //Define os endereções usados na comunicação
char message[32] = "";
char message_confirmation[32] = "123321123"; //mensagem a ser enviada

void setup() {
  Serial.begin(115200); //baudrate de 115200 bit/sec 
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(100); // 2476 Mhz (RF24 channel 77))83 => 2483 Mhz (RF24 channel 84)
  radio.disableCRC();
  radio.openWritingPipe(Adresses[1]);
  radio.openReadingPipe(0,Adresses[0]);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS); //velocidade de transmissão de 2MBPS
  radio.startListening(); //Como este RF é o receptor este é configurado para começar logo a "ouvir" uma possivel mensagem
}

void loop() {
    
    bool end_of_message = false;
    int counter = 0;
    int x = 3000;
    if(!end_of_message){
    while(!radio.available());
    radio.read(&message, sizeof(message));
    radio.stopListening(); //Para o listening no modulo RF24 que está a enviarSerial.println("Hello");
    while(radio.write(&message_confirmation,sizeof(message_confirmation)==false)); //envia a mensagem de confirmação
    radio.startListening(); // O remetente vai ficar à espera para ouvir uma resposta do receptor
    Serial.print("Packet : ");
    Serial.print(message);
    counter++;
    }
    if(counter > x){
    radio.stopListening();
    end_of_message = true;
    Serial.printf("Mensagem recebida, conteudo: %c", message);
    delay(4000);
    }
}
