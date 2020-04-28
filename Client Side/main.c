
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <unistd.h>
#include<stdlib.h>
int check_ip(char *ip);
int main(int argc , char *argv[])
{
    int sock;
    if ((argc < 4) || (argc > 6))    /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>] [Command] [param1] [optional param2]\n", argv[0]);
        exit(1);
    }
    char *host_ip = malloc(sizeof(argv[1]));
    strcpy(host_ip,argv[1]);
    if(check_ip(host_ip)!=1)
    {
        perror("Check the IP address of the Server\n");
        return 1;
    }
    if(strcmp(argv[3],"2")==0)
    {
        if((argc)!=6)
        {
            printf("Insufficient Arguments.\n");
            return 1;
        }
    }
    if(strcmp(argv[3],"1")==0 ||strcmp(argv[3],"3")==0 ||strcmp(argv[3],"6")==0)
    {
        if(argc !=5)
        {
            printf("Insufficient Arguments.\n");
            return 1;
        }
    }
    struct sockaddr_in server;
    char  message[1000]= {0};
    char server_reply[BUFSIZ];
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(argv[2]));

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    //keep communicating with server
    int flag=0;
    memset(server_reply,0,1999);

    if(argc==5)
    {
        strcat(strcat(strcat(message,argv[3])," "),argv[4]);
        flag=1;
    }
    if(argc==4)
    {
        strcat(message,argv[3]);
        flag =1;
    }



if(argc==6 && strcmp(argv[3],"2")==0)
{
    strcpy(host_ip,argv[5]);
    if(check_ip(host_ip)!=1)
    {
        printf("Check the IP address of the Domain.\n");
        return 1;
    }
    strcat(strcat(strcat(strcat(strcat(message,argv[3])," "),argv[4])," "),argv[5]);
    flag =1;
}
char sen_message[]="5";
if(flag ==1)
{
    if( send(sock , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    //Receive a reply from the server
    if( recv(sock , server_reply ,BUFSIZ-1, 0) < 0)
    {
        puts("recv failed");
        return 1;
    }


    puts("Server reply :");
    printf("%s\n",server_reply);
}
if(flag ==0)
{
    perror("Please check the inputs something wrong");
    return 1;
}
memset(message,0,1000);

close(sock);
return 0;
}
int check_ip(char *ip)
{
    int count=0;
    char *token;
    token = strtok(ip,".");
    while( token != NULL )
    {
        if((atoi(token)>0&&atoi(token)<256)||(*token=='0'))
            count++;
        else
        {
            return 0;
        }
        token = strtok(NULL, ".");
    }
    if(count==4)
        return 1;
}

