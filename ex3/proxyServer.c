
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> /* strlen, strtok, strcat */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <arpa/inet.h> /* inet_addr, inet_ntop */
#include "threadpool.h"
#define SA   struct sockaddr
#define LISTENQ  1024  /* Second argument to listen() */
#define BUFLEN 32000 //make it smaller for testing

 typedef struct proxyServer_st{
    int port;
    int pool_size;
    int max_num_req;
    int listen_pr;
    FILE *filter;
    threadpool *tp;

}proxyServer;



void Usage_print(){
    printf("proxyServer <port> <pool-size> <max-number-of-request> <filter>\n");
    _Exit(EXIT_FAILURE);
}

void clienterror(int fd, char *errnum,  char *shortmsg, char *longmsg) {
    char buf[BUFLEN], body[BUFLEN];
 
    /* Build the HTTP response body */
    sprintf(body, "<html><title>%s %s</title>", errnum, shortmsg);
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s\r\n", body, longmsg);
 
    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r", errnum, shortmsg);
    printf(" %s\n",buf);
    sprintf(buf, "Server: webserver/1.0\r");
    printf(" %s\n",buf);
    sprintf(buf, "Content-type: text/html\r");
    printf(" %s\n",buf);
    sprintf(buf, "Content-Length: %ld\r",strlen(body));
    printf(" %s\n",buf);
    sprintf(buf, "Connection: close\r\n");
    printf(" %s\n",buf);
    write(fd, body, strlen(body));
}

//this function will set up the connection to the address given
int setup_connection(int socket_fd, char *host, int port, struct sockaddr_in *my_addr){

	//get the host information
	struct hostent *server = gethostbyname(host);
	if (!server)
	{
    clienterror(socket_fd, "404", " Not Found","File not found");
    close(socket_fd);
		return -1;
	}

	//the internet addressing data structure
	my_addr->sin_family = AF_INET; //TCP type
	my_addr->sin_addr.s_addr = ((struct in_addr*)(server->h_addr_list))->s_addr; //the ip

  if (my_addr->sin_addr.s_addr < 0) //host ip didn't found 
	{
		perror("Assigning the destination address to the socket failed");
		return -1;
	}
  my_addr->sin_port = htons(port); //hostbyte order for the port

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("Socket setup failed");
		return -1;
	}
if (connect(socket_fd, (struct sockaddr*)my_addr, sizeof(*my_addr)) < 0)
	{
		perror("Connection failed");
    close(socket_fd);
		return -1;
	}

	return 0;
}



void block_connection(int connfd,char  *hostAddr ){
    //here send a block and close the connection
    clienterror(connfd, "403", "Forbidden","Access denied");
close(connfd);
        return;
}
void make_connection(int connfd,char* hostAdrr,char*portNum,char*absolutePath){
  struct sockaddr_in *my_addr;//for get
  char *send_message;//the send message
  char  read_message[1024];//the read response message
  int read_approve;
  int port = atoi(portNum);
  char buf[BUFLEN];

int len = strlen(hostAdrr) + //the host length
	(absolutePath == NULL? 0 : strlen(absolutePath)) + //if there's a file path
	192; //for additional signs and commas


    	send_message = (char*)calloc(len, sizeof(char));
	if (!send_message)
	{
		free(send_message);
		exit(EXIT_FAILURE);
	}

    sprintf(send_message,"GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",absolutePath,hostAdrr);
	printf("HTTP request =\n%s\nLEN = %ld\n", send_message, strlen(send_message));

my_addr = (struct sockaddr_in*)calloc(1, sizeof(struct sockaddr_in));
	if (!my_addr){
     perror("error creat an address for accepring connecion\n");
     free(my_addr);
		_Exit(EXIT_FAILURE);
	}

int x=setup_connection(connfd,hostAdrr,port,my_addr);
if(x<0){return;}else{

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sHost: %s\r\n", buf,hostAdrr);
    sprintf(buf, "%sContent-length: %ld\r\n", buf, strlen(buf));
    write(connfd, buf, strlen(buf));
    printf("\n%s\n",buf);

	//receive the response

	int length = 0;
	while (1){

		bzero(read_message, sizeof(read_message));
		read_approve = read(connfd, read_message, sizeof(read_message));
		if (read_approve < 0) //failed receiving data
		{
			perror("Receiving response failed");
			exit(EXIT_FAILURE);
		}
						
		else if (read_approve > 0) //received data
		{
     
    
			printf("%s", read_message);
			length += read_approve;
      write(connfd, read_message, strlen(read_message));
		}
		else //read_approve = 0, no more to read
     //write(connfd, read_message, strlen(read_message));
			break;
	}
  
	printf("\n-Total received response bytes: %d\n", length);
  write(connfd, read_message, strlen(read_message));
	close(connfd);
return;
  }
}


