#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> // For hostent structure
#include <string.h> // For strlen
#include <unistd.h> // for read, write

void error(char *s)
{
    perror(s);
    exit(1);
}

struct message{
    int a;
    char b;
    char *c;
};
    struct message msg = {1,'A',"India"};
int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        perror("ERROR: Provide both server name and port no");
    }

    int sockfd,portno,n;
    struct sockaddr_in serv_addr; //Address of the server to be connected, internet address (defined in netinet/in.h library){sin_family,sin_port,(struct)sin_addr->s_addr,sin_zero}
    struct hostent *server; //Hostent structure defines the host computer on internet {*h_name,**h_aliases_h_addrtype,h_length,**h_addr_list,h_addr = h_addr_list[0]}

    char buffer[256];

    portno = atoi(argv[2]);
    server = gethostbyname(argv[1]); //Returns pointer to hostent structure

    if(server==NULL)
    {
        error("ERROR: Host was not found");
    }

    sockfd = socket(AF_INET,SOCK_STREAM,0); //(Addressdomain,Typeofsocket,protocol(0 chooses appropriate))
    if(sockfd<0)
    {
        error("ERROR executing socket()");
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // sets binary to zero
    bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr,server->h_length); // bcopy(char*s1,char*s2,int length) copies lenght bytes from s1 to s2
    serv_addr.sin_port = htons(portno);

    if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR executing connect()");
        
    bzero(buffer,256);

    // printf("Enter the message: ");
    // fgets(buffer,255,stdin);


    n = write(sockfd,&msg,255);
    if(n<0)
        error("ERROR in write() in client");
    printf("Sent to server\n");

    n = read(sockfd,buffer,255);
    if(n<0)
        error("ERROR in read() in client");
    printf("Message Recieved from server: %s\n",buffer);
    
    return 0;

}