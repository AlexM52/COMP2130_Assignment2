/**
@filename:	spreadsheet_server.c
@author:	Lewainne Vidal(620052899), Alex McDoom(62-0025254)
@course:	COMP2130 (Systems Programming)
@lecturer:	Kevin Miller
@dateDue:	April 14, 2016
@task:	Assignment 2 - Implement spreadsheet client	
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE	1024
#define	SERVER_IP	"127.0.0.1"
#define SERVER_PORT	60000

// if a string ends with a \n, replace it with a \0
// otherwise, no change. Returns 1 if stripped \n, else 0.
int stripnl(char *string)
{
	int last;
	last = strlen(string)-1;
	if(string[last] == '\n')
	{
		string[last] = '\0';
		return 1;
	}else
	{
		return 0;
	}
}

// int main(int argc, char *argv[])
// {
// 	int	sock, r;
//     struct sockaddr_in	dest_addr;
//     // const int BUF_SIZE = 1024;
//     // const char SERVER_IP[] = "127.0.0.1";
//     // const int SERVER_PORT = 60000;
//     char msg[50];//, buf[BUF_SIZE];
//     int	send_len, bytes_sent;

//     // create socket
//     sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//     if (sock < 0){
//         printf("Could not create socket.\n");
//         exit(0);
//     }

//     // set up socket address struct
//     memset(&dest_addr, 0, sizeof (dest_addr));
//     dest_addr.sin_family = AF_INET;
//     dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
//     dest_addr.sin_port = htons((unsigned short)SERVER_PORT);

//     // connect to the server
//     r = connect(sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
//     if (r < 0){
//         printf("Could not connect to server.\n");
//         exit(0);
//     }

//     // main loop
//     while (1){
//         printf(">> ");
//         // scanf("%s", msg);
//         fgets(msg, 19, stdin);
//         // printf("%s\n", msg);
//         stripnl(msg);
//         if (strncmp(msg ,"quit", 4) == 0)
//             break;
//         // strcpy(buf,text);
//         send_len = strlen(msg);
//         // bytes_sent=send(sock_send, buf, send_len,0);
//         bytes_sent = send(sock, msg, send_len, 0);
//     }

//     // close socket and exit
//     printf("Closing socket...\n");
//     close(sock);
//     printf("DONE.\n");
// 	return 0;
// }

int main(int argc, char *argv[])
{
	int	sock;//, r;
    struct sockaddr_in	dest_addr;
    // const int BUF_SIZE = 1024;
    // const char SERVER_IP[] = "127.0.0.1";
    // const int SERVER_PORT = 60000;
    char msg[50], buf[BUF_SIZE];
    int	send_len, bytes_sent;

    // create socket
    sock=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0){
        printf("Could not create socket.\n");
        exit(0);
    }

    // set up socket address struct
    memset(&dest_addr, 0, sizeof (dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    dest_addr.sin_port = htons((unsigned short)SERVER_PORT);

    // // connect to the server
    // r = connect(sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    // if (r < 0){
    //     printf("Could not connect to server.\n");
    //     exit(0);
    // }

    // main loop
    while (1){
        printf(">> ");
        // scanf("%s", msg);
        fgets(msg, 19, stdin);
        // printf("%s\n", msg);
        stripnl(msg);
        if (strncmp(msg ,"quit", 4) == 0)
            break;
        strcpy(buf,msg);
        // send_len = strlen(msg);
        // bytes_sent=send(sock_send, buf, send_len,0);
        bytes_sent = sendto(sock, buf, strlen(msg), 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    }

    // close socket and exit
    printf("Closing socket...\n");
    close(sock);
    printf("DONE.\n");
	return 0;
}