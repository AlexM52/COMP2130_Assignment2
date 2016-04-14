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
#include <signal.h>

#define     PORT        60000
#define     BUF_SIZE    1024
#define     LOCK_TOUT   20

//========================= SERVER.. =====================//
// struct to hold an ip:port address pair
typedef struct 
{
    unsigned long addr;
    unsigned short port;
} addr_pair;

// client job wait que linked list node
typedef struct node
{
    addr_pair *cli;
    // char *job;
    struct node *next;
} node_apq;

node_apq *cwait_que;
int elock = 0;
addr_pair *locking_cli;

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

// wait queue: add item to end of queue
int enque(node_apq **que, unsigned long addr, unsigned short port)
{
    printf("Queueing next request...\n");
    // Set current to start of queue
    node_apq *current = *que;
    // create new node
    node_apq *n = malloc(sizeof(node_apq));
    addr_pair *a = malloc(sizeof(addr_pair));
    a->addr = addr;
    a->port = port;
    n->cli = a;
    // n->job = job;
    // strcpy(n->job, job);
    n->next = NULL;
    // printf("%d, %s\n", n->cli, n->job);
    // Check current's next
    if(current == NULL)
    {
        // Empty queue, set pointer to new item
        *que = n;
    }else
    {
        // Non-empty queue, add on end
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = n;
    }
}

// Function: remove item from front of queue
int deque(node_apq **que, node_apq **tnode)
{
    // Get front item
    node_apq *front = *que;
    // If there's a next
    if(front != NULL)
    {
        printf("Dequeueing next request..\n");
        // make it the new front
        *que = (*que)->next;
    }
    
    if(front == NULL)
    {
        //set to NULL
        *tnode = NULL;
        return 0;
    }else
    {
        // set to node's addr_pair
        *tnode = front;
        // // free node
        // free(front);
        return 1;
    }
}

//open up a temporary port and send msg to specified address
int oneshot_msg(unsigned long ip, unsigned short port, char *msg)
{
    int	sock, bytes_sent;
    struct sockaddr_in	dest_addr;
    // char msg[BUF_SIZE];

    // create socket
    sock=socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0){
        printf("Could not create socket.\n");
        exit(0);
    }

    // set up socket address struct
    memset(&dest_addr, 0, sizeof (dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = ip;
    dest_addr.sin_port = port;
    
    // send
    // msg = "lock_ok";
    // sprintf(msg, "lock_ok. Lock expires after %ds of inactivity.", LOCK_TOUT);
    bytes_sent = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    
    //close socket
    close(sock);
}

void alarm_handler(int signum)
{
    printf("Lock timeout.\n");
    unlock();
}

// // locks edit access to spreadsheet to client at ip:port address
// //no need to convert
int lock(unsigned long ip, unsigned short port)
{
    printf("Locking..\n");
    // set lock
    elock = 1;
    //store locking client's addr
    locking_cli->addr = ip;
    locking_cli->port = port;
    //set alarm to timeout if this lock stays too long
    alarm(LOCK_TOUT);
    
    // notify client
    char msg[BUF_SIZE];
    sprintf(msg, "lock_ok. Lock expires after %ds of inactivity.", LOCK_TOUT);
    oneshot_msg(ip, port, msg);
    
    return 1;
}

int unlock()
{
    printf("Unlocking...\n");
    int r;
    // clear lock
    elock = 0;
    // temp struct to hold a waiting addr if in wait que
    node_apq *n ;//= malloc(sizeof(addr_pair));
    // check
    if(deque(&cwait_que, &n) != 0)
    {
        // lock with next waiting client's addr
        lock(n->cli->addr, n->cli->port);
        // free mem
        free(n->cli);
        free(n);
        r = 1;
    }else
    {
        r = 0;
    }
    
    
    return r;
}

//============================== MAIN ==============================//

int main(int argc, char *argv[])
{
    // socket vars
    int sock_main, r, selected, caddr_size, msg_size;
    struct sockaddr_in srv_addr, cli_addr;
    fd_set readfds;
    char in_buffer[BUF_SIZE];
    
    //other
    int n_tok;
    char *token[10];
    cwait_que = NULL;
    
    
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
    
    // install signal handler - lock timeout
    signal(SIGALRM, alarm_handler);
    // alloc locking_cli
    locking_cli = malloc(sizeof(addr_pair));
    int bytes_sent;             //********rem
    // server loop
    while(1)
    {
        selected = select(sock_main+1, &readfds, NULL, NULL, NULL);
        if(selected > 0)
        {
            caddr_size = sizeof(cli_addr);
            msg_size = recvfrom(sock_main, in_buffer, BUF_SIZE, 0, (struct sockaddr *)&cli_addr, &caddr_size);
            // bytes_sent = sendto(sock_main, "Hi there", strlen("Hi there"), 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
            if(msg_size > 0)
            {
                // Append null terminator to message
                in_buffer[msg_size] = '\0';
                printf("%s:%d -> %s\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port, in_buffer);
                split_input(in_buffer, token);
                if(strcmp(token[0], "edit_lock") == 0)// edit_lock
                {
                    if(elock == 0)
                    {
                        lock(cli_addr.sin_addr.s_addr, cli_addr.sin_port);
                    }else
                    {
                        if((locking_cli->addr == cli_addr.sin_addr.s_addr) && (locking_cli->port == cli_addr.sin_port))
                        {
                            oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "You are already locked.");
                        }else
                        {
                            // que request
                            enque(&cwait_que, cli_addr.sin_addr.s_addr, cli_addr.sin_port);
                            oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "lock_wait");
                        }
                    }
                }else if(strcmp(token[0], "edit") == 0)
                {
                    if((elock == 1) && (locking_cli->addr == cli_addr.sin_addr.s_addr) && (locking_cli->port == cli_addr.sin_port))
                    {
                        r = 0;
                        while(token[r] != NULL)
                        {
                            printf("-%s\n", token[r]);
                            r++;
                        }
                        //reset timout
                        alarm(LOCK_TOUT);
                        oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "ok");
                    }else
                    {
                        oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "You do not have lock. (use 'edit_lock')");
                    }
                }else if(strcmp(token[0], "edit_unlock") == 0)
                {
                    printf("unlocking\n");
                    unlock();
                    // notify client
                    oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "unlock_ok");
                }
            }
        }
        if(strcmp(in_buffer, "stop") == 0) break;
    }
    
    char mout[BUF_SIZE] = "Stopping server.";
    bytes_sent = sendto(sock_main, mout, strlen(mout), 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
    
    // close socket and exit
    printf("Closing server socket...\n");
    close(sock_main);
    printf("DONE.\n");
    
    return 0;
}