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



#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
/* Headerfiles für Windows */
#include <winsock2.h> //ermöglicht Sockets + send() + read()
#include <Windows.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <In6addr.h>
#include <io.h>         //input ouput streams

#else
//##Includes für Linux/Unix
#include <sys/types.h> //sollte vor sys/socket include stehen (Linux)
#include <sys/socket.h> //The <sys/socket.h> header defines the sockaddr structure//https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h> //zum nicht-blockieren setzen der SOcket

#endif

#include "message.h" //struct für message

//###############################################################
//HOW TO COMPILE AND RUN
//###############################################################
//compilieren in WINDOWS mit:  gcc -Wall -o client client.c -lws2_32
//bzw. für UNIX: gcc -Wall -o client client.c

//ACHTUNG:Erst Server ausführen dann client
//ausführen WINDOWS: ./client.exe ::1 1234 s82099
//ausführen UNIX: ./client ::1 1234 s82099

int main(int argc, char * argv[])
{
    
    //###############################################################
    // CHECK ARGUMENTS AND PREPARE VARIABLES
    //###############################################################
    //falls nicht genügend Parameter mit Programmaufruf übergeben wurden, wird Programm verlassen
    //für den Client soll die IP-Adresse des Servers => argv[1], die Portnummer => argv[2] als auch die S-Nummer des Studierenden => argv[3] übergeben werden 
    if (argc <4)
    {
        printf("Client--> Please call programm with command-line inputs:\n<filename.c>\n<server_IP_addr>(IF EXECUTED ON ONE PC USE LOOPBACKADDR ::1) \n<portno>(MUST BE IDENTICAL TO SERVER PORT) \nYOUR <s_number>");
        exit(1);
    }
    
    //###############################################################
    //Initialisiere Variable für Portnummer
    int portno;
    
    //die als String übergebene Portnummer (argv[2]) wird mit atoi in int umgewandelt und in portno gespeichert
    portno= atoi(argv[2]);    

    //###############################################################
    //eine Variable der Struktur wie in messag.h definiert wird angelegt
    message msg_client; 
    
    strcpy(msg_client.snummer, argv[3]); //snummer in buffer speichern   
   
    //###############################################################
    //Hilfvariable zur überprüfung, ob Funktionen erfolgreich waren
    int err; 

    
    //###################################################################################
    //PREPARE SOCKET (nur für Windows)
    //###################################################################################
    // int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);

    // Damit unter MS-Windows ein Prozess überhaupt Sockets verwenden kann, muss er vor jedem Aufruf 
    // einer Socket-Funktion initialisiert werden. Durch diese Initialisierung kann ein Prozess die 
    // WS2_32.DLL bzw. WINSOCK.DLL überhaupt erst verwenden. Sie initialisieren den Prozess durch den 
    // Systemaufruf WSAStartup():
    
    //Die Funktion WSAStartup() muss für jede weitere Socket-Funktion aufgerufen werden. 
    //WSAStartup()-function initiiert die Benutzung der Winsock DLL
            //Winsock DLL ist das Modul, welches die Windows Sockets API enthält, 
            //die von den meisten Internet- und Netzwerkanwenungen verwendet wird
            //Wenn erfogreich, gibt WSAStartup() 0 zurück.
        //https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup

    // Der erste Parameter ist vom Datentyp WORD (unsigned short) mit 2 Bytes Länge. 
    // Mit diesem Parameter geben Sie die Versionsnummer von Winsock an, die Sie verwenden wollen.
    // Dabei legen Sie im Low-Order-Byte die Major-Nummer und im High-Order-Byte die Minor-Nummer (Revisionsnummer) fest. 
    
    //wVersionRequested : Diese Variable soll die Versionsnummer von Winsock enthalten, die ich verwenden will.
        //https://openbook.rheinwerk-verlag.de/c_von_a_bis_z/025_c_netzwerkprogrammierung_002.htm#mj73f2f72802f98899435ecc1c087b5cee

    
    //Mit dem zweiten Parameter von WSAStartup() geben Sie einen Zeiger auf die Struktur (LP)WSADATA (LP = Long Pointer) an. 
    //In dieser Struktur finden Sie Informationen zur Winsock-Version.
    //LPWSADATA ist ein Pointer auf diese Informationen
        //https://docs.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-wsadata
        //https://openbook.rheinwerk-verlag.de/c_von_a_bis_z/025_c_netzwerkprogrammierung_002.htm#mj73f2f72802f98899435ecc1c087b5cee

    #ifdef _WIN32
        WORD wVersionRequested; //Datentyp WORD (unsigned short) mit 2 Bytes Länge.
        WSADATA wsaData; //Datentyp der Informationen über die Winodws Sockets implementierung enthält        
        
        // Damit Sie sich jetzt nicht mit Bitverschiebungen und den Byte-Orders auseinandersetzen müssen, 
        // verwenden Sie am besten gleich das Win32-Makro MAKEWORD():

        wVersionRequested = MAKEWORD(2, 2); //bestimmt die Winsockversion, die gestartet werden soll. Hier Version 2.2 da Makeword die beiden Ziffer so verbindet

        //#########################################################
        //Nutzung von Winsock vorbereiten
        err = WSAStartup(wVersionRequested, &wsaData);

        //#########################################################
        //Test ob WSAStartup() erfolgreich war
        if (err != 0) 
        {
            printf("Client--> WSAStartup failed with error: %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        else printf("\nClient-->WSAStartup() function successfull\n");
    #endif
    
    //###################################################################################
    //INITIALIZE AND START SOCKET
    //###################################################################################
    // SOCKET  socket(int af, int type, int protocol); ==> Windows
    // int  socket(int af, int type, int protocol); ==> Unix

    //Datentyp SOCKET ist letztendlich nichts anderes als eine Typdefinition von int, 
    //und somit könnten Sie in der Praxis hierfür auch int verwenden. 
    //Als Rückgabewert erhalten Sie den Socket-Deskriptor  (sozusagen der Name mit dem die Socket addressiert werden kann).

    //1.Mit dem ersten Parameter domain bzw. af geben Sie die Adressfamilie 
    //(d. h. die Protokollfamilie) an, die Sie verwenden wollen.
    // AF_INET = Internet IP-Protokoll Version 4 (IPv4)1
    // AF_INET6 = Internet IP-Protokoll Version 6 (IPv6)
    //int af = AF_INET6;
    
    //2.Mit dem zweiten Parameter der Funktion socket() geben Sie den Socket-Typ an. 
    //Damit legen Sie die Übertragungsart der Daten fest. 
    //symbolischen Konstanten SOCK_STREAM für TCP und SOCK_DGRAM für UDP interessant.
    //int type = SOCK_DGRAM;

    //3.Mit dem dritten Parameter können Sie ein Protokoll angeben, das Sie zur Übertragung verwenden wollen. 
    //Wenn Sie hierfür 0 eintragen, was meistens der Fall ist, wird das Standardprotokoll verwendet, 
    //das dem gewählten Socket-Typ (zweiter Parameter) entspricht. 
    //Im Fall von SOCK_STREAM wird TCP und bei SOCK_DGRAM wird UDP verwendet.
    
    #ifdef _WIN32
        SOCKET socketfd; //nur Windows kennt Datentyp Socket
    #else
        int socketfd;
    #endif
    
    socketfd = socket( PF_INET6, SOCK_STREAM , 0); //socketfd = socket file descdriptor (enthält int-wert mit dem socket addressiert/aufgerufen werden kann)

    //#########################################################
    //Prüfung, ob Socket erfolgreich erstellt wurde
    // Unter MS-Windows wird bei einem Fehler die Konstante SOCKET_ERROR 
    // (ebenfalls mit –1 definiert) zurückgegeben. Hierbei können Sie den Fehlercode 
    // mit der Funktion WSAGetLastError() ermitteln.
    
    #ifdef _WIN32
        if (socketfd == INVALID_SOCKET)
        {
            printf("\nClient--> Socket creation unsuccesfull ");
            exit(EXIT_FAILURE);
        }
    #else
        if (socketfd < 0)
        {
            printf("\nClient--> Socket creation unsuccesfull");
            exit(EXIT_FAILURE);
        }
    #endif 
        else printf("\nClient-->Socket creation successfull\n");

    //#########################################################
    //#########################################################
    //#########################################################
    //#########################################################
    //#########################################################
    //SET SOCKET TO NONBLOCKING
    //#########################################################
    
    
    //FAILS CONNECTION WHEN SET TO NON-BLOCKING



    // u_long optval = 1; //ungleich 0 bedeutet nicht blockieren
    // #ifdef _WIN32    
    //     err = ioctlsocket(socketfd, FIONBIO, &optval);
    // #else
    //     err = ioctl(socketfd, FIONBIO, &optval);
    // #endif

    // if (err != 0)
    // {
    //     printf("\nClient-->failed to set non-blocking");
    //     exit(EXIT_FAILURE);
    // }
    // else printf("\nClient-->Socket non-blocking successfull\n");



    //####################################################################################
    //SET SOCKET ADDRESSINFORMATION 
    //####################################################################################
    // int connect ( SOCKET socketfd, const struct sockaddr FAR* addr, int addrlen); ==> WINDOWS
    // int connect ( int socketfd, const struct sockaddr FAR* addr, int addrlen); ==> UNIX

    // Nachdem mit den Sockets die Kommunikationsendpunkte erzeugt wurden, 
    // kann der Client nun versuchen, eine Verbindung zum Server-Socket herzustellen. 
    // Dies wird mit der Funktion connect() versucht

    //1. Als erster Parameter wird der Socket-Deskriptor erwartet, 
    //über den Sie die Verbindung herstellen wollen. Dies ist der Rückgabewert, den Sie von der Funktion socket() erhalten haben.
    //socketfd (wie der Name der Socket als int)

    //2. Die Adressinformationen über den gewünschten Verbindungspartner tragen Sie im zweiten Parameter der Funktion connect() ein.
    //Um eine Verbindung zu einem anderen Rechner aufzubauen, werden logischerweise auch Informationen über die Adresse benötigt, 
    //mit der sich der Client verbinden will. Um sich mit dem Server zu verbinden, benötigen Sie Informationen über die Adressfamilie (Protokollfamilie), 
    //die Portnummer und logischerweise die IP-Adresse. Eingetragen werden diese Informationen mit dem zweiten Parameter der Struktur sockaddr
    //sockaddr_in ist allerding leichter mit Addressinformationen zu füllen. IP-Adresse und Portnummer können hier getrennt eingetragen werden.
    //in der connect()-Funktion wird sockaddr struktur gefordert, diese kann über casten erreicht werden, siehe: (struct sockaddr *)&client_addr

    struct sockaddr_in6 client_addr; //anlegen einer Varibale der Struktru serv_addr_in
    memset(&client_addr, 0, sizeof(struct sockaddr_in6));//Variable der Serveraddresse leeren
   
    /// IPv6-Adresse: In der Strukturvariablen sin_family geben Sie die Adressfamilie (Protokollfamilie) an, mit der Sie kommunizieren wollen. 
    //Gewöhnlich gibt man hierfür dieselbe Familie an, wie schon beim ersten Parameter der Funktion socket().
    client_addr.sin6_family= PF_INET6;
    
    // als IP-Adresse wird die mit Programmaufruf übergebene IP Adresse verwendet    
    struct in6_addr addr;
    inet_pton(PF_INET6, argv[1], &addr); // die per Prgrammaufruf übergebene IPv6 Adresse in richtiges Format bringen
    client_addr.sin6_addr = addr;

    //Portnummer einfügen: In sin6_port geben Sie die Portnummer an, über die Sie mit dem Server in Kontakt treten wollen. 
    //Wichtig ist hierbei, dass Sie den Wert in der Network Byte Order angeben. Es genügt also nicht, wenn Sie sich 
    //beispielsweise mit einem Webserver verbinden wollen, als Portnummer einfach 80 hinzuschreiben. Sie müssen hierbei auch 
    //auf die verschiedenen Architekturen Rücksicht nehmen, die es in heterogenen Netzwerken gibt. Denn auf den verschiedenen 
    //Architekturen gibt es unterschiedliche Anordnungen der Bytes zum Speichern von Zahlen. So wird bei der Anordnung gewöhnlich 
    //zwischen Big Endian und Little Endian unterschieden. Man spricht dabei gern vom »Zahlendreher«. Beim Big Endian-Format wird 
    //das höchstwertige Byte an der niedrigsten Adresse gespeichert, das zweithöchste an der nächsten Adresse und so weiter. 
    //Bei der Anordnung von Little Endian ist dies genau umgekehrt. Dabei wird das niedrigstwertige Byte an der niedrigsten Stelle 
    //gespeichert, das zweitniedrigste an der nächsten Stelle usw.

    //Um jetzt aus einer lokal verwendeten Byte-Reihenfolge (Host Byte Order) eine Network-Byte-Order-Reihenfolge oder umgekehrt zu 
    //konvertieren, stehen Ihnen zB die folgende Funktionen zur Verfügung: unsigned short int htons(unsigned short int hostshort);
    client_addr.sin6_port=htons(portno); 

    client_addr.sin6_flowinfo = 0;
    
    //3. Mit dem letzten Parameter (addrlen) von connect() geben Sie die Länge in Bytes von sockaddr mit dem sizeof-Operator an.
       
    
    //#########################################################
    //#########################################################
    //#########################################################
    //#########################################################
    //CONNECT SOCKET TO SERVER ADDRESS
    //#########################################################
    // Client-Socket mit Server-Socket verbinden (Server-Socket Adresse wurde mit bind()-Funktion festgelegt)
    // und testet, ob Verbindung erfolgreich war
    
    err = connect(socketfd, (struct sockaddr *) &client_addr, sizeof(client_addr) ); 
    
    //#########################################################
    // Prüfung, ob Socket erfolgreich erstellt wurde

    if (err < 0) 
    {
        printf("Client--> Socket connection unsuccesfull ");
        exit(EXIT_FAILURE);
    }
    else printf("\nClient-->Socket connection successfull\n");



    //#########################################################
    //INITIALIZE FILE DESCRIPTOR SETS 
    //#########################################################

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
              
        FD_SET(0, &master); // Stdin zu Master hinzufügen ACHTUNG: FUNKTIONIERT NUR UNTER UNIX ODER UNIX-ähnlichen Systemen
                
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
            //HANDLE INCOMING AND OUTGOING MESSAGES
            //#########################################################
            if (FD_ISSET(i, &read_fds)) //falls Socket i lesebereit
            {
                
                
                //#########################################################
                //INCOMING
                if (i == socketfd) //falls Client Socket lesebereit ist, bedeutet dies, Server hat Nachricht gesendet
                {
                    memset(&msg_client.text, 0, 1024); 
                    // Daten von Client in Message Stuct einlesen
                    err = recv(i, &msg_client, sizeof(msg_client), 0);

                    //#########################################################
                    //Prüfung, ob recv() der Socket erfolgreich war        
                    if (err < 0) 
                    {
                        printf("Client--> recv() function unsuccesfull ");
                        #ifdef _WIN32
                            closesocket(i); // bye!
                        #else
                            close(i); // bye!
                        #endif
                        FD_CLR(i, &master); // remove from master set
                    } 
                    else printf("Rcvd from: %s--> %s", msg_client.snummer, msg_client.text);
                }
                //#########################################################
                //OUTGOING
                else if (i == 0) //falls es sich um STDIN handelt, sende Daten an Client
                {   
                    
                    memset(&msg_client.text, 0, 1024); //text feld leeren

                    fgets(msg_client.text, 1024, stdin);
                    
                    err = send(socketfd, &msg_client, sizeof(msg_client), 0);//senden an i socket    

                    //#########################################################
                    //Prüfung, ob send() der Socket erfolgreich war        
                    if (err < 0) 
                    {
                        printf("Client--> send() function unsuccesfull ");
                        #ifdef _WIN32
                            closesocket(i); // bye!
                        #else
                            close(i); // bye!
                        #endif
                        FD_CLR(i, &master); //Von Master Set entfernen da Verbindung nicht mehr aktuell
                    }  
                }
            }    
        }
        //#########################################################
        //END CHAT
        //#########################################################
        //Ende des Chats erkennen
        //Wird eine Nachricht gesendet, die mit "bye" gebinnt, wird die while-Schleife bzw. Verbindung beendet
        //strncmp = String compare n Zeichen = vergleicht erste 3 Zeichen des Buffers mit "bye" 
        //Wenn sie übereinstimmen wird 0 zurückgegeben. Daher ! = not
        if(!strncmp("bye", msg_client.text,3))
        {
            break;
        }
    }

    //#########################################################
    //SOCKETS SCHLIESSEN UND CLEANUP
    //#########################################################

    //Sobald Sie mit der Datenübertragung fertig sind, sollten Sie den Socket-Deskriptor wieder freigeben bzw. schließen. 
    //TCP-Verbindung wird beendet
   
    #ifdef _WIN32
        err = closesocket(socketfd);
        if (err < 0) 
        {
            printf("Client--> close() function on server-socket unsuccesfull ");
            exit(EXIT_FAILURE);
        }
        //#########################################################
        //WSACleanup() Damit werden diverse Aufräumarbeiten durchgeführt, 
        //und ein interner Referenzzähler, der auf WS2_32.DLL oder WINSOCK.DLL verweist, 
        //wird dekrementiert. Funktion gibt 0 zurück wenn erfolgreich.
        if (WSACleanup() != 0)
        {
            printf("\nClient--> WSACleanup() was not successful...\n");
        }

    #else
        err = close(socketfd);
        if (err < 0) 
        {
            printf("Client--> close() function on server-socket unsuccesfull ");
            exit(EXIT_FAILURE);
        }

    #endif    

    
    printf("Client--> \n##############CHAT SUCCESSFULLY ENDED##############\n##############GOODBYE##############");
    return 0;
}