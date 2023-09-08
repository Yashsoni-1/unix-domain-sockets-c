#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128


int main(int argc, char *argv[])
{
    
    struct sockaddr_un name;
    long ret;
    int connection_socket;
    int data_socket;
    int result;
    int data;
    char buffer[BUFFER_SIZE];
    
    unlink(SOCKET_NAME);
    
    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if(connection_socket == -1)
    {
        perror("connection socket\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Master socket created\n");
    
    memset(&name, 0, sizeof(struct sockaddr_un));
    
    name.sun_family = AF_UNIX;
    
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);
    
    ret = bind(connection_socket,
               (struct sockaddr *)&name,
               sizeof(struct sockaddr_un));
    
    if(ret == -1)
    {
        perror("Bind\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Bind() call success\n");
    
    ret = listen(connection_socket, 20);
    
    for(;;)
    {
        printf("Waiting on accept() sys call\n");
        
        data_socket = accept(connection_socket, NULL, NULL);
        
        if(data_socket == -1)
        {
            perror("Accept");
            exit(EXIT_FAILURE);
        }
        
        printf("Connection accepted from client\n");
        
        result = 0;
        
        for(;;)
        {
            memset(buffer, 0, BUFFER_SIZE);
            
            printf("Waiting for the data from the client \n");
            
            ret = read(data_socket, buffer, sizeof(int));
            
            if(ret == -1)
            {
                perror("Read\n");
                exit(EXIT_FAILURE);
            }
            
            memcpy(&data, buffer, sizeof(int));
            
            if(data == 0) break;
            
            result += data;
        }
        
        
        memset(buffer, 0, BUFFER_SIZE);
        
        sprintf(buffer, "Result = %d\n", result);
        
        ret = write(data_socket, buffer, BUFFER_SIZE);
        
        if(ret == -1)
        {
            perror("Write\n");
            exit(EXIT_FAILURE);
        }
        
        close(data_socket);
    }
    
    
    close(connection_socket);
    printf("Connection closed..\n");
    
    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);
    return 0;
}

