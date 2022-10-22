#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h> 
#include <string.h>
#include <stdlib.h>

int search( char a[]){
    char user[80];
    int p;
    int file_desc[2];
    if(pipe(file_desc)<0){
        printf("Error at pipe");
    }
    else{
        printf("Pipe opened\n");
    }
    p = fork();
    if( p < 0) {
        printf("Error occured at fork");
    }
    else if (p == 0){
       printf("Currently in the child\n");
       FILE *ptr; 
       ptr = fopen("clienti.txt","r"); // deschid fisierul care contine username-urile
       if(ptr == NULL){
        printf("An error occured at opening the clients file");
        }
       else{ 
        printf("File opened\n");
       }
       int ok = 0;
       while(fgets(user, sizeof(user),ptr)!=NULL && ok == 0){
            if(strcmp(a,user) == 0){
                ok = 1;
                printf("user found: ");
                printf("%d\n",ok);
            }
        }
        if(ok == 0){
            printf("user not found: ");
            printf("%d\n",ok);
        }
        fclose(ptr);

        //trimit informatia catre parinte
        close(file_desc[0]);
        write(file_desc[1],&ok, sizeof(int));
        close(file_desc[1]);
        exit(0);
    }
    else {
        wait(NULL);
        printf("Currently in the parent\n");
        int ok;
        close(file_desc[1]);
        read(file_desc[0],&ok, sizeof(int));
        close(file_desc[0]);

        //trimit mesaj catre client
        int fd = open("canal", O_WRONLY);
        if(ok == 1){
        strcpy(user,"User found. Welcome back!");
        write(fd, user, strlen(user)+1);
        }
        else{
            strcpy(user,"User not found. Please try again");
            write(fd, user, strlen(user)+1);
        }
        close(fd);
        return ok;
    }
}

int login() {
    int fd, p;
    char b[80];
    fd = open("canal", O_WRONLY);
    strcpy(b,"Write your username");
    write(fd, b, strlen(b)+1);
    close(fd);
    return 1;
}

void quit()
{
    pid_t current_pid = getpid();
    kill(current_pid,SIGKILL);
}

int main()
{
    int fd, logged,length;
    mkfifo("canal",0666);
    char a[80], b[80], mesaj[20];
    while(1){
        fd = open("canal", O_RDONLY);
        read(fd, a, 80);
        printf("CLIENTUL cere: %s\n", a); // comanda dorita de client
        close(fd);
        strcpy(mesaj, "login");
        if( strstr(a,mesaj) != NULL && logged == 0){

            fd = open("canal", O_WRONLY);
            strcpy(b,"Write your username");
            write(fd, b, strlen(b)+1);
            close(fd);

            fd = open("canal", O_RDONLY);
            read(fd, a, 80);
            printf("Username-ul este: %s\n", a); 
            close(fd);

            logged = search(a); // daca 1 = logged, 0 else
            printf("logged %d\n",logged);

        }
        else if(strstr(a,mesaj) != NULL && logged == 1){
            printf("already logged in\n");
            fd = open("canal", O_WRONLY);
            strcpy(b,"Already logged in");
            write(fd, b, strlen(b)+1);
            close(fd);
        }
        else if(0 == 0){
        //apel logout
        strcpy(mesaj, "logout");
        if(strstr(a,mesaj) != NULL && logged == 1){
            logged = 0;
            printf("logged %d\n", logged);
            fd = open("canal", O_WRONLY);
            strcpy(b,"Logged out succesfully");
            write(fd, b, strlen(b)+1);
            close(fd);
        }
        if(strstr(a,mesaj) != NULL && logged == 0){
            logged = 0;
            printf("logged %d\n", logged);
            fd = open("canal", O_WRONLY);
            strcpy(b,"Not logged in");
            write(fd, b, strlen(b)+1);
            close(fd);
        }
        else if(0 == 0){
        // apel quit
        strcpy(mesaj,"quit");
        if(strstr(a,mesaj)!=NULL){
            printf("Se inchid programele\n");
            fd = open("canal", O_WRONLY);
            strcpy(b,"Goodbye");
            write(fd, b, strlen(b)+1);
            close(fd);
            quit();
        }
        else{ // nu cunoaste comanda
            printf("unknown command\n");
            fd = open("canal", O_WRONLY); 
            strcpy(b,"Unknown command, try again");
            write(fd, b, strlen(b)+1); 
            close(fd);
        }
        }
    }
    }
    return 0;
}