//this function check the host if its filtered 
int connection_validation(char *hostAddr,FILE * tempfile, int connfd  ,char*portNum,char*absolutePath){
    char *line=NULL;
    size_t len = 0;
    ssize_t read;
   
    
        // Open file 
    tempfile = fopen("filter.txt", "r"); 
    if (tempfile == NULL) 
    { 
        printf("Cannot open file \n"); 
        exit(0); 
    } 
  
    // Read contents from file 
    
    while ((read = getline(&line, &len, tempfile)) != -1) {
        hostAddr [ strcspn(hostAddr, "\r\n") ] = '\0'; 
        line [ strcspn(line, "\r\n") ] = '\0'; 
         
        int checkValidation = strcmp(hostAddr,line);       
        if (checkValidation==0){
            block_connection(connfd,hostAddr);return 1;}
        else{
            continue;}
    }
    make_connection(connfd,hostAddr,portNum,absolutePath);
    fclose(tempfile); 
    return 1;
}

/*this function cut the first line */
int firstLine(char* request, char* returnString) {
  int firstLine_len;
  int i = 0;
  while(request[i] != '\r' && request[i+1] != '\n') { 
        returnString[i] = request[i];
        i++;
  }
  firstLine_len = i;
  printf("length of first line is: %d\n", firstLine_len);
  printf("first line: %s\n", returnString);
  return firstLine_len;
}


 // open_listenfd - open and return a listening socket on port

int open_listenfd(int port) {
    int listenfd;
    struct sockaddr_in serveraddr;
  
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	return -1;
 
    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
	return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 5) < 0)
	return -1;
    return listenfd;
}
/* $end open_listenfd */
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int rc;
    if ((rc = accept(s, addr, addrlen)) < 0)
	 return -1;
return rc;
}

int validation(char *read_message,FILE* tempfile , int connfd){ //this fuction check if the request is validate
  char   outbuf[BUFLEN];
  char* strptr;
  char* strptr2;
  char* strptr3;
  char requestType[4];
  char hostAddr[BUFLEN];
  char absolutePath[BUFLEN];
  char portNum[6];
  char temp[BUFLEN];
  char tempHandler[BUFLEN];
  char* save0;
  char* save1;
  char* save2;
  


    memset(portNum,0, sizeof(portNum));
    memset(hostAddr, 0, sizeof(hostAddr));
    memset(requestType,0, sizeof(requestType));
    memset(absolutePath,0, sizeof(absolutePath));
    memset(temp,0,sizeof(temp));
    memset(outbuf,0,sizeof(outbuf));
    memset(tempHandler,0,sizeof(tempHandler));


      if(   firstLine(read_message,temp)<=0){
          perror("error reading the first line of the response"); 
      }
    strncpy(tempHandler,temp,sizeof(temp));
  
    strptr = strtok_r(tempHandler, " ",&save0);
    strcpy(requestType, strptr);
    printf("request type: %s\n", requestType);

    /* Check if Method is a GET method */
    if(strcmp(requestType,"GET") != 0) {
      memset(outbuf,0,sizeof(outbuf));
      clienterror(connfd, "501", "Not supported","Method is not supported");
      close(connfd);
      return -1;
    }

    strptr = strtok_r(NULL, ":",&save0);
    strptr += 7;
    strptr2 = strtok_r(strptr, "/",&save1);
    strptr3 = strtok_r(NULL, " ",&save1);
    
    int length = strlen(strptr2);
    strptr = strtok_r(strptr2,":",&save2);

    if(strlen(strptr) == length) { // No port
      strncpy(hostAddr,strptr,strlen(strptr));
      strncpy(portNum,"80",2);
      if(strcmp(strptr3,"HTTP/1.1") == 0){
	*absolutePath = '/';
      } else {
	*absolutePath = '/';
	strptr = absolutePath + 1;
	strncpy(strptr, strptr3, strlen(strptr3));
      }
    } else {                        // has port
      strncpy(hostAddr,strptr,strlen(strptr));
      strptr = strtok_r(NULL, " ",&save2);
      strncpy(portNum,strptr,strlen(strptr));
      if(strcmp(strptr3,"HTTP/1.1") == 0){
	*absolutePath = '/';
      } else {
	*absolutePath = '/';
	strptr = absolutePath + 1;
	strncpy(strptr, strptr3, strlen(strptr3));
      }
    }
    memset(tempHandler, 0, sizeof(tempHandler));

    printf("host: %s\n", hostAddr);
    printf("port: %s\n", portNum);
    printf("path: %s\n\n", absolutePath);
/*this function now take the hostaddr , the file and the connfd*/
        connection_validation(hostAddr, tempfile,  connfd ,portNum,absolutePath);
return 0;
}

