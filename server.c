#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>


void operatii_server(char* tip_operatie)      //scriu toate operatiile efectuatede server in fisierul log.txt
{
    FILE* f = fopen("log.txt", "a");
    time_t current_time = time(NULL); // obtin timpul curent
    struct tm* time_info = localtime(&current_time); // convertesc timpul curent la formatul struct tm
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%c", time_info); // formatez data si ora curenta in formatul dorit
    fprintf(f, "\n");
    fprintf(f, "%s,\t", buffer); // scriu data si ora curenta in fisier
    fprintf(f, "%s\t", tip_operatie);
    fclose(f); // inchide fisierul
}

void Eroare(char* e)
{
    perror(e);
    exit(1);
}

void send_file(FILE* fp, int sock, char sendBuff[1024])
{
    bzero(sendBuff, 1024);
    int n;
    char data[1024] = { 0 };
    while (fgets(data, 1024, fp) != NULL) // ia linia si o trimite din fisier
    {
        strcat(sendBuff, data);
    }

    //printf("%s",sendBuff);
   // send(sock, sendBuff, sizeof(data), 0);
   //bzero(sendBuff, 1024);
}

void delete_(char file_name[], char sendBuff[1024])
{

    if (remove(file_name) == 0)
    {
        printf("Fisierul %s a fost sters cu succes.\n", file_name);
        strcpy(sendBuff, "Fisierul a fost sters.\n");
    }
    else
    {
        perror("Eroare la stergerea fisierului.\n");
        strcpy(sendBuff, "Eroare la stergerea fisierului.\n");

    }
}

void get_(char file_name[], char sendBuff[1024 * 1024])
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
        strcpy(sendBuff, "get$");
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

void sigterm_handler(int signum)
{
    printf("SIGTERM received, exiting...\n");
    exit(0);
}

void sigint_handler(int signum)
{
    printf("SIGINT received, exiting...\n");
    exit(0);
}

int cauta_cuvantul_in_fisier(char* nume_fisier, char* cuvant)
{
    char linie[100];
    int ok = 0;
    FILE* fp = fopen(nume_fisier, "r");
    while (fgets(linie, 100, fp))
    {
        if (strstr(linie, cuvant)) //daca gasesc cuvantul in fisier
        {
            ok = 1;
            break;
        }
    }

    if (ok == 1)
        return ok;
}

void sterge_linie(char* nume_fisier)
{
    FILE* file, * temp;
    char buffer[1024];

    file = fopen("fisiere_server.txt", "r");
    temp = fopen("temp.txt", "w");

    while (fgets(buffer, 1024, file) != NULL)
        if (strstr(buffer, nume_fisier) == NULL)
            fputs(buffer, temp);

    fclose(file);
    fclose(temp);

    remove("fisiere_server.txt");
    rename("temp.txt", "fisiere_server.txt");
}

void adauga_nume_fisier(char* nume)
{
    FILE* f = fopen("log.txt", "a");
    fprintf(f, "%s,\t", nume);    //adaug in fisierul log.txt numele fisierului afectat
    fclose(f);
}

