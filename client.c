#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <pthread.h>

void put_(char file_name[], char sendBuff[1024 * 1024])    //sendBuff=put$nume_fisier$continut_fisier
{
    FILE* file_ptr;
    // char file_name[] = "example.txt";
    char line[1024];

    file_ptr = fopen(file_name, "r");
    if (file_ptr == NULL) {
        perror("Eroare la deschiderea fisierului.\n");
        // return -1;
        exit(-1);

    }
    else {
        strcpy(sendBuff, "put$");
        strcat(sendBuff, file_name);
        strcat(sendBuff, "$");
    }

    printf("\n");
    while (fgets(line, sizeof(line), file_ptr) != NULL)
    {
        printf("%s", line);
        strcat(sendBuff, line);
    }
    strcat(sendBuff, "$");
    fclose(file_ptr);
}


void Eroare(char* e)
{
    perror(e);
    exit(1);
}

int main()
{
    int sock;
    struct sockaddr_in server;
    sock = socket(AF_INET, SOCK_STREAM, 0); //creaza un socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    server.sin_port = htons(2500);
    int status = connect(sock, (struct sockaddr*)&server, sizeof(server));    //solicita stabilirea legaturii cu un server

    if (status == 0)
        printf("Sunteti conectat! \n");
    else
        Eroare("Eroare de conexiune! Conexiunea nu s-a putut realiza.");

    char sendBuff[1024];
    char receiveBuff[1024 * 1024];
    do {
        printf("Comanda ");
        if (fgets(sendBuff, sizeof(sendBuff), stdin) != NULL) {
            sendBuff[strcspn(sendBuff, "\n")] = 0;
        }

        if (strstr(sendBuff, "put") != NULL) //sendBuff=put$nume_fisier$continut_fisier
        {
            char copie[32];
            strcpy(copie, sendBuff);
            char* p = strtok(copie, " ");
            p = strtok(NULL, "\n");
            printf("Numele fisierului pe care vreau sa il trimit este: %s.\n", p);
            //FILE*f=fopen(p,"r");
            put_(p, sendBuff);
            printf("Fisierul a fost trimis cu succes.\n");

        }


        send(sock, &sendBuff, 1024, 0);
        printf("am trimis %s", sendBuff);
        bzero(&receiveBuff, sizeof(receiveBuff));

        if (strstr(sendBuff, "get") != NULL)
        {
            //printf("%s",receiveBuff);
            char copie[1024];
            strcpy(copie, receiveBuff);
            char* p = strtok(copie, "$");
            p = strtok(NULL, "$");

            FILE* f = fopen(p, "w");

            p = strtok(NULL, "$");
            fprintf(f, "%s", p);
            fclose(f);
            printf("Fisierul a fost primit cu succes.\n");
            strcpy(receiveBuff, p);
        }

        if (strstr(sendBuff, "update") != NULL)
        {

            printf("\nIntroduceti mesajul:\n");
            char continut[1024];
            fgets(continut, sizeof(continut), stdin);
            send(sock, &continut, 1024, 0);

            printf("Am trimis %s\n", continut);
            printf("Fisierul a fost actualizat cu succes.\n");
        }
        recv(sock, &receiveBuff, 1024 * 1024, 0);

        if (strstr(sendBuff, "search") != NULL)
        {
            char cuvant[20];
            char copie[100];
            strcpy(copie, sendBuff);
            char* p = strtok(copie, " ");
            p = strtok(NULL, "\n");
            strcpy(cuvant, p);
            printf("Lista fisierelor in care se afla cuvantul %s este:\n %s\n", cuvant, receiveBuff);
        }

        printf("%s\n", receiveBuff);
    } while (strcmp(sendBuff, "bye") != 0);

    shutdown(sock, SHUT_RDWR);   //inchiderea conexiunii 
    close(sock);
}
