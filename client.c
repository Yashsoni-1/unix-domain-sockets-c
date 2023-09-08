#include <stdio.h>
#include <sys/un.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

int main(int argc, char* argv[])
{
    
    struct sockaddr_un name;
    
    long ret;
    int data;
    int data_sock;
    char buffer[BUFFER_SIZE];
    
    data_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if(data_sock == -1)
    {
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    
    printf("Socket created... \n");
    
    memset(&name, 0, sizeof(struct sockaddr_un));
    
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME,
            sizeof(name.sun_path) - 1);
    
    
    ret = connect(data_sock,
                  (struct sockaddr *)&name,
                  sizeof(struct sockaddr_un));
    
    if(ret == -1)
    {
        perror("Connect");
        exit(EXIT_FAILURE);
    }
    
    do {
        printf("Enter the number to send to the server : \n");
        scanf("%u", &data);
        
        ret = write(data_sock, &data, sizeof(int));
        
        if(ret == -1)
        {
            perror("Write");
            break;
        }
        
        printf("No of bytes sent = %ld, data sent = %d\n", ret, data);
    } while(data);
    
    memset(&buffer, 0, BUFFER_SIZE);
    
    ret = read(data_sock, buffer, BUFFER_SIZE);
    
    if(ret == -1)
    {
        perror("Read");
        exit(EXIT_FAILURE);
    }
    
    printf("Result recvd from server %s\n", buffer);
    
    close(data_sock);
    
    exit(EXIT_SUCCESS);
    
    return 0;
}