void* handle_client(void* arg)
{
    int connSock = *((int*)arg);
    char buffer[1024];
    char sendBuff[1024];
    int status;

    while (1)
    {
        //close(readSock);
        char receiveBuff[1024], sendBuff[1024 * 1024];
        int status;
        do {
            status = recv(connSock, &receiveBuff, 1024, 0);
            //printf("%d",status);
            printf("%s\n", receiveBuff);

            if (strcmp(receiveBuff, "list") == 0)
            {
                operatii_server("list");
                // system("ls > comanda.txt");
                DIR* dir;
                struct dirent* ent;
                if ((dir = opendir(".")) != NULL)
                {

                    FILE* f = fopen("comanda.txt", "w");
                    while ((ent = readdir(dir)) != NULL)
                    {
                        struct stat buf;
                        stat(ent->d_name, &buf);
                        if (S_ISREG(buf.st_mode))
                            fprintf(f, "%s\n", ent->d_name); //scriu in fisier doar numele fisierelor

                    }
                    closedir(dir);
                    fclose(f);
                }
                FILE* fp = fopen("comanda.txt", "r");
                send_file(fp, connSock, sendBuff);
                fclose(fp);
            }
            else if (strstr(receiveBuff, "get") != NULL)
            {
                char copie[32];
                strcpy(copie, receiveBuff);
                char* p = strtok(copie, " ");
                p = strtok(NULL, "\n");
                printf("Numele fisierului pe care vreau sa il afisez este: %s.\n", p);
                get_(p, sendBuff);

                operatii_server("get");
                adauga_nume_fisier(p);
                printf("Fisierul a fost adaugat cu succes.\n");
            }


            else if (strstr(receiveBuff, "delete") != NULL)
            {

                char copie[32];
                strcpy(copie, receiveBuff);
                char* p = strtok(copie, " ");
                p = strtok(NULL, "\n");
                printf("Numele fisierului pe care vreau sa il sterg este: %s.\n", p);
                sterge_linie(p);

                operatii_server("delete");
                adauga_nume_fisier(p);
                delete_(p, sendBuff);
                printf("Fisierul a fost sters cu succes.\n");

            }

            else if (strcmp(receiveBuff, "bye") == 0)
            {
                operatii_server("bye");
                signal(SIGTERM, sigterm_handler);
                signal(SIGINT, sigint_handler);

                bzero(sendBuff, 1024);
                strcpy(sendBuff, "Conexiunea s-a inchis cu succes.\n");

                if (close(connSock) == -1)
                {
                    perror("Eroare la inchiderea conexiunii.\n");

                    exit(-1);
                }
                //return 0;

                printf("Conexiunea s-a inchis cu succes.\n");
                break;
            }

            else if (strstr(receiveBuff, "put") != NULL)
            {
                //printf("%s",receiveBuff);

                char copie[1024];
                strcpy(copie, receiveBuff);
                char* p = strtok(copie, "$");
                p = strtok(NULL, "$");

                operatii_server("put");
                adauga_nume_fisier(p);

                // printf("\ncopia este: %s",copie);

                FILE* f1 = fopen(p, "w");
                FILE* f = fopen("fisiere_server.txt", "a");
                fseek(f, 0, SEEK_END);
                fwrite(p, sizeof(char), sizeof(p), f);  //scriu numele fisierului pe care il adaug la server in fisierul fisiere_server.txt
                fclose(f); // inchide fisierul
                p = strtok(NULL, "$");
                fprintf(f1, "%s", p);
                fclose(f1);
                printf("Fisierul a fost primit cu succes.\n");
            }

            else if (strstr(receiveBuff, "update") != NULL)
            {
                printf("update");

                char copie[1024];
                strcpy(copie, receiveBuff);
                char* p = strtok(copie, "$");
                p = strtok(NULL, "$");

                operatii_server("update");
                adauga_nume_fisier(p);


                FILE* f1 = fopen(p, "a");

                char continut[1024];
                recv(connSock, &continut, sizeof(continut), 0);

                fputs(continut, f1);
                fclose(f1);
                printf("Fisierul a fost actualizat cu succes.\n");


            }
            else if (strstr(receiveBuff, "search") != NULL)
            {
                char copie[32];
                strcpy(copie, receiveBuff);
                char* p = strtok(copie, " ");
                p = strtok(NULL, "\n");
                printf("Cuvantul cautat este:%s\n", p);

                operatii_server("search");
                adauga_nume_fisier(p);
                char linie[100];
                char words[100][100];
                int i = 0;
                FILE* g = fopen("fisiere_server.txt", "r");

                while (fgets(linie, 100, g))
                {
                    char* p = strtok(linie, "\n");
                    strcpy(words[i], p);
                    i++;

                }

                // printf("Matricea mea este:\n");
                // for(int j=0;j<i;j++)
                //     printf("%s\n",words[j]);

                for (int j = 0; j < i; j++)
                    if (cauta_cuvantul_in_fisier(words[j], p) == 1)
                    {
                        strcat(sendBuff, words[j]);
                        strcat(sendBuff, "\n");

                    }
                fclose(g);

            }
            else
                strcpy(sendBuff, "Comanda necunoscuta");

            send(connSock, &sendBuff, 1024, 0);
            bzero(sendBuff, 1024);

        } while (status != 0);

        close(connSock);
        exit(0);
    }
    close(connSock);
}


void main()
{

    struct sockaddr_in server;
    struct sockaddr_in client;

    pthread_t thread;


    int readSock, connSock;
    readSock = socket(AF_INET, SOCK_STREAM, 0);    //creaza un socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(2500);


    bind(readSock, (struct sockaddr*)&server, sizeof(server));  //realizeaza legatura dintre un socket si o adresa de conectare
    listen(readSock, 10);    //specifica numarul de cereri de conexiune

//introduc in fisierul fisiere_server.txt lista de fisiere disponibile din server

    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(".")) != NULL)
    {
        FILE* f1 = fopen("fisiere_server.txt", "w");

        while ((ent = readdir(dir)) != NULL)
        {
            struct stat buf;
            stat(ent->d_name, &buf);
            if (S_ISREG(buf.st_mode))
                fprintf(f1, "%s\n", ent->d_name); //scriu in fisier doar numele fisierelor

        }
        closedir(dir);
        fclose(f1);
    }


    while (1)
    {
        int length = sizeof(client);
        connSock = accept(readSock, (struct sockaddr*)&client, &length);  //acepta cererea de conexiune de la un client

        int* connSocknou = malloc(sizeof(int));
        *connSocknou = connSock;
        pthread_create(&thread, NULL, handle_client, (void*)connSocknou);
        pthread_detach(thread);

    }
    close(readSock);
}



