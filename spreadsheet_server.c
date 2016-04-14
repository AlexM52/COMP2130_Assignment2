/**
@filename:	spreadsheet_server.c
@author:	Lewainne Vidal(620052899), Alex McDoom(62-0025254)
@course:	COMP2130 (Systems Programming)
@lecturer:	Kevin Miller
@dateDue:	April 14, 2016
@task:	Assignment 2 - Implement spreadsheet server	
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define     PORT        60000
#define     BUF_SIZE    1024


// Takes a command input string and an array of strings as args.
// tokenizes string and stores the tokens in the array. Returns 
// # of tokens.
int split_input(char *string, char *array[])
{
    array[0] = strtok(string, " \0");
    int n = 1;
    char *t;
    while(t = strtok(NULL, " \0"))
    {
    	array[n] = t;
    	n++;
    }
    array[n] = NULL;
    return n;
}

int main(int argc, char *argv[])
{
    int sock_main, r, selected, caddr_size, msg_size;//, highest;
    struct sockaddr_in srv_addr, cli_addr;
    fd_set readfds;
    // struct timeval timeout={0,0};
    char in_buffer[BUF_SIZE];
    
    // set up listening socket
    sock_main = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_main < 0){
        printf("Could not create socket.\n");
        exit(0);
    }
    // set up local socket struct
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons((unsigned short)PORT);
    // bind to port
    r = bind(sock_main, (struct sockaddr *) &srv_addr, sizeof (srv_addr));
    if (r < 0){
        printf("Could not bind to port %d.\n", PORT);
        exit(0);
    }
    // Clear readfds and add main socket
    FD_ZERO(&readfds);
    FD_SET(sock_main, &readfds);
    while(1)
    {
        selected = select(sock_main+1, &readfds, NULL, NULL, NULL);
        if(selected > 0)
        {
            caddr_size = sizeof(cli_addr);
            msg_size = recvfrom(sock_main, in_buffer, BUF_SIZE, 0, (struct sockaddr *)&cli_addr, &caddr_size);
            if(msg_size > 0)
            {
                // Append null terminator to message
                in_buffer[msg_size] = '\0';
                printf("%s:%d -> %s\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port, in_buffer);
            }
        }
        if(strcmp(in_buffer, "stop") == 0) break;
    }
    
    // close socket and exit
    printf("Closing server socket...\n");
    close(sock_main);
    printf("DONE.\n");
}