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

int main()
{
    int fd;
    mkfifo("canal",0666);
    char a[80], b[80], mesaj[20];
    while(1){
        if(strstr(a,"username")==NULL) //ASTA VA FI PUSA PT FIECARE CAZ, momentan if-ul acesta este temporar
        {
            printf("CLIENT: ");//clientul scrie
        }
        // se scrie si transmite informatia catre server
        fd = open("canal", O_WRONLY); 
        fgets(b, 80, stdin);
        printf("\n");
        write(fd, b, strlen(b)+1); 
        close(fd); 

        //se deschide canalul de citire pentru a primi informatiile de la server
        fd = open("canal",O_RDONLY);
        read(fd, a, sizeof(a));
        printf("SERVER: %s\n", a);

        strcpy(mesaj, "username");
        if(strstr(a,mesaj) !=NULL){
            printf("Username: ");
        }

        //goodbye ==> quit
        strcpy(mesaj,"Goodbye");
        if( strstr(a,mesaj) != NULL) {
            quit();
        }
    }
    return 0;
}