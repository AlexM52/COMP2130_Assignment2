/**
@filename:	spreadsheet_server.c
@author:	Lewainne Vidal(620052899), Alex McDoom(62-0025254)
@course:	COMP2130 (Systems Programming)
@lecturer:	Kevin Miller
@dateDue:	April 15, 2016
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
#include <ctype.h>
#include <math.h>

#define     PORT        60000
#define     BUF_SIZE    1024
#define     LOCK_TOUT   60


/*Constants*/
const int maxNumRows = 9;

/*Enum Definitions*/
typedef enum {
	false,
	true
}bool;

/*Struct Definitions*/
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



/*Function Prototypes*/
//Static
void initializeSheet(workbook[][maxNumRows]);
void showSheet(workbook[][maxNumRows]);
void viewFormulas( workbook[][maxNumRows] );
int determineFirstIndex(char*);
int determineSecondIndex(char*);
int findMax(int, int);
int findMin(int, int);
//Analyse Data
char upperCase(char);//converts a lower case element to upper case
char lowerCase(char);//converts an upper case element to lower case
char* determineOperands(char*);//finds all the operands specified
const char * determineOperation(char*,int);//finds the type of operation entered
bool validateFormula(char*);//validates a function/formula entry
bool validateString(char*);//validates a text/mixed entry
bool validateNumeric(char*);//validates a numeric entry
//Input Data & Processing
float doCalculation(char*, workbook[][maxNumRows]);
void placeData(char*,int,int,workbook[][maxNumRows]);



/*Function Definitions*/
//Find Max
int findMax(int cellPart1, int cellPart2){
	int maxVal;
	
	if(cellPart1 > cellPart2){
		maxVal = cellPart1;
	}else if(cellPart2 > cellPart1){
		maxVal = cellPart2;
	}else{
		maxVal = cellPart1;
	}

	return maxVal;
}//end Find max

//Find Min
int findMin(int cellPart1, int cellPart2){
	int minVal;
	
	if(cellPart1 < cellPart2){
		minVal = cellPart1;
	}else if(cellPart2 < cellPart1){
		minVal = cellPart2;
	}else{
		minVal = cellPart1;
	}

	return minVal;
}//end Find min

//Get first/open bracket index
int determineFirstIndex(char *cellContent){
	int i;
	int firstIndex=0;
	
	for(i=0; i<strlen(cellContent); i++)
		if((int)cellContent[i]==40)
			firstIndex=i;

	return firstIndex;
}//end determine first index

//Get closing bracket index
int determineSecondIndex(char *cellContent){
	int i;
	int secondIndex=0;
	
	for(i=0; i<strlen(cellContent); i++)
		if((int)cellContent[i]==41)
			secondIndex=i;
	
	return secondIndex;
}//end determine second index

