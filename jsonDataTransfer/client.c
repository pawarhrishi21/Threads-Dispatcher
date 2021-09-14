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
    /*Creating a json object*/
    json_object *jobj = json_object_new_object();

    /*Creating a json string*/
    json_object *jstring = json_object_new_string("Joys of Programming");

    /*Creating a json integer*/
    json_object *jint = json_object_new_int(10);

    /*Creating a json boolean*/
    json_object *jboolean = json_object_new_boolean(1);

    /*Creating a json double*/
    json_object *jdouble = json_object_new_double(2.14);

    /*Creating a json array*/
    json_object *jarray = json_object_new_array();

    /*Creating json strings*/
    json_object *jstring1 = json_object_new_string("c");
    json_object *jstring2 = json_object_new_string("c++");
    json_object *jstring3 = json_object_new_string("php");

    /*Adding the above created json strings to the array*/
    json_object_array_add(jarray,jstring1);
    json_object_array_add(jarray,jstring2);
    json_object_array_add(jarray,jstring3);

    /*Form the json object*/
    /*Each of these is like a key value pair*/
    json_object_object_add(jobj,"Site Name", jstring);
    json_object_object_add(jobj,"Technical blog", jboolean);
    json_object_object_add(jobj,"Average posts per day", jdouble);
    json_object_object_add(jobj,"Number of posts", jint);
    json_object_object_add(jobj,"Categories", jarray);

    printf("Size of JSON object- %lu\n", sizeof(jobj));
    printf("Size of JSON_TO_STRING- %lu,\n %s\n", sizeof(json_object_to_json_string(jobj)), json_object_to_json_string(jobj));

    //printf("Size of string- %lu\n", sizeof(json_object_to_json_string(jobj)));
    
    char temp_buff[1000];

    if (strcpy(temp_buff, json_object_to_json_string(jobj)) == NULL)
    {
        perror("strcpy");
        return EXIT_FAILURE;
    }

    if (write(sockfd, temp_buff, strlen(temp_buff)) == -1)
    {
        perror("write");
        return EXIT_FAILURE;
    }

    printf("Written data\n");


    // n = write(sockfd,&msg,255);
    // if(n<0)
    //     error("ERROR in write() in client");
    // printf("Sent to server\n");

    n = read(sockfd,buffer,255);
    if(n<0)
        error("ERROR in read() in client");
    printf("Message Recieved from server: %s\n",buffer);
    
    close(sockfd);
    return 0;

}