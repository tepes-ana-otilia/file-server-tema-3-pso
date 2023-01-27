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

void Eroare(char *e) 
{ 
    perror(e); 
    exit(1);
}

void send_file(FILE *fp, int sock,char sendBuff[1024])
{
    bzero(sendBuff, 1024);
    int n;
    char data[1024] = {0};
    while (fgets(data, 1024, fp) != NULL) // ia linia si o trimite din fisier
    {
        strcat(sendBuff, data);
    }

    //printf("%s",sendBuff);
   // send(sock, sendBuff, sizeof(data), 0);
   //bzero(sendBuff, 1024);
}

void delete_(char file_name[],char sendBuff[1024])
{

    if (remove(file_name) == 0) 
    {
        printf("Fisierul %s a fost sters cu succes.\n", file_name);
        strcpy(sendBuff,"Fisierul a fost sters.\n");
    } 
    else 
    {
        perror("Eroare la stergerea fisierului.\n");
        strcpy(sendBuff,"Eroare la stergerea fisierului.\n");

    }
}

void get_(char file_name[],char sendBuff[1024*1024])
{
    FILE *file_ptr;
   // char file_name[] = "example.txt";
    char line[1024];

    file_ptr = fopen(file_name, "r");
    if (file_ptr == NULL) {
        perror("Eroare la deschiderea fisierului.\n");
       // return -1;
       exit(-1);

    }
    else{
        strcpy(sendBuff,"get$");
        strcat(sendBuff,file_name);
        strcat(sendBuff,"$");
    }

    printf("\n");
    while (fgets(line, sizeof(line), file_ptr) != NULL)
    {
        printf("%s", line);
        strcat(sendBuff,line);
    }
    strcat(sendBuff,"$");
    fclose(file_ptr);
}

void sigterm_handler(int signum) {
    printf("SIGTERM received, exiting...\n");
    exit(0);
}

void sigint_handler(int signum) {
    printf("SIGINT received, exiting...\n");
    exit(0);
}

void main()
{
    
    struct sockaddr_in server;
    struct sockaddr_in client;
    int readSock, connSock;
    readSock=socket(AF_INET, SOCK_STREAM,0);    //creaza un socket
    bzero(&server, sizeof(server));
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    server.sin_port=htons(2500);

    bind(readSock,(struct sockaddr*) &server, sizeof(server));  //realizeaza legatura dintre un socket si o adresa de conectare
    listen(readSock,10);    //specifica numarul de cereri de conexiune

while(1) 
{
	int length=sizeof(client);
	connSock=accept(readSock, (struct sockaddr*) &client,&length);  //acepta cererea de conexiune de la un client

	if(!fork())
    {
	    close(readSock);
	    char receiveBuff[1024],sendBuff[1024*1024];
	    int status;
	    do {
		    status=recv(connSock, &receiveBuff, 1024,0);
		    //printf("%d",status);
		    //printf("%s\n", receiveBuff);
		    
            if (strcmp(receiveBuff,"list")==0) 
            {
               // system("ls > comanda.txt");
                DIR* dir;
                struct dirent* ent;
                if ((dir = opendir(".")) != NULL)
                {

                   FILE *f = fopen("comanda.txt","w");
                    while ((ent = readdir(dir)) != NULL)
                    {
                        struct stat buf;
                        stat(ent->d_name, &buf);
                        if (S_ISREG(buf.st_mode)) 
                            fprintf(f,"%s\n", ent->d_name); //scriu in fisier doar numele fisierelor
        
                    }
                    closedir(dir);
                    fclose(f);
                }
                FILE*fp = fopen("comanda.txt", "r");
                send_file(fp,connSock,sendBuff);
                fclose(fp);
            }
		    else if(strstr(receiveBuff, "get")!=NULL) 
            { 
                char copie[32];
                strcpy(copie,receiveBuff);
                char*p=strtok(copie," ");
                p=strtok(NULL,"\n");
                printf("Numele fisierului pe care vreau sa il afisez este: %s.\n",p);
                get_(p,sendBuff);
            }
            else if(strstr(receiveBuff,"delete")!=NULL) 
            { 
                char copie[32];
                strcpy(copie,receiveBuff);
                char*p=strtok(copie," ");
                p=strtok(NULL,"\n");
                printf("Numele fisierului pe care vreau sa il sterg este: %s.\n",p);
                delete_(p,sendBuff);
            }
            
            else if(strcmp(receiveBuff,"bye")==0) 
            { 
                 signal(SIGTERM, sigterm_handler);
                signal(SIGINT, sigint_handler);
                
                bzero(sendBuff, 1024);
                strcpy(sendBuff,"Conexiunea s-a inchis cu succes.\n");
                
                if (close(connSock) == -1) 
                   { perror("Eroare la inchiderea conexiunii.\n");
               
                    exit(-1);
                    } 
                //return 0;
                
                printf("Conexiunea s-a inchis cu succes.\n");
               break;
            }
        
            else if(strstr(receiveBuff,"put")!=NULL)
                {
                    printf("%s",receiveBuff);
                    char copie[1024];
                    strcpy(copie,receiveBuff);
                    char*p=strtok(copie,"$");
                    p=strtok(NULL,"$");

                   // printf("\ncopia este: %s",copie);
                
                    FILE*f1=fopen("fisier_nou_server.txt","w");
                     p=strtok(NULL,"$");
                    fprintf(f1,"%s",p);
                    fclose(f1);
                }
            else
                strcpy(sendBuff,"Comanda necunoscuta");

	        send(connSock,&sendBuff,1024,0);
            bzero(sendBuff, 1024);

	        } while(status!=0);
	
        close (connSock);
	    exit(0);
	}
	close(connSock);
}
close(readSock);
}