//Validate Function
bool validateFormula(char *cellContent){
	int i,j;//counters
	int fIndex,sIndex;
	//Functions
	char sum[]={"sum"};
	char range[]={"range"};
	char average[]={"average"};
	//Extra functions
	char product[]={"product"};
	char quotient[]={"quotient"};
	char power[]={"power"};
		
	char descriptionHolder[10],operands[5];
	int operandsLength;
	
	//counters for various boolen checks
	int oBcnt=0,cBcnt=0;//open bracket count, close bracket count
	int nlCount=0,lnCount=0,rowMatch=0,columnMatch=0;//num-letter, row and column matching
	int charIntConverter1, charIntConverter2;
	
	bool validOperandLength,validOperationLength,operationValid,equalValid,commaPresent;
	bool bracketsPresent,operandOrderValid,matchRowColumn=true,validFormula;	

	fIndex = determineFirstIndex(cellContent);
	sIndex = determineSecondIndex(cellContent);
	strcpy(descriptionHolder,determineOperation(cellContent,fIndex));	
	operandsLength = (sIndex - fIndex) -1;
	//Boolean Checks (Part 1)
	equalValid = (int)cellContent[0] == 61;//checks if the ascii value of the first element of the entry matches that of an equal sign
	commaPresent = (int)cellContent[fIndex+3]==44;
	validOperandLength = operandsLength == 5;
	validOperationLength = (strlen(descriptionHolder) >= strlen(sum)) && (strlen(descriptionHolder) <= strlen(quotient));
	operationValid = (strcmp(descriptionHolder,sum)==0 || strcmp(descriptionHolder,range)==0 || strcmp(descriptionHolder,average)==0 || strcmp(descriptionHolder,product)==0 || strcmp(descriptionHolder,quotient)==0 || strcmp(descriptionHolder,power)==0);	
	
	/*Functions for doing other boolean validations*/
	//Open vs. Close Bracket Count
	for(i=0; i<strlen(cellContent); i++){
		if((int)cellContent[i]==40)
			oBcnt+=1;
		else if((int)cellContent[i]==41)
			cBcnt+=1;
	}
	//Number Letter Count
	for(i=(fIndex+1); i<sIndex; i++){
		if((int)upperCase(cellContent[i])>=65 && (int)upperCase(cellContent[i])<=73){
			if(isdigit(cellContent[i+1])){
				lnCount+=1;
			}
		}else if(isdigit(cellContent[i])){
			if((int)upperCase(cellContent[i-1])>=65 && (int)upperCase(cellContent[i-1])<=73){
				nlCount+=1;
			}
		}
	}
	//Row Match vs. Column Match
	if(strcmp(descriptionHolder,sum)==0 || strcmp(descriptionHolder,average)==0 || strcmp(descriptionHolder,range)==0){
		for(i=fIndex+1; i<sIndex; i++){
			if((int)upperCase(cellContent[i])>=65 && (int)upperCase(cellContent[i])<=73){
				if((int)upperCase(cellContent[i]) == (int)upperCase(cellContent[i+3])){
					columnMatch+=1;
				}
			}else if(isdigit(cellContent[i])){
				charIntConverter1 = cellContent[i];
				charIntConverter2 = cellContent[i+3];
				if(charIntConverter1 == charIntConverter2){
					rowMatch+=1;
				}
			}
		}
		matchRowColumn = (rowMatch == 1 || columnMatch ==1);//re assign the variable if the operation is either sum, average or range
	}
	
	//Boolean Checks (Part 2)
	bracketsPresent = (oBcnt == 1) && (cBcnt == 1);
	operandOrderValid = (lnCount == nlCount && (lnCount > 1));
	validFormula = equalValid && commaPresent && validOperationLength && validOperandLength && operationValid && bracketsPresent && matchRowColumn && operandOrderValid;
	
	return validFormula;
}//end validate formula

//Validate Numeric
bool validateNumeric(char *cellContent){
	bool numValid=false;
	int i,j;
	int digitCount=0;
	
	for(i=0; i<strlen(cellContent); i++)
		if(isdigit(cellContent[i]))
			digitCount+=1;
			
	numValid = digitCount==strlen(cellContent);	
	return numValid;
}//end validate numeric

//Validate String
bool validateString(char *cellContent){
	bool strValid = false;
	int i,j;
	int charCount=0;//should be char count

	for(i=0; i<strlen(cellContent); i++)
		if(!(isdigit(cellContent[i])))
			charCount+=1;
	
	strValid = (charCount>0) && ((int)cellContent[0] != 61);
	return strValid;
}//end validate string


//Determine Operation - needed to determine the type of calculation needed
const char* determineOperation(char *cellContent, int index_first){
	//Counter Variables
	int i,j;
	static char descriptionHolder[10];//stores the operation found in one variable for comparisons later
	
	for(i=1; i<determineFirstIndex(cellContent); i++)
		descriptionHolder[i-1] = lowerCase(cellContent[i]);
		
	return descriptionHolder;
}//end determine operation

//Determine Operands
char* determineOperands(char *cellContent){
	int i,j=-1;
	int opIndex = -1;
	int rangeOperands = (determineSecondIndex(cellContent) - determineFirstIndex(cellContent))- 1;
	static char operandsFound[3];

	for(i=determineFirstIndex(cellContent)+1; i<determineSecondIndex(cellContent); i++){
		if(	(int)cellContent[i] != 44 ){
			opIndex+=1;
			operandsFound[opIndex] = cellContent[i];
		}
	}
	
	return operandsFound;
}//end determine operands

