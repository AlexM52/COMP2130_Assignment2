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

const int maxNumRows = 9;

typedef struct{
	char operand[4];
}opSet;//this allows for an array of operands instead of a list concatenation

typedef struct{
	char operation[10];
	opSet operandSet[2];//declares spaces for 10 possible operands, each having a max of 10 elements
}formulaWhole;//allows one to store the various formula elements

//Simple operations - the above structs have not been included as yet
typedef struct{
	char text[20];
	formulaWhole formula;
	float numeric;
}contentType;

typedef struct{
	contentType content;
}workbook;

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

// // receive a sheet
// int rcv_sheet(int sock, struct sockaddr_in rad, int rlen)
// {
    
// }

//Show Sheet
void showSheet( workbook spreadSheet[][maxNumRows] ){
	int numRows = sizeof(spreadSheet[0])/sizeof(spreadSheet[0][0]);
	int i,j;
	int ascii = 65;
	
	printf("\n\n\t");

	//Display headers (A-I)
	for (i=ascii; i<(ascii+numRows); i++){
		printf("%c\t",i);
	}
	printf("\n");	
	//Display other content
	for(i=0; i<numRows; i++){
		printf("%d",i+1); //displays row number (1-9)
		for(j=0; j<numRows; j++){
			//display content only if data has been entered; otherwise, show empty values
			if(strlen(spreadSheet[i][j].content.text)>0)
				printf("\t%s",spreadSheet[i][j].content.text);
			else
				printf("\t%.2f",spreadSheet[i][j].content.numeric);
		}
		printf("\n");
	}
}//end show sheet

//Initialize Sheet
void initializeSheet( workbook spreadSheet[][maxNumRows] ){
	int numRows = sizeof(spreadSheet[0])/sizeof(spreadSheet[0][0]);
	int i,j;	

	//Initialize Content
	for (i=0; i<numRows; i++){
		for(j=0; j<numRows; j++){
			strcpy(spreadSheet[i][j].content.text,"");
			strcpy(spreadSheet[i][j].content.formula.operation,"");
			strcpy(spreadSheet[i][j].content.formula.operandSet[0].operand,"");
			strcpy(spreadSheet[i][j].content.formula.operandSet[1].operand,"");
			spreadSheet[i][j].content.numeric = 0;
		}
	}
}//end initialize sheet

int main(int argc, char *argv[])
{
	int	sock;
    struct sockaddr_in	dest_addr;
    char msg[50];//, buf[BUF_SIZE];
    int	bytes_sent;
    struct sockaddr_in r_addr;
    int r_addr_size, bytes_rcvd;
    char rbuf[BUF_SIZE];
    
    char *token[10];
    workbook sheet[9][maxNumRows];
    initializeSheet(sheet);

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
            }else if(strcmp(rbuf, "sending_sheet")==0)
            {
                int x, y;
                // split_input(rbuf, token);
                bytes_rcvd = recvfrom(sock, rbuf, BUF_SIZE, 0, (struct sockaddr *)&r_addr, &r_addr_size);
                split_input(rbuf, token);
                while(strcmp(token[0], "done") != 0)
                {
                    x = atoi(token[1]);
                    y = atoi(token[2]);
                    strcpy(sheet[x][y].content.text, token[3]);
                    // printf("%d, %d, %s\n", x, y, token[3]);
                    bytes_rcvd = recvfrom(sock, rbuf, BUF_SIZE, 0, (struct sockaddr *)&r_addr, &r_addr_size);
                    split_input(rbuf, token);
                }
                showSheet(sheet);
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