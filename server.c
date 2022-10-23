#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h> 
#include <string.h>
#include <stdlib.h>
#include <utmp.h>
#include <wait.h>

int search( char a[]){
    char user[80];
    int p;
    int file_desc[2];
    if(pipe(file_desc)<0){
        printf("Error at pipe");
        exit(1);
    }
    else{
        printf("Pipe opened\n");
    }
    p = fork();
    if( p < 0) {
        printf("Error occured at fork");
        exit(1);
    }
    else if (p == 0){
       printf("Currently in the child\n");
       FILE *ptr; 
       ptr = fopen("clienti.txt","r"); // deschid fisierul care contine username-urile
       if(ptr == NULL){
        printf("An error occured at opening the clients file");
        exit(1);
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
        if(fd == -1){
            printf("An error occured at opening the channel");
            exit(1);
        }
        else{ 
        printf("File opened\n");
       }
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
    if(fd == -1){
        printf("An error occured at opening the channel");
        exit(1);
        }
       else{ 
        printf("File opened\n");
       }
    write(fd, b, strlen(b)+1);
    close(fd);
}

void reading_channel(char a[]) // opening fifo reading channel
{
    int fd = open("canal", O_RDONLY);
    if(fd == -1){
        printf("An error occured at opening the channel");
        exit(1);
        }
       else{ 
        printf("File opened\n");
       }
    read(fd, a, 80);
    close(fd);
}

struct my_utmp{
    char user[32], host[256];
    struct{
        int32_t sec;
        int32_t usec;
    }tv;
}a[256];

void get_users(){
    int sockp[2], p, k = 0;
    struct utmp *data;
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0){
        printf("Error occured at socketpair");
        exit(1);
    }
    else {
        printf("Socketpair opened\n");
    }
    p = fork();

    if( p < 0 ){
        printf("Error occured at fork");
        exit(1);
    }
    else if( p == 0 ){
        printf("We're in the child\n");
        close(sockp[0]);// parent file descriptor

        data = getutent(); // man:  It returns a pointer to a structure containing the fields of the line. The definition of this structure is shown in utmp(5)
        // username: char ut_user[UT_NAMESIZE]
        // hostname for remote login: char ut_host[UT_HOSTSIZE]
        // time entry was made
        // #define UT_NAMESIZE  32
        // #define UT_HOSTSIZE  256
        while(data != NULL)
        {
            strncpy(a[k].user, data -> ut_user,32);
            a[k].user[32]='\0';
            strncpy(a[k].host, data -> ut_host, 256);
            a[k].host[256]='\0';
            a[k].tv.sec = data -> ut_tv.tv_sec;
            a[k].tv.usec = data -> ut_tv.tv_usec;
            k++;
            data = getutent();
        }

        write(sockp[1], &k, sizeof(int)); // sends size of a[]
        int i;
        for(i = 0; i<k;i++)
        {
            write(sockp[1], a[i].user, sizeof(a[i].user));
            write(sockp[1], a[i].host, sizeof(a[i].host));
            write(sockp[1], &a[i].tv.sec, sizeof(int));
            write(sockp[1], &a[i].tv.usec, sizeof(int));
        }
        printf("Leaving child\n");
        close(sockp[1]);
        exit(0);
    }
    else{
        wait(NULL);
        printf("We're in the parent\n");
        close(sockp[1]);

        read(sockp[0], &k, sizeof(int));

        int i;
        for(i = 0;i < k; i++)
        {
            read(sockp[0], a[i].user, sizeof(a[i].user));
            read(sockp[0], a[i].host, sizeof(a[i].host));
            read(sockp[0], &a[i].tv.sec, sizeof(int));
            read(sockp[0], &a[i].tv.usec, sizeof(int));
        }
        close(sockp[0]);
        
        FILE* fd = fopen("canal", "w");
        if(fd == NULL){
            printf("An error occured at opening the channel");
            exit(1);
        }
        else{ 
            printf("File opened\n");
        }
        for(i = 0; i < k ; i++){
            fprintf(fd, "User %s\n",a[i].user);
            fprintf(fd, "Host %s\n",a[i].host);
            fprintf(fd, "%d\n",a[i].tv.sec);
            fprintf(fd, "%d\n",a[i].tv.usec);
            printf("User %s ",a[i].user);
            printf("Host %s ",a[i].host);
            printf("Sec %d ",a[i].tv.sec);
            printf("Usec %d ",a[i].tv.usec);
            printf("\n");
        }
        fclose(fd);
        printf("Leaving the parent\n");
    }
}
struct my_info{
    char name[80], state[80],ppid[80],uid[80],vmsize[80];
};

void get_info(char pid[]){
    int sockp[2], p,k = 0;
    struct my_info b[100];
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0){
        printf("Error occured at socketpair");
        exit(1);
    }
    else {
        printf("Socketpair opened\n");
    }

    p = fork();
    if( p < 0){
        printf("Error occured at fork");
        exit(1);
    }
    else if(p == 0){
        printf("We're in the child\n");
        close(sockp[0]);

        //create /proc/pid/status 
        char file[80];
        file[0]='\0';

        strcat(file, "/proc/");
        strcat(pid, "/status");
        strcat(file, pid);
        printf("File created: %s\n",file);

        FILE* ptr = fopen(file, "r");
        if(ptr == NULL){
            printf("Error at opening");
            exit(1);
        }
        else{
            printf("File opened\n");
        }
        char info[80];
        while(fgets(info, 80, ptr) != NULL){
            
            info[strlen(info)-1]='\0';
            if(strstr(info, "Name") != NULL){
                strcpy(b[k].name,info);
            }
            else if(strstr(info, "State") != NULL){
                strcpy(b[k].state,info);
            }
            else if(strstr(info, "PPid") != NULL){
                strcpy(b[k].ppid,info);
            }
            else if(strstr(info, "Uid") != NULL){
                strcpy(b[k].uid,info);
            }
            else if(strstr(info, "VmSize") != NULL){
                strcpy(b[k].vmsize,info);
            }
        }
        fclose(ptr);

        write(sockp[1],b[k].name, sizeof(b[k].name));
        write(sockp[1],b[k].state, sizeof(b[k].state));
        write(sockp[1],b[k].ppid, sizeof(b[k].ppid));
        write(sockp[1],b[k].uid,sizeof(b[k].uid));
        write(sockp[1],b[k].vmsize,sizeof(b[k].vmsize));

        close(sockp[1]);
        printf("Leaving child\n");
        exit(0);
    }
    else{//parinte
        wait(NULL);
        close(sockp[1]);
        printf("We're in the parent\n");
        read(sockp[0],b[k].name, sizeof(b[k].name));
        read(sockp[0],b[k].state, sizeof(b[k].state));
        read(sockp[0],b[k].ppid, sizeof(b[k].ppid));
        read(sockp[0],b[k].uid,sizeof(b[k].uid));
        read(sockp[0],b[k].vmsize,sizeof(b[k].vmsize));
        close(sockp[0]);

        FILE* fd = fopen("canal","w");
        if(fd == NULL){
            printf("An error occured at opening the channel");
            exit(1);
        }
        else{ 
            printf("File opened\n");
       }
        fprintf(fd, "%s\n",b[k].name);
        fprintf(fd, "%s\n",b[k].state);
        fprintf(fd, "%s\n",b[k].ppid);
        fprintf(fd, "%s\n",b[k].uid);
        fprintf(fd, "%s\n",b[k].vmsize);
        printf("Name %s\n",b[k].name);
        printf("State %s\n",b[k].state);
        printf("Ppid %s\n",b[k].ppid);
        printf("Uid %s\n",b[k].uid);
        printf("VmSize %s\n",b[k].vmsize);
        
        fclose(fd);
        printf("Leaving the parent\n");
    }
}