//Do Calculation
float doCalculation(char* cellContent, workbook spreadSheet[][maxNumRows]){
	int i,j,k=-1,count=0;//counter variables
	int paramCindex1,paramRindex1;//stores the column and row values in terms of array coordinates for the first operand
	int paramCindex2,paramRindex2;//stores the column and row values in terms of array coordinates for the second operand
	float result=0,minValue,maxValue;
	int modIndex=-1;//used to help in comparing similar arrays at different starting points
	float swap;//used to help in ordering elements for max/min comparison
	
	char operationEntered[10];//will store the result of the 'operationFound' function
	char operands[4];

	strcpy(operationEntered, determineOperation(cellContent,determineFirstIndex(cellContent)));
	strcpy(operands, determineOperands(cellContent));
	
	paramCindex1 = (int)upperCase(operands[0]) - 65;
	paramRindex1 = ((int)operands[1] - 48) - 1;
	paramCindex2 = (int)upperCase(operands[2]) - 65;
	paramRindex2 = ((int)operands[3] - 48) - 1;

	if(strcmp(operationEntered,"sum")==0){
		if(paramCindex1 == paramCindex2){
			for(i=findMin(paramRindex2,paramRindex1); i<=findMax(paramRindex2,paramRindex1); i++){
				result += (float)spreadSheet[i][paramCindex1].content.numeric;
			}
		}else if(paramRindex1 == paramRindex2){
			for(i=findMin(paramCindex2,paramCindex1); i<=findMax(paramCindex2,paramCindex1); i++){
				result += (float)spreadSheet[paramRindex1][i].content.numeric;
			}
		}					
	}else if(strcmp(operationEntered,"average")==0){
		if(paramCindex1 == paramCindex2){
			for(i=findMin(paramRindex2,paramRindex1); i<=findMax(paramRindex2,paramRindex1); i++){
				result += (float)spreadSheet[i][paramCindex1].content.numeric;
				count += 1;//increment for each element that is included in the sum
			}
		}else if(paramRindex1 == paramRindex2){
			for(i=findMin(paramCindex2,paramCindex1); i<=findMax(paramCindex2,paramCindex1); i++){
				result += (float)spreadSheet[paramRindex1][i].content.numeric;
				count += 1;
			}
		}
		result = result/count;//divide the cumulative sum by the amount of elements summed
	}else if(strcmp(operationEntered,"range")==0){
		//Approach: copy the elements of the range in a list, then order that list, then choose the first and last element
		//Do different traversals depending on if the columns or the rows are equal
		if(paramCindex1 == paramCindex2){
			float orderArray[(findMax(paramRindex1,paramRindex2) - findMin(paramRindex1,paramRindex2)) + 1];
			for(i=findMin(paramRindex1,paramRindex2); i<= findMax(paramRindex1,paramRindex2); i++){
				modIndex+=1;
				orderArray[modIndex] = spreadSheet[i][paramCindex1].content.numeric;
			}
			for(i=0; i<(sizeof(orderArray)/sizeof(orderArray[0])); i++){
				//Order the values found
		        if(i != (sizeof(orderArray)/sizeof(orderArray[0]))){
					k=i+1;
					for(k;k<(sizeof(orderArray)/sizeof(orderArray[0]));k++){
						if(orderArray[i] > orderArray[k]){
						    swap = orderArray[i];
						    orderArray[i] = orderArray[k];
						    orderArray[k] = swap;
						}
					}
				}
			}
			result = (orderArray[(sizeof(orderArray)/sizeof(orderArray[0])) - 1] - orderArray[0]);
		}
		else if(paramRindex1 == paramRindex2){
			float orderArray[(findMax(paramCindex1,paramCindex2) - findMin(paramCindex1,paramCindex2)) + 1];
			for (i=findMin(paramCindex1,paramCindex2); i<= findMax(paramCindex1,paramCindex2); i++){
				modIndex+=1;
				orderArray[modIndex] = spreadSheet[paramRindex1][i].content.numeric;
			}
			for(i=0; i<(sizeof(orderArray)/sizeof(orderArray[0])); i++){
				//Order the values found
		        if(i != (sizeof(orderArray)/sizeof(orderArray[0]))){
					k=i+1;
					for(k;k<(sizeof(orderArray)/sizeof(orderArray[0]));k++){
						if(orderArray[i] > orderArray[k]){
						    swap = orderArray[i];
						    orderArray[i] = orderArray[k];
						    orderArray[k] = swap;
						}
					}
				}
			}
			
			result = (orderArray[(sizeof(orderArray)/sizeof(orderArray[0])) - 1] - orderArray[0]);
		}
	//Additional Functions(*)	
	}else if(strcmp(operationEntered,"product")==0){
		result = (float)spreadSheet[paramRindex1][paramCindex1].content.numeric * (float)spreadSheet[paramRindex2][paramCindex2].content.numeric;		
	}else if(strcmp(operationEntered,"power")==0){
		result = pow(spreadSheet[paramRindex1][paramCindex1].content.numeric, spreadSheet[paramRindex2][paramCindex2].content.numeric);
	}else if(strcmp(operationEntered,"quotient")==0){
		result = (float)spreadSheet[paramRindex1][paramCindex1].content.numeric/(float)spreadSheet[paramRindex2][paramCindex2].content.numeric;
	}

	return result;
}//end do Calculation

