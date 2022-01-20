/*
 *   MIETI 2021/2022 EMULAÇÃO E SIMULAÇÃO DE REDES DE TELECOMUNICAÇÕES
 *   GRUPO 4 - FASE 1.2
 *   CATARINA AMORIM A93094
 *   JOÃO MOURA A93099
 *   LUÍS PINTO A94155
 *   MIGUEL PEREIRA A94152
 * *  */

import java.awt.*;
import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.Scanner;
import com.fazecast.jSerialComm.SerialPort;
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
        String OS_BEING_USED = System.getProperty("os.name");
        boolean You_Pass;

        //ver disponibilidade das portas (getCommPorts / check ports/
        SerialPort[] AvailablePorts = SerialPort.getCommPorts();
        if(AvailablePorts.length == 0){
            System.out.println("No Ports Available");
            System.out.println("Press Any key to Exit. \n If You wish to restart press R");
            String exit_option = input.nextLine();
            if(exit_option == "R" || exit_option == "r"){
                //Implementar reset
            }
            else{
                System.exit(1);
            }


        }

        //Selecionar Port
        for(SerialPort ToChoose_Port : AvailablePorts){
            counter++;
            System.out.println(counter-1 + " - " + ToChoose_Port.getSystemPortName());
        }
        int Sent_Port = input.nextInt();
        int Choosen_port = VerifyPort(Sent_Port,AvailablePorts);
        SerialPort Selected_Port = AvailablePorts[Choosen_port]; //É necessário verificar se o port está aberto


        //configuração da porta série (baudrate, bits (8), stop bits (1), paridade (0))
        Selected_Port.setComPortParameters(115200, 8, 1 , 0);


        //while(true){
        if (OS_BEING_USED.contains("win")) {
            System.out.print("\033[H\033[2J"); //limpar consola
            System.out.flush();
            System.out.println("SYSTEM BEING USED: " + OS_BEING_USED);
        }
        if (OS_BEING_USED.contains("nix") || OS_BEING_USED.contains("nux") || OS_BEING_USED.contains("aix")) {
            System.out.print("\033[H\033[2J"); //limpar consola
            System.out.flush();
            System.out.println("SYSTEM BEING USED: " + OS_BEING_USED);
        }

        System.out.println("SELECTED PORT: " + Selected_Port);
        System.out.println(" _________________________________");
        System.out.println("<1.ENVIAR TEXTO                     >");
        System.out.println("<2.ENVIAR IMAGEM                    >");
        System.out.println("<3.RECEBER TEXTO                    >");
        System.out.println("<4.RECEBER IMAGEM                   >");
        System.out.println("<5.VERIFICAR CONEXÃO                >");
        System.out.println("<6.SAIR                             >");
        System.out.println(" ---------------------------------");

        option = input.nextInt();

        while (option < 1 || option > 6) {
            System.out.println("Choose an valid option");
            option = input.nextInt();
        }

        switch (option) {
            case 1: //Enviar mensagem tentar fazer texto escrito em consola, mas também texto de um ficheiro
                System.out.println("1. SEND FROM TEXT \n 2.SEND FROM TEXT FROM FILE");
                option = input.nextInt();
                while (option < 1 || option > 2) {
                    System.out.println("Choose an valid option");
                    option = input.nextInt();
                }
                start_clock = System.currentTimeMillis();
                SendText_File(Selected_Port, option);
                turnoff_clock = System.currentTimeMillis();
                total_time = -(start_clock - turnoff_clock);
                System.out.println("This message took " + total_time + "ms to be sent");
                break;
            case 2: //Enviar imagem (introduzir path)
                start_clock = System.currentTimeMillis();
                SendImage(Selected_Port);
                turnoff_clock = System.currentTimeMillis();
                total_time = -(start_clock - turnoff_clock);
                System.out.println("This image took " + total_time + "ms to be sent");
                break;
            case 3: //Receber Mensagem de texto
                byte[] buffer_toreceive = new byte[385];
                String File_Store = "C:/TesteEmul/Hinorecived.txt";
                Receive_Text(Selected_Port, buffer_toreceive, 385, File_Store);
                System.out.println("I arrived");
                Selected_Port.closePort();
                break;
            case 4: //Receber Imagem
                byte[] buffer_toreceiveimage = new byte[5120];
                String File_StoreImage = "C:/TesteEmul/Aviaorecived.jpg";
                Receive_Image(Selected_Port, buffer_toreceiveimage, 5120, File_StoreImage);
                System.out.println("I arrived");
                Selected_Port.closePort();
                break;
            case 5: //Verificar Conexão verifica se o port ainda está ligado, e envia uma mensagem de teste entre o arduinos.

                break;
            case 6: //Sair do programa
                Selected_Port.closePort();
                System.out.println("The Program will exit now.");
                System.exit(0);
                break;
        }
    }



    // Verifica se o Port selecionado está aberto e caso não esteja o utilizador pode sair do programa ou então escolher outro port.
    public static int VerifyPort(int NeedVerification_Port, SerialPort[] ArrayofPorts){
        int tobereturned = 0;
        boolean Port_Verified = false;
        Scanner input = new Scanner(System.in);
        while(!Port_Verified) {
            SerialPort Verication_Port = ArrayofPorts[NeedVerification_Port];
            Verication_Port.openPort();
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
    //Função para enviar texto contido em ficheiro entre os DevKits
    public static void SendText_File(SerialPort Selected_Port, int option) throws IOException{
        //String OS_BEING_USED = OS_USED();
        //  int buffer_size = Text_File.length; a usar futuramente
        Scanner input = new Scanner(System.in);
        if(option == 1){
            input.nextLine();
            String To_Send = input.nextLine();
            byte[] Text_Send = To_Send.getBytes(StandardCharsets.UTF_8);
            Selected_Port.writeBytes(Text_Send, Text_Send.length);
        }
        if(option == 2){
            /*  if(OS_BEING_USED == 1){
            }
            if(OS_BEING_USED == 2){
            } Implementar caso haja tempo */
            byte[] Text_File = (Files.readAllBytes(Paths.get("C:/TesteEmul/Hino.txt")));
            Selected_Port.writeBytes(Text_File, 385); //Definir ficheiro e tamanho
        }
    }

    //Função para enviar imagens entre os DevKits
    public static void SendImage(SerialPort Selected_Port) throws IOException{
        byte[] Image_to_byte = Files.readAllBytes(Paths.get("C:/TesteEmul/Aviao.jpg"));
        Selected_Port.writeBytes(Image_to_byte, 5120);
    }

    public static void Receive_Text(SerialPort Selected_Port, byte[] buffer,int size, String File_Store) throws IOException{
        while(Selected_Port.bytesAvailable()<size){
            System.out.println("RECEIVING MESSAGE. AWAIT"); //O Programa espera até que todos os bytes sejam recebidos
        }
        Selected_Port.readBytes(buffer,size);
        Files.write(new File(File_Store).toPath(), buffer);
    }
    public static void Receive_Image(SerialPort Selected_Port, byte[] buffer,int size, String File_Store) throws IOException{
        while(Selected_Port.bytesAvailable()<size){
            //System.out.println(Selected_Port.bytesAvailable());
            System.out.println("RECEIVING MESSAGE. AWAIT"); //O Programa espera até que todos os bytes sejam recebidos
        }
        Selected_Port.readBytes(buffer,size);
        Files.write(new File(File_Store).toPath(), buffer);
    }

}



/*
  _|_|_|  _|  _|
_|        _|  _|
_|  _|_|  _|_|_|_|
_|    _|      _|
  _|_|_|      _|
  WORK DONE BY G4*/