#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>




#define BUFSIZE 100

struct downloadsend {
    char type;
    char data[BUFSIZE];
};

struct pdu {
    char type;
    char user[10];
    char contentName[10];
    struct sockaddr_in addr;
};

//Initializing all functions

void printoption();
void userInit(struct sockaddr_in reg_addr, int s, char *user, int *registered);
void clientInit(int sd);
int initPDU(struct pdu *senderf, char type, char user[], char contentName[]);
int serverconnection_1(int s, char contname[], struct sockaddr_in addr, char user[]);




int main(int argc, char **argv)
{
    char *host = "localhost";
    int port = 3000;
    struct hostent *phe;
    struct sockaddr_in C_client;
    struct sockaddr_in C_address;
    struct sockaddr_in C_server;
    int client_L;
    int s, type;
    int sd, tempsd, selector;

    char user[10];
    user[0] = '\0';
    int registered = 0;
    int *regPtr = &registered;
    int s_counter;
    int templen;
    fd_set rfds, afds;
    switch (argc){
    case 1:
        break;
    case 2:
        host = argv[1];
    case 3:
        host = argv[1];
        port = atoi(argv[2]);
        break;
    default:
        fprintf(stderr, "usage: UDPtime [host [port]]\n");
        exit(1);
}

memset(&C_server, 0, sizeof(C_server));
    C_server.sin_family = AF_INET;
    C_server.sin_port = htons(port);

    if ( phe = gethostbyname(host)){
        memcpy(&C_server.sin_addr, phe->h_addr, phe->h_length);
    }
    else if ((C_server.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
    fprintf(stderr,"Can't get host entry \n");


    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s<0)
    fprintf(stderr, "Can't create socket \n");


    if (connect(s, (struct sockaddr *)&C_server, sizeof(C_server)) < 0)
    fprintf(stderr, "Can't connect to %s %s \n", host, "Time");


if ((sd= socket(AF_INET, SOCK_STREAM,0)) == -1){
    fprintf(stderr, "Can't create a socket\n");
    exit(1);
}
bzero((char*)&C_address, sizeof(struct sockaddr_in));
C_address.sin_family = AF_INET;
C_address.sin_port = htons(0);
C_address.sin_addr.s_addr = inet_addr("127.0.0.1");


if (bind(sd,(struct sockaddr*)&C_address, sizeof(C_address)) == -1){
    fprintf(stderr, "Can't bind name to socket\n");
    exit(1);
}

templen = sizeof(struct sockaddr_in);
getsockname(sd,(struct sockaddr*)&C_address,&templen);

if(listen(sd,5) < 0){
    fprintf(stderr, "Listening failed\n");
    exit(1);
}

printf("Enter username:\n");

while(1){

    FD_ZERO(&afds);
    FD_SET(0,&afds);
    FD_SET(sd,&afds);
    memcpy(&rfds, &afds, sizeof(rfds));

        if (selector = select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0){
            printf("Cannot select, error.\n");
            exit(1);
        }

    if(FD_ISSET(sd,&rfds)){
        client_L = sizeof(C_client);
        tempsd = accept(sd,(struct sockaddr*)&C_client,&client_L);
        if (tempsd >= 0){
            clientInit(tempsd);
            close(tempsd);
            printf("Enter Command: \n");
        }
    }
    if(FD_ISSET(fileno(stdin), &rfds)){
        userInit(C_address,s, user, regPtr);
        printf("Enter Command:\n");
    }
}

close(sd);
exit(EXIT_SUCCESS);
}


void userInit(struct sockaddr_in C_address, int s, char *user, int *registered)
{
    struct pdu senderf;
    struct downloadsend rpdu;
    struct pdu temps1;
    char cmd;
    char contentName[10];
    int getserver, downloadStatus;
    char contname[10];

    if(user[0] == '\0'){
        scanf("%s", user);
        user[10] = '\0';
        printf("Enter Command:\n");
    }

    scanf(" %c", &cmd);
    switch(cmd){
        case '?':
            printoption();
            break;
        case 'Q':
        	exit(1);
        case 'D':
            printf("which content do you want to download?\n");
            scanf("%s", contname);
            getserver = serverconnection_1(s, contname, C_address, user);
            if (getserver < 0){
                printf("No such content available\n\n");
                break;
            }

            int n_2;
            int receivefiles=1;
            int receiveconnections;
            struct downloadsend C_download;
            FILE *f1;

            temps1.type = 'D';
            strcpy(temps1.contentName, contname);
            temps1.contentName[10] = '\0';

            n_2 = write(getserver, &temps1,sizeof(temps1));

            f1 = fopen(contname,"w");

            if (f1 == NULL)
            {
                receivefiles = 1;
            }
            else
            {
                while(1)
                {
                    receiveconnections = recv(getserver, &C_download, 101, 0);
                    C_download.data[100] = '\0';
                
                if(C_download.type == 'E')
                {
                    printf("ERROR IN DOWNLOAD \n");
                    remove(contname);
                    receivefiles = 1;
                    break;
                }
                else
                {
                    fprintf(f1, "%s", C_download.data);

                    if (C_download.type == 'F') // final batch of data
                    {
                        receivefiles=100;
                        break;
                    }
                }
                }
                fclose(f1);

            }

            if (receivefiles == 1)
                break;
        case 'R':
            if (cmd == 'R'){
                printf("What content do you want to register?\n");
                scanf("%s", contentName);
                initPDU(&senderf, 'R', user, contentName);
            }
            else {
                initPDU(&senderf, 'R', user, contname);
            }

            senderf.addr = C_address;
	    printf("Port number: %d \n", senderf.addr.sin_port);
            write(s, &senderf, sizeof(senderf));

            recv(s, &rpdu, BUFSIZE+1,0);
            if (rpdu.type == 'E'){
                printf("%s\n", rpdu.data);
            }
            else if (rpdu.type == 'A'){
                printf("\n");
                printf("Content Name: %s ", senderf.contentName);
                printf("\nIn client Port: '%d'\n\n", senderf.addr.sin_port);
                (*registered)++;
            }
            break;

        case 'T':
            if (*registered == 0 && cmd == 'T')
                printf("No content to deregister.\n");
            else if (*registered >= 1){
		printf("What content do you want to register?\n");
                scanf("%s", contentName);
                initPDU(&senderf, 'T', user, contentName);
                write(s, &senderf, sizeof(senderf));
                recv(s, &rpdu, BUFSIZE+1, 0);
                if (rpdu.type == 'A') {
                    --(*registered);
                    printf("Deregistered all the content. \n");
                }
            }
            break;
        case 'O':
            senderf.type = 'O';
            write(s,&senderf, sizeof(senderf));
            recv(s, &rpdu, BUFSIZE+1, 0);
            printf("Online Content:\n%s\n", rpdu.data);
            break;
            default:
                break;
    }  
}


void clientInit(int sd)
{
    struct pdu rpdu;
    struct downloadsend senderf;
    char fileName[10];
    char fileNotFound[] = "FILE NOT FOUND\n";
    int n;
    FILE *file;

    if ((n = recv(sd, &rpdu, BUFSIZE+1,0)) == -1){
        fprintf(stderr, "Content Server recv: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    if (rpdu.type == 'D'){
        memcpy(fileName, rpdu.contentName,10);
        char filePath[12];
        snprintf(filePath, sizeof(filePath), "%s%s", "./", fileName);

        file = fopen(filePath, "r");
        if (file == NULL){
            senderf.type = 'E';
            memcpy(senderf.data, fileNotFound, sizeof(fileNotFound));
            write(sd,&senderf, sizeof(senderf));
        }
        else {
            printf("Sending file...\n");
            struct stat filesizenum;
            struct downloadsend packet;
            char filename[BUFSIZE] = {0};
            int nfread, sendingsdata, totaldata;
            totaldata = 0;
            stat(fileName, &filesizenum);


            while ((nfread = fread(filename,1,100,file))>0)
            {
                if (totaldata + 100 >= filesizenum.st_size)
                {
                    packet.type='F';
                }
                else
                {
                    packet.type = 'C';
                }

                memcpy(packet.data, filename,100);

                if ((sendingsdata = send(sd,&packet,sizeof(packet),0)) == -1)
                {
                    printf("Error in code\n");
                    exit(1);
                }
                else
                {
                    totaldata = totaldata + nfread;
                    bzero(filename, 100);
                }
            }
            printf("Successfuly sent file\n\n");
            fclose(file);
        }
    }  
}





int serverconnection_1(int s, char contname[], struct sockaddr_in addr, char user[])
{
    struct pdu senderf, rpdu;
    struct downloadsend dpdu;
    int tcp_sock;

    initPDU(&senderf, 'S', user, contname);
    senderf.addr = addr; 
    write(s, &senderf, sizeof(senderf));
    recv(s, &rpdu, BUFSIZE+1,0);

    if (rpdu.type == 'E'){
        return -1;
    } 
    else if (rpdu.type == 'S')
        initPDU(&senderf, 'D', user, contname);
    printf("Connecting to content server using address. \n");
    printf("Using Port: '%d'\n", rpdu.addr.sin_port);



        if ((tcp_sock = socket(AF_INET, SOCK_STREAM,0)) == -1){
            fprintf(stderr, "Cannot make the socket.\n");
            exit(EXIT_FAILURE);
        }


        if (connect(tcp_sock, (struct sockaddr *)&rpdu.addr, sizeof(rpdu.addr)) == -1){
            fprintf(stderr, "No connection available %s (%d)\n", strerror(errno), errno);
            exit(EXIT_FAILURE);
        }

        return tcp_sock;
}



void printoption()
{
    printf("\n");
    printf("? = Show option \n");
    printf("R = Register content \n");
    printf("T = Unregister content \n");
    printf("L = Show Local content \n");
    printf("O = Show Online content \n");
    printf("D = Download content \n");
    printf("Q = Quit \n\n");

}

//Generic PDU structure mapping
int initPDU(struct pdu *senderf, char type, char user[], char contentName[])
{
    senderf->type = type;
    strcpy(senderf->user,user);
    senderf->user[10] ='\0';
    strcpy(senderf->contentName,contentName);
    senderf->contentName[10] = '\0';
    return 0;

}