//Place Data 
void placeData(char *cellContent,int row,int column,workbook spreadSheet[][maxNumRows]){
	int i,j,counter=0;//counter variables
	float resultCalculated=0;//holds the value returned from the doCalculation function for placing
	char opHolder[2];//used to transfer each cell reference as a single string
	char operationEntered[10];//will store the result of the 'operationFound' function
	char operands[4];//stores the operands found in the cell content upon user input
	
	if(validateNumeric(cellContent)){
		spreadSheet[row][column].content.numeric = atoi(cellContent);
		
		//Initializing other sections of the cell
		strcpy(spreadSheet[row][column].content.formula.operation,"");
		strcpy(spreadSheet[row][column].content.formula.operandSet[0].operand,"");
		strcpy(spreadSheet[row][column].content.formula.operandSet[1].operand,"");
		strcpy(spreadSheet[row][column].content.text,"");
	
	}else if(validateString(cellContent)){
		strcpy(spreadSheet[row][column].content.text,cellContent);
	
		//Initializing other sections of the cell
		strcpy(spreadSheet[row][column].content.formula.operation,"");
		strcpy(spreadSheet[row][column].content.formula.operandSet[0].operand,"");
		strcpy(spreadSheet[row][column].content.formula.operandSet[1].operand,"");
		spreadSheet[row][column].content.numeric = 0;
	
	}else if(validateFormula(cellContent)){	
		//Find the operation associated
		strcpy(operationEntered, determineOperation(cellContent,determineFirstIndex(cellContent)));
		strcpy(operands, determineOperands(cellContent));
		resultCalculated = doCalculation(cellContent,spreadSheet);

		strcpy(spreadSheet[row][column].content.formula.operation,operationEntered);//copy the name of the function to the appropriate location
		spreadSheet[row][column].content.numeric = resultCalculated;//assign the result to the numeric section
		//Place operands in the appropriate location
		for(i = 0; i<strlen(operands); i+=2){
			if(i < strlen(operands)){
				opHolder[0] = operands[i];
				opHolder[1] = operands[i+1];
				strcpy(spreadSheet[row][column].content.formula.operandSet[counter].operand,opHolder);		
			}
			counter+=1;
		}
		
		//Initialize the other sections of the cell
		strcpy(spreadSheet[row][column].content.text,"");
	}
}//end place data

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

//View Formulas
void viewFormulas( workbook spreadSheet[][maxNumRows] ){
	int numRows = sizeof(spreadSheet[0])/sizeof(spreadSheet[0][0]);
	int i,j;
	int ascii = 65;
	
	//Display headers
	printf("\nSPREADSHEET VIEW (FORMULAS): \n\n");
	printf("\t");
	for (i=ascii; i<(ascii+numRows); i++){
		printf("%c\t",i);
	}
	printf("\n");
	
	for(i=0; i<numRows; i++){
		printf("%d",i+1); //displays row number (1-9)
		for(j=0; j<numRows; j++){		
			if(strlen(spreadSheet[i][j].content.formula.operation)>0){
				printf("\t=%s",spreadSheet[i][j].content.formula.operation);
				printf("(%s,%s)",spreadSheet[i][j].content.formula.operandSet[0].operand,spreadSheet[i][j].content.formula.operandSet[1].operand);
			}else{
				printf("\t");
			}		
		}
		printf("\n");
	}
}//end view formulas

//Upper Case
char upperCase(char value){
	int newValue;//used for calculating the uppercase integer representation of a character
	char valToChar;//used for converting the generated integer to a character value
	if((int)value>=97 && (int)value<=122){
		newValue = (int)value-32;
		valToChar = newValue;
	}else{
		valToChar = value;
	}
	return valToChar;
}//end uppercase

//Lower Case
char lowerCase(char value){
	int newValue;//used for calculating the lowercase integer representation of a character
	char valToChar;//used for converting the generated integer to a character value
	if((int)value>=65 && (int)value<=90){
		newValue = (int)value+32;
		valToChar = newValue;
	}else{
		valToChar = value;
	}
	return valToChar;
}//end lowercase


