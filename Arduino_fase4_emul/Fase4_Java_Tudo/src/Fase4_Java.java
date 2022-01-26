/*
 *   MIETI 2021/2022 EMULAÇÃO E SIMULAÇÃO DE REDES DE TELECOMUNICAÇÕES
 *   GRUPO 4 - FASE 1.2
 *   CATARINA AMORIM A93094
 *   JOÃO MOURA A93099
 *   LUÍS PINTO A94155
 *   MIGUEL PEREIRA A94152
 * *  */

import java.awt.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.InputMismatchException;
import java.util.Scanner;
import com.fazecast.jSerialComm.SerialPort;
import javax.swing.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.lang.*;

public class Fase4_Java extends JFrame {


    /*public Tester() {
        JFrame frame = new JFrame();
        JPanel Jpanel_chat = new JPanel();
        Jpanel_chat.setBorder(BorderFactory.createEmptyBorder());
        Jpanel_chat.setLayout(new GridLayout(0,1));

        frame.add(Jpanel_chat, BorderLayout.CENTER);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setTitle("CHAT");
        frame.pack();
        frame.setVisible(true);
    }*/

    public static void main(String[] args) throws IOException {
       /* SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                new chatbox();
            }
        });*/
        int option; //para opções
        long start_clock; //para medições de tempo
        long turnoff_clock;
        Scanner input = new Scanner(System.in);
        long total_time;
        String OS_BEING_USED = System.getProperty("os.name");
        SerialPort[] AvailablePorts;
        boolean Sair = false;

        int counter = 0;
        //ver disponibilidade das portas (getCommPorts / check ports/
        AvailablePorts = SerialPort.getCommPorts();
        if (AvailablePorts.length == 0) {
            System.out.println("No Ports Available");
            System.out.println("Press Any key to Exit.");
            String exit_option = input.nextLine();
            System.exit(1);
        }

        //Selecionar Port
        for (SerialPort ToChoose_Port : AvailablePorts) {
            counter++;
            System.out.println(counter - 1 + " - " + ToChoose_Port.getSystemPortName());
        }
        int Sent_Port = input.nextInt();
        int Choosen_port = VerifyPort(Sent_Port, AvailablePorts);
        SerialPort Selected_Port = AvailablePorts[Choosen_port]; //É necessário verificar se o port está aberto


        //configuração da porta série (baudrate, bits (8), stop bits (1), paridade (0))
        Selected_Port.setComPortParameters(115200, 8, 1, 0);
        try {
            Thread.sleep(2000); //Como ao selecionarmos a porta o arduino é resetado, temos de esperar que ele renicie, logo são dados 2 segundos para o mesmo ser feito
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        do {
            Selected_Port.openPort();
            String File_Store;
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
            System.out.println("<6.CHAT                             >");
            System.out.println("<7.SAIR                             >");
            System.out.println(" ---------------------------------");

            option = input.nextInt();

            while (option < 1 || option > 7) {
                System.out.println("Choose an valid option");
                option = input.nextInt();
            }

            switch (option) {
                case 1: //Enviar mensagem tentar fazer texto escrito em consola, mas também texto de um ficheiro
                    start_clock = System.currentTimeMillis();
                    SendText_File(Selected_Port);
                    turnoff_clock = System.currentTimeMillis();
                    total_time = -(start_clock - turnoff_clock);
                    System.out.println("This message took " + total_time + "ms to be sent");
                    Selected_Port.closePort();
                    break;
                case 2: //Enviar imagem (introduzir path)
                    start_clock = System.currentTimeMillis();
                    SendImage(Selected_Port);
                    turnoff_clock = System.currentTimeMillis();
                    total_time = -(start_clock - turnoff_clock);
                    System.out.println("This image took " + total_time + "ms to be sent");
                    Selected_Port.closePort();
                    break;
                case 3: //Receber Mensagem de texto
                    System.out.println("Introduza o path exato onde quer que o ficheiro de texto seja guardado");
                    File_Store = input.nextLine();
                    //File_Store = "/Files_Java/Receber/Hinorecived.txt";
                    Receive_Text(Selected_Port, File_Store);
                    System.out.println("I arrived");
                    Selected_Port.closePort();
                    break;
                case 4: //Receber Imagem
                    System.out.println("Introduza o path exato onde quer que a imagem seja guardado");
                    File_Store = input.nextLine();
                    Receive_Image(Selected_Port, File_Store);
                    System.out.println("I arrived");
                    Selected_Port.closePort();
                    break;
                case 5: //Verificar Conexão verifica se o port ainda está ligado, e envia uma mensagem de teste entre o arduinos.

                    break;
                case 6:
                    chat(Selected_Port);
                    Selected_Port.closePort();
                    break;
                case 7: //Sair do programa
                    Selected_Port.closePort();
                    Sair = true;
                    System.out.println("The Program will exit now.");
                    System.exit(0);
                    break;
            }
        } while (!Sair);
    }

    // Verifica se o Port selecionado está aberto e caso não esteja o utilizador pode sair do programa ou então escolher outro port.
    public static int VerifyPort(int NeedVerification_Port, SerialPort[] ArrayofPorts) {
        int tobereturned = 0;
        boolean Port_Verified = false;
        Scanner input = new Scanner(System.in);
        while (!Port_Verified) {
            SerialPort Verication_Port = ArrayofPorts[NeedVerification_Port];
            Verication_Port.openPort();
            if (Verication_Port.openPort()) {
                Port_Verified = true;
                tobereturned = NeedVerification_Port;
                return tobereturned;
            } else {
                System.out.println("It seems that this port is closed. \n Choose another one.");
                NeedVerification_Port = input.nextInt();
            }
        }
        return tobereturned;
    }

    //Função para enviar texto contido em ficheiro entre os DevKits
    public static void SendText_File(SerialPort Selected_Port) throws IOException {
        //String OS_BEING_USED = OS_USED();
        int nPackets; //A quantidade de pacotes a enviar após a divisão da mensagem em tramas de 30 bits
        float BytesFromFile;
        boolean sucesso = false;
        int String_size; //Cantidade de bytes contida no ficheiro de texto
        int text_newpos = 0; //A cada iteração a posição do array que contem o texto tem de ser incrementada
        int counter = 0;
        int insert_counter = 0; // counter para a inserção dos dados do ficheiro na payload a enviar
        Scanner input2 = new Scanner(System.in);
            boolean sucesso_envio = false;
            int size_contador = 0;
            /*  if(OS_BEING_USED == 1){
            }
            if(OS_BEING_USED == 2){
            } Implementar caso haja tempo */
            System.out.println("Introduza o path exato do ficheiro de texto a enviar");
            String Path = input2.nextLine();
            //String Text_File2 = (Files.readString(Paths.get("/home/johnnyminho/IdeaProjects/Files_Java/Boas")));
            byte[] Text_File = (Files.readAllBytes(Paths.get(Path)));
            //OutputStream output = Selected_Port.getOutputStream();
            String_size = Text_File.length;
            BytesFromFile = Text_File.length;
            nPackets = (int) Math.ceil(BytesFromFile / 30);
            System.out.println(" " + nPackets);
            for (counter = 0; counter < nPackets; counter++) {
                sucesso_envio = false;
                byte[] bytesToSend = new byte[30];
                if (String_size > 30) {
                    size_contador = 30;
                    String_size -= 30;
                } else {
                    size_contador = String_size;
                }
                for (insert_counter = 0; insert_counter < size_contador; insert_counter++) {
                    bytesToSend[insert_counter] = Text_File[insert_counter + text_newpos];
                    System.out.println(" " + (char) bytesToSend[insert_counter]);
                }
                Selected_Port.writeBytes(bytesToSend, size_contador);
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                text_newpos = text_newpos + 30; //proxima posição no ficheiro de texto será 30 bytes à frente da que foi enviad
                while (!sucesso_envio) {
                    byte[] bufferackfinal = new byte[2];
                    Selected_Port.readBytes(bufferackfinal, 1);
                    System.out.println(" " + bufferackfinal[0]);
                    if (bufferackfinal[0] == 'B') {
                        System.out.println("ACK RECEBIDO");
                        sucesso_envio = true;
                    }
                }
            }
        }


    //Função para enviar imagens entre os DevKits
    public static void SendImage(SerialPort Selected_Port) throws IOException {
        int nPackets; //A quantidade de pacotes a enviar após a divisão da mensagem em tramas de 30 bits
        float BytesFromFile;
        boolean sucesso = false;
        int Image_size; //Cantidade de bytes contida no ficheiro de texto
        int Image_newpos = 0; //A cada iteração a posição do array que contem o texto tem de ser incrementada
        int counter = 0;
        int insert_counter = 0; // counter para a inserção dos dados do ficheiro na payload a enviar
        Scanner input_imagem = new Scanner(System.in);
        boolean sucesso_envio = false;
        int size_contador = 0;
            /*  if(OS_BEING_USED == 1){
            }
            if(OS_BEING_USED == 2){
            } Implementar caso haja tempo */
        System.out.println("Introduza o path exato da imagem a enviar");
        String Path = input_imagem.nextLine();
        //String Text_File2 = (Files.readString(Paths.get("/home/johnnyminho/IdeaProjects/Files_Java/Boas")));
        byte[] Text_File = (Files.readAllBytes(Paths.get(Path)));
        //OutputStream output = Selected_Port.getOutputStream();
        Image_size = Text_File.length;
        BytesFromFile = Text_File.length;
        nPackets = (int) Math.ceil(BytesFromFile / 30);
        System.out.println(" " + nPackets);
        for (counter = 0; counter < nPackets; counter++) {
            sucesso_envio = false;
            byte[] bytesToSend = new byte[30];
            if (Image_size > 30) {
                size_contador = 30;
                Image_size -= 30;
            } else {
                size_contador = Image_size;
            }
            for (insert_counter = 0; insert_counter < size_contador; insert_counter++) {
                bytesToSend[insert_counter] = Text_File[insert_counter + Image_newpos];
                System.out.println(" " + (char) bytesToSend[insert_counter]);
            }
            Selected_Port.writeBytes(bytesToSend, size_contador);
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            Image_newpos = Image_newpos + 30; //proxima posição no ficheiro de texto será 30 bytes à frente da que foi enviad
            while (!sucesso_envio) {
                byte[] bufferackfinal = new byte[2];
                Selected_Port.readBytes(bufferackfinal, 1);
                System.out.println(" " + bufferackfinal[0]);
                if (bufferackfinal[0] == 'B') {
                    System.out.println("ACK RECEBIDO");
                    sucesso_envio = true;
                }
            }
        }
    }


    public static void Receive_Text(SerialPort Selected_Port, String File_Store) throws IOException {
        boolean sucesso = false;
        File file_save = new File(File_Store);
        byte[] data = new byte[30];
        while (!sucesso) {
            while (Selected_Port.bytesAvailable() < 30) ;
            Selected_Port.readBytes(data, 30);
            if (data[0] == 'F' && data[1] == 'I' && data[2] == 'M')
                Files.write(Path.of(file_save.getPath()), data);

        }
    }

    public static void Receive_Image(SerialPort Selected_Port, String File_Store) throws IOException {
        boolean sucesso = false;
        File file_save = new File(File_Store);
        byte[] data = new byte[30];
        while (!sucesso) {
            while (Selected_Port.bytesAvailable() < 30) ;
            Selected_Port.readBytes(data, 30);
            if (data[0] == 'F' && data[1] == 'I' && data[2] == 'M') {
                sucesso = true;
            }
        }
        Files.write(Path.of(file_save.getPath()), data);
    }

    public static void chat(SerialPort Selected_Port) throws IOException {
        Scanner inputchat = new Scanner(System.in);
        String to_send;
        boolean Leave_chat = false;
        boolean valid_op = false;
        int option_inp = 0;
        do {
            valid_op = false;
            System.out.println(" ____________________________________________________");
            System.out.println("|     1-> ENVIAR MENSAGEM                            |");
            System.out.println("|     2-> RECEBER MENSAGEM                           |");
            System.out.println("|     3-> SAIR  DO CHAT                              |");
            System.out.println(" ----------------------------------------------------");
            System.out.println("USER: " + Selected_Port);
            if (inputchat.hasNextInt()) {
                option_inp = inputchat.nextInt();
                valid_op = true;
            }
            while (option_inp < 1 || option_inp > 3 || !valid_op) {
                System.out.println("Opção Inválida");
                option_inp = inputchat.nextInt();
            }
            switch (option_inp) {
                case 1:
                    System.out.println("Digite a mensagem que quer enviar: ");
                    to_send = inputchat.next();
                    byte[] byte_send = to_send.getBytes(StandardCharsets.UTF_8);
                    int counter = 0;
                    int size_contador = 0;
                    int text_newpos = 0;
                    int insert_counter = 0;
                    boolean sucesso_envio = false;
                    int String_size = byte_send.length;
                    float BytesFromFile = byte_send.length;
                    int nPackets = (int) Math.ceil(BytesFromFile / 30.0);
                    System.out.println(" " + nPackets);
                    for (counter = 0; counter < nPackets; counter++) {
                        sucesso_envio = false;
                        byte[] bytesToSend = new byte[30];
                        if (String_size > 30) {
                            size_contador = 30;
                            String_size -= 30;
                        } else {
                            size_contador = String_size;
                        }
                        for (insert_counter = 0; insert_counter < size_contador; insert_counter++) {
                            bytesToSend[insert_counter] = byte_send[insert_counter + text_newpos];
                        }
                        Selected_Port.writeBytes(bytesToSend, size_contador);
                        try {
                            Thread.sleep(100);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        text_newpos = text_newpos + 30; //proxima posição no ficheiro de texto será 30 bytes à frente da que foi enviad
                        while (!sucesso_envio) {
                            byte[] bufferackfinal = new byte[2];
                            Selected_Port.readBytes(bufferackfinal, 1);
                            System.out.println(" " + bufferackfinal[0]);
                            if (bufferackfinal[0] == 'B') {
                                sucesso_envio = true;
                            }
                        }
                    }
                    break;
                case 2:
                    boolean sucesso = false;
                    int counter_receber = 30;
                    byte[] data = new byte[30];
                    while (!sucesso) {
                        while (Selected_Port.bytesAvailable() < 30);
                        Selected_Port.readBytes(data, 30);
                        if(data[0] == 'F' && data[1] == 'I' && data[2] == 'M'){
                            sucesso = true;
                            for(counter_receber = 0; counter_receber < 30; counter_receber++){
                                System.out.print("Recebido: ");
                                System.out.print(" " + (char)data[counter_receber]);
                            }
                        }
                    }
                    break;
                case 3:
                    Leave_chat = true;
                    break;
            }
        } while (!Leave_chat);
    }
}