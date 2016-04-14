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

// // manage client socket
// int cli_sock_loop(int socket)
// {
//     // const int BUF_SIZE = 1024;
//     int rcvd_bytes;
//     char in_buffer[BUF_SIZE];
//     // sock_cli=accept(sock_main, (struct sockaddr *) &cli_addr, &addr_size);

//     // handle client requests
//     char *args[5];
//     char *t;
//     int n;
//     int id;
//     while(1)
//     {
//     	rcvd_bytes = recv(socket, in_buffer, BUF_SIZE, 0);
//     	in_buffer[rcvd_bytes]=0;
//         printf("Received: %s\n", in_buffer);
// 	    n = split_input(in_buffer, args);
// 	    // if (strcmp(in_buffer, "finish") == 0)
// 	    //     break;
// 	    if (strcmp(args[0], "finish") == 0)
// 	        break;
// 	    // args[0] = strtok(in_buffer, " \n\0");
// 	    // n = 1;
// 	    // while(t = strtok(NULL, " \n\0"))
// 	    // {
// 	    // 	args[n] = t;
// 	    // 	n++;
// 	    // }
// 	    // // args[0] = in_buffer;
// 	    // // args[1] = NULL;
// 	    // args[n] = NULL;
// 	    // id = fork();
// 	    // if(id == 0)
// 	    // {
// 	    // 	execvp(args[0], args);
// 	    // }else
// 	    // {
// 	    // 	wait();
// 	    // 	printf("\n");
// 	    // }
	    
// 	    // print tokens just to verify they were processed properly
// 	    printf("Tokens:\n");
// 	    int x;
// 	    for(x=0; x<n; x++)
// 	    {
// 	        printf(" -%s\n", args[x]);
// 	    }
//     }

//     // close sockets and exit
//     printf("Closing client socket...\n");
//     close(socket);
// }

// int main(int argc, char *argv[])
// {
// 	int	sock_main, sock_cli, r, addr_size;
//     struct sockaddr_in	my_addr, cli_addr;
//     // const unsigned short PORT = 60000;
//     // const int BUF_SIZE = 1024;
//     // int rcvd_bytes;
//     // char in_buffer[BUF_SIZE];


//     // set up listening socket
//     sock_main=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//     if (sock_main < 0){
//         printf("Could not create socket.\n");
//         exit(0);
//     }
//     // set up local socket struct
//     memset(&my_addr, 0, sizeof(my_addr));
//     my_addr.sin_family = AF_INET;
//     my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     my_addr.sin_port = htons(PORT);
//     // bind to port
//     // int r;
//     r=bind(sock_main, (struct sockaddr *) &my_addr, sizeof (my_addr));
//     if (r < 0){
//         printf("Could not bind to port %d.\n", PORT);
//         exit(0);
//     }
//     // start listening
//     r=listen(sock_main, 5);
//     if (r < 0){
//         printf("Could not listen on main socket.\n");
//         exit(0);
//     }

//     // set up socket to manage client
//     addr_size=sizeof(cli_addr);
//     sock_cli=accept(sock_main, (struct sockaddr *) &cli_addr, &addr_size);
//     // cli_sock_loop(sock_cli);
//     int id;
//     id = fork();
//     if(id == 0)
//     {
//         printf("%d\n", sock_cli);
//         cli_sock_loop(sock_cli);
//         return 0;
//     }else
//     {
//         wait(-1);
//     }

//     // // handle client requests
//     // char *args[5];
//     // char *t;
//     // int n;
//     // int id;
//     // while(1)
//     // {
//     // 	rcvd_bytes = recv(sock_cli, in_buffer, BUF_SIZE, 0);
//     // 	in_buffer[rcvd_bytes]=0;
//     //     printf("Received: %s\n", in_buffer);
// 	   // n = split_input(in_buffer, args);
// 	   // // if (strcmp(in_buffer, "finish") == 0)
// 	   // //     break;
// 	   // if (strcmp(args[0], "finish") == 0)
// 	   //     break;
// 	   // // args[0] = strtok(in_buffer, " \n\0");
// 	   // // n = 1;
// 	   // // while(t = strtok(NULL, " \n\0"))
// 	   // // {
// 	   // // 	args[n] = t;
// 	   // // 	n++;
// 	   // // }
// 	   // // // args[0] = in_buffer;
// 	   // // // args[1] = NULL;
// 	   // // args[n] = NULL;
// 	   // // id = fork();
// 	   // // if(id == 0)
// 	   // // {
// 	   // // 	execvp(args[0], args);
// 	   // // }else
// 	   // // {
// 	   // // 	wait();
// 	   // // 	printf("\n");
// 	   // // }
	    
// 	   // // print tokens just to verify they were processed properly
// 	   // printf("Tokens:\n");
// 	   // int x;
// 	   // for(x=0; x<n; x++)
// 	   // {
// 	   //     printf(" -%s\n", args[x]);
// 	   // }
//     // }

//     // // close sockets and exit
//     // printf("Closing client socket...\n");
//     // close(sock_cli);
//     printf("Closing main socket...\n");
//     close(sock_main);
//     printf("DONE.\n");
// 	return 0;
// }

int main(int argc, char *argv[])
{
    int sock_main, r, selected, caddr_size, msg_size;//, highest;
    struct sockaddr_in srv_addr, cli_addr;
    fd_set readfds;//, active_fd_set, read_fd_set;
    struct timeval timeout={0,0};
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
    // int r;
    r = bind(sock_main, (struct sockaddr *) &srv_addr, sizeof (srv_addr));
    if (r < 0){
        printf("Could not bind to port %d.\n", PORT);
        exit(0);
    }
    // Clear readfds and add main socket
    FD_ZERO(&readfds);
    FD_SET(sock_main, &readfds);
    // highest = sock_main
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
                // printf(":%d -> %s\n", cli_addr.sin_port, in_buffer);
                // printf("From %s:%d received: %s\n",inet_ntoa(cli_addr.sin_addr), srv_addr.sin_port, in_buffer);
            }
        }
        if(strcmp(in_buffer, "stop") == 0) break;
    }
    
    // close socket and exit
    printf("Closing main socket...\n");
    close(sock_main);
    printf("DONE.\n");
}