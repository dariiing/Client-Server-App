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

void quit()
{
    pid_t current_pid = getpid();
    kill(current_pid,SIGKILL);
}

void writing_channel(char b[]){ // opening fifo writing channel
    int fd = open("canal", O_WRONLY);
    write(fd, b, strlen(b)+1);
    close(fd);
}

void reading_channel(char a[]) // opening fifo reading channel
{
    int fd = open("canal", O_RDONLY);
    read(fd, a, 80);
    close(fd);
}

int main()
{
    int logged;
    mkfifo("canal",0666);
    char a[80], b[80], mesaj[20];
    while(1){
        reading_channel(a);
        printf("CLIENTUL cere: %s\n", a); // comanda dorita de client
        strcpy(mesaj, "login");
        if( strstr(a,mesaj) != NULL && logged == 0){
            strcpy(b,"Write your username");
            writing_channel(b);

            reading_channel(a);
            printf("Username-ul este: %s\n", a); 

            logged = search(a); // daca 1 = logged, 0 else
            printf("logged %d\n",logged);

        }
        else if(strstr(a,mesaj) != NULL && logged == 1){
            printf("already logged in\n");
            strcpy(b,"Already logged in");
            writing_channel(b);
        }
        else if(0 == 0){
        //apel logout
        strcpy(mesaj, "logout");
        if(strstr(a,mesaj) != NULL && logged == 1){
            logged = 0;
            printf("logged %d\n", logged);
            strcpy(b,"Logged out succesfully");
            writing_channel(b);
        }
        else if(strstr(a,mesaj) != NULL && logged == 0){
            logged = 0;
            printf("logged %d\n", logged);
            strcpy(b,"Not logged in");
            writing_channel(b);
        }
        else if(0 == 0){
        // apel quit
        strcpy(mesaj,"quit");
        if(strstr(a,mesaj)!=NULL){
            printf("Se inchid programele\n");
            strcpy(b,"Goodbye");
            writing_channel(b);
            quit();
        }
        else{ // nu cunoaste comanda
            printf("unknown command\n");
            strcpy(b,"Unknown command, try again");
            writing_channel(b);
        }
        }
    }
    }
    return 0;
}