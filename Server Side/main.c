#include <stdlib.h>
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include "uthash.h"
#include <string.h>
#include<time.h>
#include<netdb.h>

struct my_struct
{
    char domain[200];
    int requests;
    int count;
    char ip_address[20][15];
    UT_hash_handle hh;         /* makes this structure hashable */
};
struct my_struct *s, *domains = NULL;

void loadfile(char *loc);
char * HandleTCPClient(char message[2000],int sock,char *loc, char * client);
void addEntry(char * host,char * ipaddress);
void trim(char *c);
int main(int argc , char *argv[])

{
 if(argc != 4){
 printf("Insuffcient arguments.Correct Input format [<server_port>] <DataFile location> [<Retry_Time>]\n");
 return 1;
 };
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[BUFSIZ];
    char *loc =argv[2];

    loadfile(loc); // Loading the Data file

    int time_reaccept=atoi(argv[3]);


    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc == -1)
    {
        printf("Could not create socket");
        return 1;
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( atoi(argv[1]) );
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
    //Listen
    if(listen(socket_desc , 3)<0) // Maxpending set to 3
    {
        perror("Listen failed");
        return 1;
    }
    int flag=0;
    for(;;){
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    char clientName[INET_ADDRSTRLEN];

    if(inet_ntop(AF_INET,&client.sin_addr.s_addr,clientName,sizeof(clientName))!=NULL)
        printf("Handling Client %s/%d\n",clientName,ntohs(client.sin_port));
    else
        puts("Unable get Client Address");
    puts("Connection accepted");

    //Receive a message from client
    time_t sec;
    time_t prev;
    char prev_client[sizeof(clientName)];
    while( (read_size = recv(client_sock , client_message , BUFSIZ-1 , 0)) > 0 )
    {
        printf("Client message: %s\n",client_message);
        trim(client_message);
        //Send the message back to client
        sec=time(NULL);
        if(flag == 0)
        {

            char * output1=HandleTCPClient(client_message,client_sock,loc,clientName);

            send(client_sock , output1 , strlen(output1),0);
            flag=1;
            prev=sec;
            strcpy(prev_client,clientName);
        }
        else
        {

            if(sec-prev < time_reaccept && strcmp(prev_client,clientName)==0)
            {
                char *output=(char*)malloc(100);
                sprintf(output,"A recent enquiry has been made %ld sec ago wait atleast %d sec to try again",sec-prev,time_reaccept);
                send(client_sock,output,strlen(output),0);
                prev=sec;
                strcpy(prev_client,clientName);

            }
            else
            {
                char * output=HandleTCPClient(client_message,client_sock,loc,clientName);
                send(client_sock , output , strlen(output),0);
                prev=sec;
                strcpy(prev_client,clientName);

            }
        }
        memset(client_message,0,1999);
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
}
    return 0;
}
void loadfile(char *loc)
{
    char c[1000];
    FILE *fptr;
    fptr = fopen(loc, "r+");
    if (fptr  == NULL)
    {
        printf("Error! opening file");
        // Program exits if file pointer returns NULL.
        exit(1);
    }
    int flag =0;
    while(fgets(c,1000,fptr) !=NULL)
    {
        char * pch;
        flag=0;
        pch=strtok(c," ");
        char site[300];
        while (pch!=NULL)
        {
            if(flag==0)
            {
                HASH_FIND_STR(domains, pch, s);
                if(s==NULL)
                {
                    s = (struct my_struct*)malloc(sizeof(struct my_struct))  ;
                    strcpy( s->domain,pch);
                    strcpy(site,pch);
                    s->count=0;
                    s->requests=0;
                    HASH_ADD_STR(  domains, domain,  s );
                    flag=1;
                }
            }
            else
            {
                if(flag==1)
                {
                    HASH_FIND_STR(domains, site, s);
                    if(s!=NULL)
                    {
                        s->requests = atoi(pch);
                        flag=2;
                    }
                }
                else
                {
                    HASH_FIND_STR(domains, site, s);
                    if(s!=NULL && s->count <15)
                    {
                        strncpy(s->ip_address[s->count],pch,19);
                        s->count +=1;
                    }
                }
            }
            pch =strtok (NULL, " ");
        }
    }
    fclose(fptr);
}
char * HandleTCPClient(char  message[2000],int sock,char *loc, char * client)
{
    char *cmnd = strtok(message, " ");
    char * output=NULL;
    int i;
    FILE *log=fopen("log.txt","a+");
    char buff[100];
     struct tm *tm;
     time_t now = time (NULL);
     tm = localtime (&now);
    strftime (buff, 100, "%Y-%m-%d %H:%M:%S",tm );

    switch(atoi(cmnd))
     {

    case 1:
    {
        char *test =strtok(NULL," ");
        HASH_FIND_STR(domains,test,s);
        fprintf(log,"%s Client %s:%d inquired about %s.\n",buff,client,sock,test);
        if(s != NULL)
        {
            output= (char *)malloc(sizeof(char)*(s->count+1)*30);
            strcpy(output,"The Ipaddress/es for the inquired domain are:");
            for(i=0; i<s->count; i++)
            {
                strcat(strcat(output," "),s->ip_address[i]);
            }
            s->requests +=1;

        }
        else
        {
            struct hostent *server;
            server =gethostbyname(test);
            if(server !=NULL){
                output = inet_ntoa(*(struct in_addr *)server->h_addr_list) ;
                addEntry(test,output);
                fprintf(log,"%s Client %s:%d  added ( %s , %s) to the database.\n",buff,client,sock,test,output);
                }
            else
               output = "This domain does not exist.";


        }
        break;
    }
    case 2:
    {
        char *test =strtok(NULL," ");

        HASH_FIND_STR(domains,test,s);
        if(s==NULL)
        {
            char *ip = strtok(NULL," ");
            addEntry(test,ip);
             fprintf(log,"%s Client %s:%d  added ( %s , %s) to the database.\n",buff,client,sock,test,ip);
            output="The record has been added to database";
        }
        else
        {
            char *ip =strtok(NULL," ");
            for(i=0; i<s->count; i++)
            {
                if(strcmp(s->ip_address[i],ip)==0)
                {
                    output="This record already exist";
                }
            }
            if(output == NULL)
            {
                output="The record has been added to database";
                strcpy(s->ip_address[s->count],ip);
                s->count +=1;
            }

        }
        break;
    }
    case 3:{
    char *temp=strtok(NULL," ");
    HASH_FIND_STR(domains,temp,s);
            fprintf(log,"%s Client %s:%d deleted this entry: %s.\n",buff,client,sock,temp);
   if(s !=NULL){
    HASH_DEL(domains,s);
    output="This is record has been deleted";
    }
    else{
    output="This record does not exist";
    }
    break;
    }
    case 4:{
     int max=0;
        fprintf(log,"%s Client %s:%d  enquired about maximum requests .\n",buff,client,sock);
     char * record= (char*)malloc(100);
     for(s=domains; s != NULL; s=s->hh.next) {
     if(s->requests>max){
     max=s->requests;
     record=s->domain;
     }
     }
     output = (char *)malloc(200);
     sprintf(output, "The most requested domain is : %s with requests : %d",record,max);
     break;
    }
    case 5: {
     int min=2000;
     fprintf(log,"%s Client %s:%d enquired about minimum requests .\n",buff,client,sock);
     char * record= (char*)malloc(100);
     for(s=domains; s != NULL; s=s->hh.next) {
     if(s->requests<min){
     min=s->requests;
     record=s->domain;
     }
     }
     output = (char *)malloc(200);
     sprintf(output, "The least requested domain is : %s with requests : %d" ,record,min);
     break;
    }
    case 6: {
    char * code = strtok(NULL," ");
    printf("Security code: %s \n",code);
    if(strcmp(code,"test")==0){
    fprintf(log,"%s Client %s:%d successfully shut the server  .\n",buff,client,sock);
    output="Closing down the server";
    send(sock , output , strlen(output),0);
    FILE *fp=fopen(loc,"r+");
    for(s=domains; s != NULL; s=s->hh.next) {
    fprintf(fp,"%s %d ",s->domain,s->requests);
    i=0;
      while(i< s->count){
      char *test=strtok(s->ip_address[i],"\n");
       if(test !=NULL)
       fprintf(fp,"%s ", test);
        i++;
    }
    fprintf(fp,"\n");
    }
    fclose(fp);
    shutdown(sock,2);
    //close(sock);
    exit(0);
    }
    else{
    output="Security code does not match";
    fprintf(log,"%s Client %s:%d tried and failed to shut the server  .\n",buff,client,sock);
    }
    }
    break;
    default:
    output = "Please check the Client Input.";
    }
    fclose(log);
    return output;



}
void addEntry(char * host, char *ip_address)
{
    s = (struct my_struct*)malloc(sizeof(struct my_struct))  ;
    strcpy( s->domain,host);
    s->requests=1;
    s->count=1;
    strcpy(s->ip_address[0],ip_address);
    HASH_ADD_STR(  domains, domain,  s );

}
void trim (char *s) {
  int i = strlen(s)-1;
  if ((i > 0) && (s[i] == '\n'))
    s[i] = '\0';

}
