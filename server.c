#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>                                                                   
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/stat.h>
#include <stdbool.h>

#define SERVER_TCP_PORT 3000
#define BUFSIZE 100

struct pdu {
    char type;
    char data[100];
};

struct content {
    char peer_name[10];
    char content_name[10];
    int port;
    struct sockaddr_in addr;
};

struct registerPDU {
    char type;
    char peer_name[10];
    char content_name[10];
    struct  sockaddr_in addr;
};

void obtain_list(struct content contents[5], int length, char *msg);

int main(int argc, char **argv) {
    struct sockaddr_in fsin;
    int sock;
    int alen;
    struct sockaddr_in sin;
    int s, type, n, i;
    int port = 3000;
    bool exists = false;

    int value;

    struct pdu spdu;
    struct registerPDU request;
    struct registerPDU response;

    struct content *contents = malloc(5 * sizeof(struct content));
    int peer_name_index = 0;
    int content_name_index = 0;
    int pc_index = 0;

    int compare;
    int compare2;

    char message[55];

    switch (argc) {
        case 1:
            break;
        case 2:
            port = atoi(argv[1]);
            break;
        default:
            fprintf(stderr, "Usage: %s [port]\n", argv[0]);
            exit(1);
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    //Allocate a socket
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
        fprintf(stderr, "Can't create socket \n");

    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) 
        fprintf(stderr, "Can't bind to %d port\n", port);

    listen (s, 5);
    alen = sizeof(fsin);

    while(1) {
        if ((n = recvfrom(s, &request, sizeof(request), 0, (struct sockaddr *)&fsin, &alen)) < 0)
            fprintf(stderr, "Request error.\n");
        
        switch(request.type) {
            case 'R':
                spdu.type = 'A';

                for (i = 0; i < 5; i++) {
                    if (strcmp(contents[i].peer_name, request.peer_name) == 0) {
                        exists = true;
                    } else {
                        //printf("Peer name %s\n", contents[i].peer_name);
                    }
                }

                if(peer_name_index > 0 && exists == true) {
                    spdu.type = 'E';
                    exists = false;
                    memcpy(spdu.data, "Peer name already exists.\n", sizeof("Peer name alrady exists.\n"));
                }

                if (sendto(s, &spdu, 101, 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0) {
                    printf("Error sending data.\n");
                    exit(1);
                }

                if (spdu.type == 'E')
                    break;
                printf("Peer name: %s \n", request.peer_name);
                printf("Content name: %s \n", request.content_name);
                printf("Port number: %d \n", request.addr.sin_port);

                strcpy(contents[content_name_index].content_name, request.content_name);
                contents[content_name_index].content_name[10] = '\0';
                printf("Content name2: %s \n", contents[content_name_index].content_name);

                strcpy(contents[content_name_index].peer_name, request.peer_name);
                contents[content_name_index].peer_name[10] = '\0';

                contents[content_name_index].addr = request.addr;
                content_name_index++;
                peer_name_index++;
                break;
            case 'O':
                printf("Request recieved. Current available content: \n");
                printf("Content name2: %s \n", contents[0].content_name);

                if (content_name_index == 0) {
                    spdu.type = 'E';
                    memcpy(spdu.data, "No Registered Content Avaliable.\n", sizeof("No Registered Content Available \n"));
                    
                    if (sendto(s, &spdu, 101, 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0) {
                        printf("Error sending data. \n");
                        exit(1);
                    }
                } else {
                    obtain_list(contents, content_name_index, message);
                    spdu.type = 'O';
                    memcpy(spdu.data, message, sizeof(message));
                    if (sendto(s, &spdu, 101, 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0) {
                        printf("Error sending data. \n");
                        exit(1);
                    }
                }
                break;
            
            case 'T':
                for (i = 0; i < 5; i++) {
                    if (strcmp(contents[i].peer_name, request.peer_name) == 0) {
                        compare = i;
                    } 
			//else {
                        //compare = -1;
                	//}
                }

                pc_index = compare;
                printf("De-Registering Name: '%s'\n", contents[pc_index].peer_name);
                printf("De-Registering Content: '%s'\n", contents[pc_index].content_name);
                printf("De-Registering Port: '%d'\n", contents[pc_index].addr.sin_port);

                for (i = pc_index; i < content_name_index; i++) {
                    if (i > 0 && i + 1 >= content_name_index) {
                        strcpy(contents[i].content_name, "");
                        strcpy(contents[i].peer_name, "");
                        break;
                    }
                    
                    contents[i] = contents[i + 1];
                }

                printf("De-registration complete.\n");

                content_name_index--;
                peer_name_index--;
                spdu.type = 'A';

                if (sendto(s, &spdu, 101, 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0) {
                    printf("Error sending data. \n");
                    exit(1);
                }
                break;
            
            case 'S':
                for (i = pc_index; i >= 0; i--) {
                    if (strcmp(contents[i].content_name, request.content_name)) {
                        compare2 = i;
                    } else {
                        compare2 = -1;
                    }
                }

                pc_index = compare2;

                if (pc_index == -1) {
                    response.type = 'E';
                } else {
                    printf("%s found\n", response.content_name);
                    response.type = 'S';
                    response.addr = contents[pc_index].addr;
			contents[pc_index].addr = request.addr;
                }

                if (sendto(s, &response, 101, 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0) {
                    printf("Error sending data. \n");
                    exit(1);
                }
                break;
        }
        bzero(spdu.data, 100);
    }
}

void obtain_list (struct content contents[5], int length, char *msg) {
    char buffer[55];
    int i, j, k;
    char unique_names[length][10];
    int count = 1;
    int duplicate = 0;

    strcpy(unique_names[0], contents[0].content_name);

    for (i = 1; i < length; i++) {
        for (j = 0; j < count; j++) {
            if (strcmp(contents[i].content_name, unique_names[j]) == 0)
                duplicate = 1;
            
            if (duplicate == 0) {
                strcpy(unique_names[count], contents[i].content_name);
                count++;
            }

            duplicate = 0;
        }

        for (k = 0; k < count; k++) {
            strcat(buffer, unique_names[k]);
            strcat(buffer, "\n");
        }

        printf("%s\n", buffer);
        strcpy(msg, buffer);
    }
}
