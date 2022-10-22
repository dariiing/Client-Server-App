#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h> 
#include <string.h>
#include <stdlib.h>

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
        sec = (int) strtol(buf,NULL,10);
        fprintf(stdout, "Sec %d\n", sec);
        fgets(buf, 256, fd);
        usec = (int) strtol(buf,NULL,10);
        fprintf(stdout, "Usec %d\n\n\n", usec);

    }
    fclose(fd);
}


int main()
{
    int fd,login=0;
    mkfifo("canal",0666);
    char a[80], b[80],mesaj[20];
    while(1){
        // se scrie si transmite informatia catre server
        printf("CLIENT: ");
        fgets(b, 80, stdin);
        writing_channel(b);
        printf("\n");
        if(strstr(b,"login"))
        {
            reading_channel(a);
            printf("%s",a);
            fgets(b,80, stdin);
            writing_channel(b);
            reading_channel(a);
            if(strstr(a,"User found")){
                    login = 1;
            }
            printf("%s",a);
        }
        //se deschide canalul de citire pentru a primi informatiile de la server
        if( strstr(b,"users")!=NULL && login == 1){
            reading_users_info();
        }
        if(strstr(b,"logout")){
            login = 0;
            reading_channel(a);
            printf("%s\n", a);
        }
        //goodbye ==> quit
        if( strstr(b,"quit") != NULL) {
            reading_channel(a);
            printf("%s\n", a);
            quit();
        }
    }
    return 0;
}