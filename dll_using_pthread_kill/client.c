#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> // For hostent structure
#include <string.h> // For strlen
#include <unistd.h> // for read, write
#include <json-c/json.h>

void error(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char* argv[])
{
    if(argc < 4)
    {
        perror("ERROR: Provide both server name and port no, function name and input");
    }

    int sockfd,portno,n;
    
    struct sockaddr_in serv_addr; //Address of the server to be connected, internet address (defined in netinet/in.h library){sin_family,sin_port,(struct)sin_addr->s_addr,sin_zero}
    struct hostent *server; //Hostent structure defines the host computer on internet {*h_name,**h_aliases_h_addrtype,h_length,**h_addr_list,h_addr = h_addr_list[0]}

    char buffer[1000];

    char *func_name, *value,*dll_name;  
    portno = atoi(argv[1]);
    server = gethostbyname("localhost"); //Returns pointer to hostent structure
    func_name = argv[2];
    value = argv[3];
    dll_name = argv[4];

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
    bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr,server->h_length); // bcopy(char*s1,char*s2,int length) copies length bytes from s1 to s2
    serv_addr.sin_port = htons(portno);

    if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR executing connect()");
        
    bzero(buffer,1000);
    // Creating a json object
    json_object *jobj = json_object_new_object();
    
    // Json Strings
    json_object *jfunction = json_object_new_string(func_name);
    json_object *jvalue = json_object_new_string(value);
    json_object *jdll = json_object_new_string(dll_name);

    // Form the json object as key-val pairs
    json_object_object_add(jobj,"FunctionName", jfunction);
    json_object_object_add(jobj,"FunctionInput", jvalue);
    json_object_object_add(jobj,"DLLName", jdll);

    if (strcpy(buffer, json_object_to_json_string(jobj)) == NULL)
    {
        error("ERROR storing data in buffer");
    }

    if (write(sockfd, buffer, strlen(buffer)) == -1)
    {
        error("ERROR executing write() in client");
    }

    printf("Data sent to Server\n");

    n = read(sockfd,buffer,1000);

    if(n<0)
        error("ERROR executing read() in client");

    printf("Message Received from server: %s\n",buffer);
    
    close(sockfd);

    return 0;

}