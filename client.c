#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h> 
#include <string.h>


// mai intai clientul furnizeaza informatia => acesta scrie, serverul ii preia comenzile si le va transmite inapoi 

void quit()
{
    pid_t current_pid = getpid();
    kill(current_pid,SIGKILL);
}

void writing_channel(char b[]){
    int fd = open("canal", O_WRONLY);
    write(fd, b, strlen(b)+1);
    close(fd);
}

void reading_channel(char a[])
{
    int fd = open("canal", O_RDONLY);
    read(fd, a, 80);
    close(fd);
}

void reading_users_info(){
    FILE* fd= fopen("canal", "r");
    char buf[256];
     int sec, usec;
    while(fgets(buf, 256,fd) != NULL){
        buf[strlen(buf)-1]='\0';
        fprintf(stdout, "%s\n", buf);
        fgets(buf, 256, fd);
        buf[strlen(buf)-1]='\0';
        fprintf(stdout, "%s\n", buf);
        fgets(buf, 256, fd);
        buf[strlen(buf)-1]='\0';
        fprintf(stdout, "Sec %s\n", buf);
        fgets(buf, 256, fd);
        buf[strlen(buf)-1]='\0';
        fprintf(stdout, "Usec %s\n\n\n", buf);
    }
    fclose(fd);
}

int main()
{
    int fd;
    mkfifo("canal",0666);
    char a[80], b[80];
    int login = 0;
    while(1){
        if(strstr(a,"username")==NULL)
        {
            printf("CLIENT: ");
        }

        // se scrie si transmite informatia catre server
        fgets(b, 80, stdin);
        writing_channel(b);
        printf("\n");

        //se deschide canalul de citire pentru a primi informatiile de la server
        if( strstr(b,"users")!=NULL && login == 1){ // pt comanda users
            reading_users_info();
        }
        else{
        reading_channel(a); // pt celelalte comenzi
        printf("SERVER: %s\n", a);
        if( strstr(a,"User found") != NULL) {
            login = 1;
        }
        if( strstr(a,"Logged out") != NULL) {
            login = 0;
        }
        }

        if(strstr(a,"username") !=NULL){
            printf("Username: ");
        }

        //goodbye ==> quit
        if( strstr(a,"Goodbye") != NULL) {
            quit();
        }
    }
    return 0;
}