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

int main(int argc, char *argv[])
{
	int	sock;
    struct sockaddr_in	dest_addr;
    char msg[50];//, buf[BUF_SIZE];
    int	bytes_sent;
    struct sockaddr_in r_addr;
    int r_addr_size, bytes_rcvd;
    char rbuf[BUF_SIZE];

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

    // main loop
    while (1){
        printf(">> ");
        fgets(msg, 19, stdin);
        stripnl(msg);
        if (strncmp(msg ,"quit", 4) == 0)
            break;
        bytes_sent = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
        r_addr_size = sizeof(r_addr);
        bytes_rcvd = recvfrom(sock, rbuf, BUF_SIZE, 0, (struct sockaddr *)&r_addr, &r_addr_size);
        if(bytes_rcvd > 0)
        {
            // Append null terminator to message
            rbuf[bytes_rcvd] = '\0';
            // printf("Response: %s\n", rbuf);
            
            if(strcmp(rbuf, "lock_wait")==0)
            {
                printf("Request queued. Waiting...");
                fflush(stdout);
                bytes_rcvd = recvfrom(sock, rbuf, BUF_SIZE, 0, (struct sockaddr *)&r_addr, &r_addr_size);
                if(bytes_rcvd > 0)
                {
                    rbuf[bytes_rcvd] = '\0';
                    printf("\n%s\n", rbuf);
                }else
                {
                    printf("Error: empty response.\n");
                }
            }else
            {
                printf("Response: %s\n", rbuf);
            }
        }
    }

    // close socket and exit
    printf("Closing socket...\n");
    close(sock);
    printf("DONE.\n");
	return 0;
}