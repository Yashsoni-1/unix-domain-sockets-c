#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128
#define MAX_CLIENT_SUPPORTED 20

int monitored_set[MAX_CLIENT_SUPPORTED];

int client_data[MAX_CLIENT_SUPPORTED] = {0};

static void
init_monitored_set(void)
{
    for(int i=0; i < MAX_CLIENT_SUPPORTED; ++i)
    {
        monitored_set[i] = -1;
    }
}

static void
reinit_readfds(fd_set *readfds)
{
    FD_ZERO(readfds);
    
    for(int i=0; i<MAX_CLIENT_SUPPORTED; ++i)
    {
        if(monitored_set[i] != -1)
        {
            FD_SET(monitored_set[i], readfds);
        }
    }
}

static void
add_to_monitored_set(int fd)
{
    for(int i=0; i < MAX_CLIENT_SUPPORTED; ++i)
    {
        if(monitored_set[i] != -1) continue;
        
        monitored_set[i] = fd;
        break;
    }
}

static void
remove_from_monitored_set(int fd)
{
    for(int i=0; i < MAX_CLIENT_SUPPORTED; ++i)
    {
        if(monitored_set[i] == fd){
            monitored_set[i] = -1;
            break;
        }
    }
}



static int
get_max_fd(void)
{
    int max = -1;
    
    for(int i=0; i<MAX_CLIENT_SUPPORTED; ++i)
    {
        if(monitored_set[i] > max)
            max = monitored_set[i];
    }
    
    return max;
}



int main(int argc, char *argv[])
{
    struct sockaddr_un name;
    
    int master_socket_fd;
    int comm_socket_fd;
    
    char buffer[BUFFER_SIZE];
    
    int ret;
    int data;
    int result;
    
    fd_set readfds;
    
    unlink(SOCKET_NAME);
    init_monitored_set();
    
    master_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if(master_socket_fd == -1)
    {
        perror("Master Socket");
        exit(EXIT_FAILURE);
    }
    
    printf("Master socket created...\n");
    
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);
    
    ret = bind(master_socket_fd,
               (struct sockaddr *)&name,
               sizeof(struct sockaddr_un));
    
    if(ret == -1)
    {
        perror("Bind() ");
        exit(EXIT_FAILURE);
    }
    
    printf("Bind() call success..\n");
    
    
    ret = listen(master_socket_fd, 20);
    
    add_to_monitored_set(master_socket_fd);
    
    for(;;)
    {
        reinit_readfds(&readfds);
        
        printf("Blocked on select system call...\n");
        
        select(get_max_fd() + 1,
                     &readfds, NULL, NULL, NULL);
        
        if(FD_ISSET(master_socket_fd, &readfds))
        {
            printf("New connection request recvd, accept the connection\n");
            
            comm_socket_fd = accept(master_socket_fd,
                                    NULL, NULL);
            
            if(comm_socket_fd == -1)
            {
                perror("accept() ");
                break;
            }
            
            printf("Connection established\n");
            
            add_to_monitored_set(comm_socket_fd);
        }
        else
        {
            comm_socket_fd = -1;
            
            for(int i=0; i<MAX_CLIENT_SUPPORTED; ++i)
            {
                if(FD_ISSET(monitored_set[i], &readfds))
                {
                    comm_socket_fd = monitored_set[i];
                    
                    printf("Server ready to server the client\n");
                    
                    memset(buffer, 0, BUFFER_SIZE);
                    
                    printf("Waiting for the data from the client \n");
                    
                    ret = read(comm_socket_fd, buffer, sizeof(int));
                    
                    if(ret == -1)
                    {
                        perror("Read\n");
                        exit(EXIT_FAILURE);
                    }
                    
                    memcpy(&data, buffer, sizeof(int));
                    
                    if(data == 0) {
                        memset(buffer, 0, BUFFER_SIZE);
                        
                        sprintf(buffer, "Result = %d\n", client_data[i]);
                        
                        printf("Sending final result to client\n");
                        
                        ret = write(comm_socket_fd, buffer, BUFFER_SIZE);
                        
                        if(ret == -1)
                        {
                            perror("Write\n");
                            exit(EXIT_FAILURE);
                        }
                        
                        client_data[i] = 0;
                        remove_from_monitored_set(comm_socket_fd);
                        close(comm_socket_fd);
                        continue;
                    }
                    
                    client_data[i] += data;
                }
            }
        }
    }
    
    close(master_socket_fd);
    printf("Connection closed..\n");
    remove_from_monitored_set(master_socket_fd);
    
    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);
    
    return 0;
}