int doit(void * proxy){//this is the main function that do all the work
int  connfd; 
socklen_t clientlen;
struct sockaddr_in clientaddr;
char  read_message[1024];//the read response message
int read_approve , len=0;
FILE * tempfile ;//for file debug
proxyServer *tmp_proxy = (proxyServer*)proxy;
tempfile=tmp_proxy->filter;




for (int i =0;i< tmp_proxy->max_num_req;i++){

    clientlen = sizeof(clientaddr);
    
    connfd = Accept(tmp_proxy->listen_pr, (SA *)&clientaddr, &clientlen);
    if(connfd<0){perror("error in connfd "); _Exit(EXIT_FAILURE); }

    
		bzero(read_message, sizeof(read_message));
		read_approve = read(connfd, read_message, sizeof(read_message));
		if (read_approve < 0) //failed receiving data
		{
            clienterror(connfd, "400", "Bad Request","Bad Request");
            close(connfd);
			exit(EXIT_FAILURE);
		}
						
		else if (read_approve > 0) //received data
		{   validation(read_message,tempfile,connfd);//this function take GET ... and file to compare 
            len += read_approve;
		}
		else //read_approve = 0, no more to read
			break;
	
	printf("\nTotal received response bytes: %d\n", len);
 
    }   
    return 1;
}




int main(int argc , char * argv[]){


    if (argc < 5 || argc > 5){ //not correct usage
		Usage_print(); _Exit(EXIT_FAILURE);}

proxyServer* proxy = (proxyServer*) malloc(sizeof(proxyServer));
if(atoi(argv[1])>=1025 && atoi(argv[1]) <=65535){//port number
proxy->port = atoi(argv[1]);
}
    else{
    perror("invalid port "); Usage_print();
}
if(atoi(argv[2])>0){
    proxy->pool_size=atoi(argv[2]);
    }
    else{
   perror("invalid pool size "); Usage_print();
}
if(atoi(argv[3])>0){
    proxy->max_num_req=atoi(argv[3]);
    }
    else{
   perror("invalid max number of requests "); Usage_print();
}
proxy->filter=(FILE*)argv[4];
proxy->tp=create_threadpool(proxy->pool_size);
proxy->listen_pr = open_listenfd(proxy->port);

    for (int i =0;i< proxy->max_num_req;i++){ //i dont know why dispatch doesn't work here
               //it worked with the tester in threadpool.c
    dispatch(proxy->tp, doit, (void *) proxy);	
  if (doit(proxy)>=0){
    continue;}
    else{
        perror("error in the doit function : ");_Exit(EXIT_FAILURE);
    }
    close(proxy->listen_pr);
}
destroy_threadpool(proxy->tp);
free(proxy);
}