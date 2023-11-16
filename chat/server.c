//Quellen: 
//1. http://beej.us/guide/bgnet/pdf/bgnet_a4_c_2.pdf
//2. https://openbook.rheinwerk-verlag.de/c_von_a_bis_z/025_c_netzwerkprogrammierung_001.htm#mja8101c6e0e4cb2e6fd8312114dad30d7
//3. https://www.youtube.com/watch?v=O5Mpj1ixzxY
//4. https://www.youtube.com/watch?v=dquxuXeZXgo
//5. https://www.youtube.com/watch?v=l7RQJxwAmsU
//6. https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-ioctlsocket
//7. https://docs.microsoft.com/de-de/windows/win32/winsock/windows-sockets-error-codes-2?redirectedfrom=MSDN
//8. https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-listen
//9. https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect
//10.https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select
//11.https://man7.org/linux/man-pages/man7/ipv6.7.html
//12.https://pubs.opengroup.org/onlinepubs/7908799/xsh/select.html
//13.https://www.youtube.com/watch?v=hFt9Mn_dui0
//14.https://docs.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-fd_set
//14.http://www.andreadrian.de/select/index.html
//15.https://www.geeksforgeeks.org/write-header-file-c/
//16.https://www.youtube.com/watch?v=qM2BAjhbUg0
//17.https://www.youtube.com/watch?v=O5Mpj1ixzxY
//18.https://coderedirect.com/questions/355438/using-stdin-with-select-in-c




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32 //falls es sich um ein Windwos Betriebssystem handelt
/* Headerfiles für Windows */
#include <winsock2.h> //ermöglicht Sockets + send() + read()
#include <Windows.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <In6addr.h>
#include <io.h>         //input ouput streams


#else //falls es sich um ein Unix Betriebssystem handelt
//##Includes für Linux/Unix
#include <sys/types.h> //sollte vor sys/socket indlude stehen (Linux)
#include <sys/socket.h> //The <sys/socket.h> header defines the sockaddr structure//https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h> //zum nicht-blockieren setzen der SOcket

#endif

//include für beide
#include "message.h" //struct für message

//###############################################################
//HOW TO COMPILE AND RUN
//###############################################################

//compilieren in WINDOWS mit: gcc -Wall -o server server.c -lws2_32
//bzw für UNIX: gcc -Wall -o server server.c

//ACHTUNG: Erst Server ausführen dann client
//ausführen WINDOWS: ./server.exe 1234 s82099
//ausführen UNIX: ./server ::1 1234 s82099          

