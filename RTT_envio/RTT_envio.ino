#include "RF24.h"
#include "nRF24L01.h"
#include <SPI.h>;


RF24 radio(16,17); //Configuração dos pinos CE, CSN (Pinos na placa ESP32)

byte Adresses[2][6] = {"00001","00002"}; //Define os endereções usados na comunicação
char message_sent[32] = "123321123"; //mensagem a ser enviada
float RTT_Time; //Round Trip Time, tempo total de envio e receção
long Start_Timer = 0;
long End_Timer = 0;
bool I_Started = false;

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
    RTT_Time = 0;
     if(!I_Started){
      while (!Serial.available()) {
        Serial.println(F("Press any key to start"));
        delay(1000);
        }
      }
    Start_Timer = millis(); //começa a contagem do tempo (retorna em microsegundos);
    while(!radio.write(&message_sent,sizeof(message_sent))); //a função retorna false enquanto não for enviada uma confirmação pelo receptor
    Serial.println("Message was sent");
    radio.startListening(); // O remetente vai ficar à espera para ouvir uma resposta do receptor
    while(radio.available() == false); //verifica se chegaram bytes para ser lidos
    Serial.println("Confirmation Received");
    End_Timer = millis();
    RTT_Time = End_Timer-Start_Timer;
    Serial.printf("RTT was : %f milisegundos", RTT_Time);
    delay(3000);
}