// save function
int saveWorksheet(worksheet sheet[][maxNumRows])
{
    int r;
    // open up a file
    FILE *fpt;
    fpt = fopen("spreadsheet.dat", "w");
    if(fpt == NULL)
        return 0;
    //variables
    int i, j;
    worksheet cell;
    char s[100] = "";
    char tf[50];
    // for every cell in the sheet
    for(i=0; i<9; i++)
    {
        for(j=0; j<9; j++)
        {
            // get the cell
            worksheet cell = sheet[i][j];
            
            // if cell text empty
            if(strlen(cell.content.text)==0)
            {
                // append special string
                strcat(s, "%%");
            }else
            {
                // append cell text + space
                strcat(s, cell.content.text);
                strcat(s, " ");
            }
            // append cell numeric as string
            sprintf(tf, "%f", cell.content.numeric);
            strcat(s, tf);
            strcat(s, " ");
            // generate and append formula string
            strcat(s, "=");
            strcat(s, cell.content.formula.operation);
            strcat(s, "(");
            strcat(s, cell.content.formula.operandSet[0].operand);
            strcat(s, ",");
            strcat(s, cell.content.formula.operandSet[1].operand);
            strcat(s, ")");
            
            // write cell representation to file
            r = fprintf(fpt, "%s\n");
        }
    }
    
    // close the file
    fclose(fpt);
    return r;
}

int readWorksheet(worksheet sheet[][maxNumRows]){
	int i,j,ch,count=0;
	worksheet cell;
	fpt = fopen("spreadsheet.dat","r");
// 	filename = "spreadsheet.dat";
	
	//Read from file
	//Get number of lines in file
	//Iterate using that number
	//Store the value associated with each index re: text/numeric/formula[operation[operand1,operand2]]
	//Place values found in spreadsheet
	do{
	   ch = fgetc(fp);
	   if( ch== '\n') 
	   	count++;   
	}while( ch != EOF );	
	
	FILE *fpt;
	char dataSpace[100];
	
	fpt=fopen(filename,"r");
		fread(dataSpace,1,100,fpt);
		text[100]=0;
		printf("%s\n",text);
		
		for(i=0; i<count; i++){
			//
		}
	fclose(fpt);
	 return 0;
}

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
    
    //========= spreadsheet related ================//
    int i,j; //loop counters	
	int rowIndex, columnIndex; //For Sum,range(#rows similar, #columns similar), element row, element column
	char intendedCell[2];//accepts the user's target cell
	char cellContent[17-1];//stores the text/formula/digit that will then be tested for type in order to then store it in the appropriate area in the cell
	int seeFormulas,endProgram;
	workbook sheet[9][maxNumRows];//stores each element of the spreadsheet
	//=============================================//
	
	// initialize sheet
	initializeSheet(sheet);
	showSheet(sheet);
    
    
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
                        char resp[BUF_SIZE];
                        //get cell to edit
                        strncpy(intendedCell, token[1], 2);
                        /*Calculate the array equivalent of the coordinates given. i.e [A,1] == [0,0]*/
                		columnIndex = (int)upperCase(intendedCell[0]) - 65;//gets the column index of the array
                		rowIndex = ((int)intendedCell[1] - 48) - 1;//gets the row index of the array
                		// get old data
                		if(strlen(sheet[rowIndex][columnIndex].content.text) > 0)
                			sprintf(resp, "(!) - Old Value: [%s]\nedit ok.",sheet[rowIndex][columnIndex].content.text);	
                		else
                			sprintf(resp, "(!) - Old Value: [%.2f]\nedit ok.",sheet[rowIndex][columnIndex].content.numeric);
                		//replace
                		placeData(token[2],rowIndex,columnIndex,sheet);
                		
                // 		showSheet(sheet);
                		saveWorksheet(sheet);
                		
                        //reset timout
                        alarm(LOCK_TOUT);
                        oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, resp);
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
                }else if(strcmp(token[0], "view_sheet") == 0)
                {
                    // printf("Got to view_sheet..\n");
                    char data[BUF_SIZE];
                    oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "sending_sheet");
                    sleep(2);
                    for(i=0; i<9; i++)
                    {
                        for(j=0; j<9; j++)
                        {
                            if(strlen(sheet[i][j].content.text) > 0)
                    			sprintf(data, "cell %d %d %s", i, j, sheet[i][j].content.text);
                    		else
                    			sprintf(data, "cell %d %d %.2f", i, j, sheet[i][j].content.numeric);
                    // 		printf("%s\n", data);
                			oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, data);
                        }
                    }
                    oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "done");
                }else if(strcmp(token[0], "save_sheet") == 0){
                    printf("Saving sheet..\n");
                    int res;
                    res = saveWorksheet(sheet);
                    if(res == 0)
                        oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "Save failed.");
                    else
                        oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "Spreadsheet saved.");
                }else if(strcmp(token[0], "load_sheet") == 0){
                    printf("Loading sheet..\n");
                    int res;
                    res = saveWorksheet(sheet);
                    if(res == 0)
                        oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "Load failed.");
                    else
                        oneshot_msg(cli_addr.sin_addr.s_addr, cli_addr.sin_port, "Spreadsheet loaded.");
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