int main()
{
    int logged;
    mkfifo("canal",0666);
    char a[80], b[80];
    while(1){
        reading_channel(a);
        printf("CLIENTUL cere: %s\n", a); // comanda dorita de client
        if( strstr(a,"login") != NULL && logged == 0){
            strcpy(b,"Write your username");
            writing_channel(b);

            reading_channel(a);
            printf("Username-ul este: %s\n", a); 

            logged = search(a); // daca 1 = logged, 0 else
            printf("logged %d\n",logged);

        }
        else if(strstr(a,"login") != NULL && logged == 1){
            printf("already logged in\n");
            strcpy(b,"Already logged in");
            writing_channel(b);
        }
        else if(strstr(a,"users") != NULL && logged == 1 ){
            get_users();
        }
        else if(strstr(a, "info") != NULL && logged == 1 ){
            strcpy(b,"Write the process id");
            writing_channel(b);

            reading_channel(a);
            printf("pid-ul este: %s\n", a);
            a[strlen(a)-1]='\0';
            get_info(a);
        }
        //apel logout
        else if(strstr(a,"logout") != NULL && logged == 1){
            logged = 0;
            printf("logged %d\n", logged);
            strcpy(b,"Logged out succesfully");
            writing_channel(b);
        }
        else if((strstr(a,"logout") != NULL && logged == 0 ) || ( strstr(a,"users") != NULL && logged == 0 ) || ( strstr(a,"info") != NULL && logged == 0 )){
            printf("logged %d\n", logged);
            strcpy(b,"Not logged in");
            writing_channel(b);
        }
        else if(strstr(a,"quit")!=NULL){
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

    return 0;
}