int main(int argc, char * argv[])
{
    //###############################################################
    // CHECK ARGUMENTS AND PREPARE VARIABLES
    //###############################################################
    //falls nicht genügend Parameter mit Programmaufruf übergeben wurden, wird Programm verlassen
    //für den Server soll die Portnummer übergeben werden => argv[1] als auch die S-Nummer des Studierenden => argv[2] übergeben werden
    if (argc <3)
    {
        printf("Server--> Please call programm with command-line inputs: \n<filename.c> \n<portno>(MUST BE IDENTICAL TO SERVER PORT) \nYOUR <s_number>" );
        exit(1);
    }

    
    //###############################################################
    //Initialisiere Variable für Portnummer
    unsigned int portno;
    
    //die als String übergebene Portnummer (argv[1]) wird mit atoi in int umgewandelt und in portno gespeichert
    portno= atoi(argv[1]);          
    
    //###############################################################
    //eine Variable der Struktur wie in messag.h definiert wird angelegt
    message msg_server; 
    
    strcpy(msg_server.snummer, argv[2]); //snummer des Clients in buffer speichern   

    //###############################################################
    //Hilfvariable zur überprüfung, ob Funktionen erfolgreich waren
    int err; 
    
    
    //###################################################################################
    //PREPARE SOCKET (nur für Windows)
    //###################################################################################
    // int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);

    //Erklärung siehe Client
    #ifdef _WIN32
        WORD wVersionRequested; //Datentyp WORD (unsigned short) mit 2 Bytes Länge.
        WSADATA wsaData; //Datentyp der Informationen über die Winodws Sockets implementierung enthält
        

        wVersionRequested = MAKEWORD(2, 2); //bestimmt die Winsockversion, die gestartet werden soll. Hier Version 2.2 da Makeword die beiden Ziffer so verbindet

        //#########################################################
        //Nutzung von Winsock vorbereiten
        err = WSAStartup(wVersionRequested, &wsaData);

        //#########################################################
        //Test ob WSAStartup() erfolgreich war
        if (err != 0) 
        {
            printf("Server--> WSAStartup failed with error: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        else printf("\nServer-->WSAStartup successfull\n");
    #endif
    

    //###################################################################################
    //INITIALIZE AND START SOCKET
    //###################################################################################
    // SOCKET  socket(int af, int type, int protocol); ==> Windows
    // int  socket(int af, int type, int protocol); ==> Unix

    #ifdef _WIN32
        SOCKET socketfd;
    #else
        int socketfd;
    #endif
    
    socketfd = socket( PF_INET6, SOCK_STREAM , 0); //socketfd = socket file descdriptor (enthält int-wert mit dem socket addressiert/aufgerufen werden kann)


    //#########################################################
    //Prüfung, ob Socket erfolgreich erstellt wurde

    #ifdef _WIN32
        if (socketfd == INVALID_SOCKET)
        {
            printf("\nServer--> Socket creation unsuccesfull ");
            exit(EXIT_FAILURE);
        }
    #else
        if (socketfd < 0)
        {
            printf("\nServer--> Socket creation unsuccesfull");
            exit(EXIT_FAILURE);
        }
    #endif 
        else printf("\nServer-->Socket creation successfull\n");

    //#########################################################
    //SET SOCKET TO NONBLOCKING
    //#########################################################
    
    u_long optval = 1; //ungleich 0 bedeutet nicht blockieren
    #ifdef _WIN32    
        err = ioctlsocket(socketfd, FIONBIO, &optval);
    #else
        err = ioctl(socketfd, FIONBIO, &optval);
    #endif

    if (err != 0)
    {
        printf("\nServer-->failed to set non-blocking\n");
        exit(EXIT_FAILURE);
    }
    else printf("\nServer-->Socket non-blocking successfull\n");

    //#########################################################
    //SET SOCKET ADDRESSINFORMATION
    //#########################################################
    
    struct sockaddr_in6 serv_addr; //anlegen einer Varibale der Struktru serv_addr_in

    
    memset(&serv_addr, 0, sizeof(serv_addr));//Variable der Serveraddresse leeren           
    

    serv_addr.sin6_family= PF_INET6;

    serv_addr.sin6_addr= in6addr_any; // Jede IP-Adresse ist gültig

    serv_addr.sin6_port=htons(portno);  


    //#########################################################
    //BIND SERVER SOCKET TO ADDRESS
    //#########################################################
    //Bind socket to the address and port number specified in structure
    //Socket soll an eine IP-Adresse und Port-Nummer gebunden werden, so dass sie über diese erreichbar ist
    //Sobald dann ein Datenpaket eingeht, erkennt das Betriebssystem anhand der Portnummer, für welchen Socket das Paket ist.
    //Argumente:
    //1. sockfd = der Socket Descriptor enthält den "Namen" der zu bindenden Socket
    //2.1 (struct sockaddr *) = casted die Struktur serv_addr vom Typ sockaddr_in auf sockaddr
    //2.2 &serv_addr = Zeiger auf Adresse und Portnummer (in struct Varibale serv_addr gespeichert)
    //3. sizeof(serv_addr) = Länge der Struktur als Datentyp socklen_t
    //bind() liefert im Falle eines Fehlers –1 (gleichwertig mit dem Fehlercode SOCKET_ERROR unter MS-Windows)

    err = bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    //#########################################################
    //Prüfung, ob bind() der Socket erfolgreich war  
    if (err < 0) 
    {
            printf("\nServer--> bind() function unsuccesfull ");
            exit(EXIT_FAILURE);
    }
    else printf("\nServer-->bind() function successfull\n");

    //#########################################################
    //LISTEN TO INCOMING CONNECTIONS
    //#########################################################
    // Warteschlange für eingehende Verbindungen einrichten
    // Dabei wird die Programmausführung des Servers so lange unterbrochen, bis ein Verbindungswunsch eintrifft. 
    // Mit listen() lassen sich durchaus mehrere Verbindungswünsche »gleichzeitig« einrichten.
    // Mit dem ersten Parameter geben Sie wie immer den Socket-Deskriptor an und mit dem zweiten Parameter die Länge der Warteschlange. Die Länge der Warteschlange ist 
    // die maximale Anzahl von Verbindungsanfragen, die in eine Warteschlange gestellt werden, wenn keine Verbindungen mehr angenommen werden können.

    err = listen(socketfd, 5);

    //#########################################################
    //Prüfung, ob listen() der Socket erfolgreich war  
    
    if (err < 0) 
    {
            printf("Server--> listen() function unsuccesfull ");
            exit(EXIT_FAILURE);
    }
    else printf("\nServer-->listen() function successfull\n");

    // //#########################################################
    // //INITIALIZE FILE DESCRIPTOR SETS 
    // //#########################################################

    fd_set master; //Soll alle socket file descriptoren enthalten, die sich verbinden wollen
    FD_ZERO(&master);  //File Descriptor Set sicherheitshalber leeren
    fd_set read_fds; //Dieses File Descriptor Set soll alle Socket File Descriptoren enthalten die etwas zum lesen bereitstellen
    FD_ZERO(&read_fds); //auch dieses File Descriptor Set sicherheitshalber leeren


    FD_SET(socketfd, &master); //client socket zur master fd_set hinzufügen
    
    int fd_max = socketfd; //den Socket zähler auf die aktuell höchste Zahl setzen Da es nur socketfd gibt, hat diese den aktuell höchsten Wert

    //###################################################################################
    //SEND AND RECEIVE DATA
    //###################################################################################
    
    while(1)
    {
              

        FD_SET(0, &master); // Stdin zu Master hinzufügen ACHTUNG: NUR UNIX
        
        read_fds = master; //da select() function das master fd_set zerstoeren würde, es würde nur noch die lesebereiten Socket FDs entahlten, 
        //schaffen wir eine Kopie davon mit der wir weiterarbeiten werden 
        //read_fds soll nach select nur noch die sockets enthalten, welche gerade etwas senden möchten
        //Mit diesem Parameter wird innerhalb der select() function überprüft, ob auf den (Socket-)Deskriptoren Daten zum Lesen vorhanden sind.
                
        // Mit dem ersten Parameter n geben Sie die Größe der folgenden Menge an. 
        // Hierfür wird gewöhnlich der Wert des höchsten (Socket-)Deskriptors plus eins angegeben. 
        // Welche Nummer der nächste (Socket-)Deskriptor verwendet, entscheidet immer noch das System. Daher empfiehlt es sich, jeden gesetzten 
        // (Socket-)Deskriptor mit dem zu vergleichen, der rein theoretisch der höchste ist.
        // Die nächsten drei Parameter sind Zeiger auf die fd_sets, die zum Lesen, Schreiben oder auf Ausnahmen getestet werden. Sofern Sie einen der Parameter nicht verwenden wollen, können Sie hierfür NULL angeben. 
        // Drei getrennte Sets sind nötig, da man ja nicht alle (Socket-)Deskriptoren auf Lesen oder Schreiben testen möchte.
        
        if (select(fd_max+1, &read_fds, NULL, NULL, NULL) == -1) 
        {
            perror("select");
            exit(4);
        }

        for(int i = 0; i <= fd_max; i++)
        {
            
            //#########################################################
            //HANDLE NEW CONNECTIONS, INCOMING AND OUTGOING MESSAGES
            //#########################################################
            if (FD_ISSET(i, &read_fds)) //falls Socket lesebereit
            {
                //#########################################################
                //NEW CONNECTION
                if (i == socketfd) //falls listener Socket lesebereit ist, bedeutet dies, es gibt eine neue Vebrindungsanfrage von CLient
                {
                    struct sockaddr_in6 client_addr;
                    memset(&client_addr, 0, sizeof(struct sockaddr_in6));//Variable der Serveraddresse leeren
                    
                    #ifdef _WIN32
                        //Unix kennt Datentyp Socket nicht, Windows braucht diesen aber zwingend
                        //Windws kennt Datentyp socklen_t für Addressgröße nicht,
                        int len = sizeof(client_addr);
                        SOCKET client_socketfd = accept(socketfd, (struct sockaddr*)&client_addr, &len);
                        
                    #else
                        //Windwos braucht Datentyp Socket statt int. Unix kennt diesen nicht
                        //Windws kennt Datentyp socklen_t nicht, Unix braucht diesen
                        socklen_t len = sizeof(client_addr);
                        int client_socketfd = accept(socketfd, (struct sockaddr*)&client_addr, &len);
                    #endif

                    if (client_socketfd < 0) 
                    {
                        printf("\nServer--> accept() function unsuccesfull ");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        FD_SET(client_socketfd, &master); // add to master set
                        if (client_socketfd > fd_max) 
                        {    // Zähler für Select immer auf höchste Wert setzen, damit select() kein Socket "übersieht"
                            fd_max = client_socketfd;
                        }                      
                    }
                }
                //#########################################################
                //INCOMING
                else if(i != 0) //falls nicht listenere Socket und nicht STDIN, muss es sich um Daten eines Clients handeln
                {
                    memset(&msg_server.text, 0, 1024); 
                    // Daten von Client in Message Stuct einlesen
                    err = recv(i, &msg_server, sizeof(msg_server), 0);

                    //#########################################################
                    //Prüfung, ob recv() der Socket erfolgreich war        
                    if (err < 0) 
                    {
                        printf("Server--> recv() function unsuccesfull ");
                        #ifdef _WIN32
                            closesocket(i); // bye!
                        #else
                            close(i); // bye!
                        #endif
                        FD_CLR(i, &master); //Von Master Set entfernen da Verbindung nicht mehr aktuell
                    } 
                    else printf("Rcvd from %s--> %s", msg_server.snummer, msg_server.text);
                }
                //#########################################################
                //OUTGOING
                else if (i == 0) //falls es sich um STDIN handelt, sende Daten an Client
                {   
                    
                    memset(&msg_server.text, 0, 1024); //text feld leeren
                    fgets(msg_server.text, 1024, stdin);
                    
                    for(int j = 1; j <= fd_max; j++) 
                    {
                        if(FD_ISSET(j, &master))
                        {
                            if (j != socketfd && j != i) //Message von STDIN nicht an STDIN und Server Socket senden
                            {
                                err = send(j, &msg_server, sizeof(msg_server), 0);//senden an i socket    

                                //#########################################################
                                //Prüfung, ob send() der Socket erfolgreich war        
                                if (err < 0) 
                                {
                                    printf("Server--> send() function unsuccesfull ");
                                    #ifdef _WIN32
                                        closesocket(i); // Socket schließen
                                    #else
                                        close(i); // Socket schließen
                                    #endif
                                    FD_CLR(i, &master); //Von Master Set entfernen da Verbindung nicht mehr aktuell
                                } 
                            }
                        }                        
                    }   
                }
            }    
        }
        //#########################################################
        //END CHAT
        //#########################################################
        //Ende des Chats erkennen
        //Wird eine Nachricht gesendet, die "bye" enthält, wird die while-Schleife bzw. Verbindung beendet
        //strncmp = String compare n Zeichen = vergleicht erste 3 Zeichen des Buffers mit "bye" 
        //Wenn sie übereinstimmen wird 0 zurückgegeben. Daher ! = not
        if(!strncmp("bye", msg_server.text,3))
        {
            break;
        }

    }

    //#########################################################
    //SOCKETS SCHLIESSEN UND CLEANUP
    //#########################################################

    //Server Socket schließen
    
    #ifdef _WIN32
        err = closesocket(socketfd);
        if (err < 0) 
        {
            printf("Server--> close() function on server-socket unsuccesfull ");
            exit(EXIT_FAILURE);
        }
        //#########################################################
        //WSACleanup() Damit werden diverse Aufräumarbeiten durchgeführt, 
        //und ein interner Referenzzähler, der auf WS2_32.DLL oder WINSOCK.DLL verweist, 
        //wird dekrementiert. Funktion gibt 0 zurück wenn erfolgreich.
        if (WSACleanup() != 0)
        {
            printf("\nServer--> WSACleanup() was not successful...\n");
        }

    #else
        err = close(socketfd);
        if (err < 0) 
        {
            printf("Server--> close() function on server-socket unsuccesfull ");
            exit(EXIT_FAILURE);
        }

    #endif                      
    


    printf("Server--> \n##############CHAT SUCCESSFULLY ENDED##############\n##############GOODBYE##############");
    return 0;
}