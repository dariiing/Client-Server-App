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

void reading_users_info(char a[]){
    FILE* fd= fopen("canal", "r");
    while(1){
        if(fgets(a, 80,fd) == NULL) {
            break;
        }
        fgets(a, 80, fd);
        fprintf(stdout, "%s\n", a);
        fgets(a, 80, fd);
        fprintf(stdout, "%s\n", a);
        fgets(a, 80, fd);
        fprintf(stdout, "%s\n", a);
        fgets(a, 80, fd);
        fprintf(stdout, "%s\n", a);
    }
    fclose(fd);
}

int main()
{
    int fd;
    mkfifo("canal",0666);
    char a[80], b[80],mesaj[20];
    while(1){

        if(strstr(a,"username")==NULL)
        {
            printf("CLIENT: ");
        }

        // se scrie si transmite informatia catre server
        fgets(b, 80, stdin);
        strcpy(mesaj,b);
        writing_channel(b);
        printf("\n");

        //se deschide canalul de citire pentru a primi informatiile de la server
        if( strstr(b,"users")!=NULL && strstr(mesaj,"login")){
            reading_users_info(a);
        }
        else{
        reading_channel(a);
        printf("SERVER: %s\n", a);
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