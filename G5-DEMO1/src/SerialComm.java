/*
*   MIETI 2021/2022 EMULAÇÃO E SIMULAÇÃO DE REDES DE TELECOMUNICAÇÕES
*   GRUPO 4 - FASE 1.2
*   CATARINA AMORIM A93094
*   JOÃO MOURA A93099
*   LUÍS PINTO A94155
*   MIGUEL PEREIRA A94152
* *  */

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Scanner;
import com.fazecast.jSerialComm.SerialPort;
import java.util.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.lang.*;

public class SerialComm {


        public static void main(String[] args) throws IOException{
            int option; //para opções
            long start_clock; //para medições de tempo
            long turnoff_clock;
            Scanner input = new Scanner(System.in);
            long total_time;
            int counter = 0;
            boolean You_Pass;



            //ver disponibilidade das portas (getCommPorts / check ports/
            SerialPort[] AvailablePorts = SerialPort.getCommPorts();
            if(AvailablePorts.length == 0){
                System.out.println("No Ports Available");
                System.out.println("Press Any key to Exit. \n If You wish to restart press R");
                String exit_option = input.nextLine();
                System.exit(1);
            }

            //Selecionar Port
            for(SerialPort ToChoose_Port : AvailablePorts){
                counter++;
                System.out.println(counter + " - " + ToChoose_Port.getSystemPortName());
            }
            int Sent_Port = input.nextInt();
            int Choosen_port = VerifyPort(Sent_Port,AvailablePorts);
            SerialPort Selected_Port = AvailablePorts[Choosen_port]; //É necessário verificar se o port está aberto


            //configuração da porta série (baudrate, bits (8), stop bits (1), paridade (0))
            Selected_Port.setComPortParameters(115200, 8, 1 , 0);

            System.out.print("\033[H\033[2J"); //limpar consola

            System.out.println("SELECTED PORT: " + Selected_Port.toString());
            System.out.println(" _________________________________");
            System.out.println("<1.ENVIAR TEXTO                     >");
            System.out.println("<2.ENVIAR IMAGEM                    >");
            System.out.println("<3.VERIFICAR CONEXÃO                >");
            System.out.println("<4.                                 >");
            System.out.println("<5.SAIR                             >");
            System.out.println(" ---------------------------------");

            option = input.nextInt();

            while(option<1 || option>5 ){
                System.out.println("Choose an valid option");
                option = input.nextInt();
            }
            switch(option){
                case 1: //Enviar mensagem tentar fazer texto escrito em consola, mas também texto de um ficheiro
                    System.out.println("1. SEND FROM TEXT \n 2.SEND FROM TEXT FROM FILE");
                    option = input.nextInt();
                    while(option<1 || option>2){
                        System.out.println("Choose an valid option");
                        option = input.nextInt();
                    }
                    if(option == 1){
                        start_clock = System.currentTimeMillis();
                        String To_Send = input.nextLine();
                        byte[] Text_Send = To_Send.getBytes(StandardCharsets.UTF_8);
                        Selected_Port.writeBytes(Text_Send, Text_Send.length);
                        turnoff_clock = System.currentTimeMillis();
                        total_time = turnoff_clock - start_clock;
                    }
                    if(option == 2){

                    }
                    break;
                case 2: //Enviar imagem (introduzir path)
                    break;
                case 3: //
                    break;
                case 4: //
                    break;
                case 5: //Sair do programa
                    Selected_Port.closePort();
                    System.exit(0);
                    break;
            }
        }



    public static int VerifyPort(int NeedVerification_Port, SerialPort[] ArrayofPorts){
            int tobereturned = 0;
            boolean Port_Verified = false;
            Scanner input = new Scanner(System.in);
            while(!Port_Verified) {
            SerialPort Verication_Port = ArrayofPorts[NeedVerification_Port];
            if(Verication_Port.isOpen()){
                Port_Verified = true;
                tobereturned = NeedVerification_Port;
                return tobereturned;
            }
            else{
                System.out.println("It seems that this port is closed. \n Choose another one.");
                NeedVerification_Port = input.nextInt();
            }
            }
            return tobereturned;
    }

    public void SendText_File(SerialPort Selected_Port){

            byte[] Text_File(Files.readAllBytes("C:/"))
    }

    public void SendImage(SerialPort Selected_Port){

    }

    public static String OS_USED(){
        String what_system = System.getProperties().toString();
    }
    }



/*
  _|_|_|  _|  _|
_|        _|  _|
_|  _|_|  _|_|_|_|
_|    _|      _|
  _|_|_|      _|   */