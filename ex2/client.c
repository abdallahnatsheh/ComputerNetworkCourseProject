#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include  <ctype.h>
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <unistd.h>
#define SA   struct sockaddr
typedef struct url_parser_url {//this is a struct for the url
	char *protocol;
	char *host;
	int port;
	char *path;
  char *query_string;
	int host_exists;
	char *host_ip;
} url_parser_url_t;

void free_parsed_url(url_parser_url_t *url_parsed) {//this a private function to parse empty the url
	if (url_parsed->protocol)
		free(url_parsed->protocol);
	if (url_parsed->host)
		free(url_parsed->host);
	if (url_parsed->path)
		free(url_parsed->path);


	free(url_parsed);
}
int parse_url(char *url, bool verify_host, url_parser_url_t *parsed_url) {//this private function to parse the url
	char *local_url = (char *) malloc(sizeof(char) * (strlen(url) + 1));
	char *token;
	char *token_host;
	char *host_port;
	//char *host_ip;

	char *token_ptr;
	char *host_token_ptr;

	char *path = NULL;

	// Copy our string
if (strcmp(local_url,url)==0){
    fprintf(stderr,"The request is same as before\n");
    _Exit(EXIT_SUCCESS);}
    else{
strcpy(local_url, url);
	token = strtok_r(local_url, ":", &token_ptr);
  if(strcmp(token,"http")==0){
	parsed_url->protocol = (char *) malloc(sizeof(char) * strlen(token) + 1);
	strcpy(parsed_url->protocol, token);}else//here the protocol like http
  {
    perror("http only");_Exit(EXIT_FAILURE);
  }
  

	// Host:Port
	token = strtok_r(NULL, "/", &token_ptr);//here the host like google.com
	if (token) {
		host_port = (char *) malloc(sizeof(char) * (strlen(token) + 1));
		strcpy(host_port, token);
	} else {
		host_port = (char *) malloc(sizeof(char) * 1);
		strcpy(host_port, "");
	}

	token_host = strtok_r(host_port, ":", &host_token_ptr);//inside token_host is google.com
	parsed_url->host_ip = NULL;
	if (token_host) {
		parsed_url->host = (char *) malloc(sizeof(char) * strlen(token_host) + 1);
		strcpy(parsed_url->host, token_host);//copy google.com to parsed_url->host

		if (verify_host) {
			struct hostent *host;
			host = gethostbyname(parsed_url->host);
			if (host != NULL) {
				parsed_url->host_ip = inet_ntoa(* (struct in_addr *) host->h_addr_list);
				parsed_url->host_exists = 1;
			} else {
				parsed_url->host_exists = 0;
			}
		} else {
			parsed_url->host_exists = -1;
		}
	} else {
		parsed_url->host_exists = -1;
		parsed_url->host = NULL;
	}

	// Port
	token_host = strtok_r(NULL, ":", &host_token_ptr);
	if (token_host)
		parsed_url->port = atoi(token_host);
	else
		parsed_url->port = 80;

	token_host = strtok_r(NULL, ":", &host_token_ptr);
	assert(token_host == NULL);

	token = strtok_r(NULL, "?", &token_ptr);
	parsed_url->path = NULL;
	if (token) {
		path = (char *) realloc(path, sizeof(char) * (strlen(token) + 2));
		strcpy(path, "/");
		strcat(path, token);

		parsed_url->path = (char *) malloc(sizeof(char) * strlen(path) + 1);
		strncpy(parsed_url->path, path, strlen(path));

		free(path);
	} else {
		parsed_url->path = (char *) malloc(sizeof(char) * 2);
		strcpy(parsed_url->path, "/");
	}

	token = strtok_r(NULL, "?", &token_ptr);
	if (token) {
		parsed_url->query_string = (char *) malloc(sizeof(char) * (strlen(token) + 1));
		strncpy(parsed_url->query_string, token, strlen(token));
	} else {
		parsed_url->query_string = NULL;
	}

	token = strtok_r(NULL, "?", &token_ptr);
	assert(token == NULL);

	free(local_url);
	free(host_port);
	return 0;
	}
}

int namevalue_only( char *word){// I DONT KNOW WHAT IS THIS YET =======================
if (word==0x0)
{
	return -1;
}

	if(strchr(word,'=')!=NULL)
		return 1;
	return -1;
}

//this function will set up the connection to the address given
int setup_connection(int *socket_fd, char *host, int port, struct sockaddr_in *my_addr)
{
	//setting up the TCP socket
	if ((*socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Socket setup failed");
		return -1;
	}
	
	//get the host information
	struct hostent *server = gethostbyname(host);
	if (!server)
	{
		perror("Unable to resolve host");
		return -1;
	}
	
	//the internet addressing data structure
	my_addr->sin_family = AF_INET; //TCP type
	my_addr->sin_addr.s_addr = ((struct in_addr*)(server->h_addr))->s_addr; //the ip
	
	if (my_addr->sin_addr.s_addr < 0) //host ip didn't found 
	{
		perror("Assigning the destination address to the socket failed");
		return -1;
	}
	
	my_addr->sin_port = htons(port); //hostbyte order for the port
	//connect to the host requested
	if (connect(*socket_fd, (struct sockaddr*)my_addr, sizeof(*my_addr)) < 0)
	{
		perror("Connection failed");
		return -1;
	}
	
	return 0;
}
int legal_text(char *word, int i)// I DONT KNOW THIS YET ====================
{
	while(i < strlen(word))
	{
		if ((int)word[i] < 48 || (int)word[i] > 32)
			return -1;
		i++;
	}
	return 0;
}

int process_http(int sockfd, char *host, char *page, char *poststr)
{
	char sendline[4096 + 1], recvline[4096 + 1];
	int n;
	snprintf(sendline, 200,
		 "POST %s HTTP/1.0\r\n"
		 "Host: %s\r\n"
		 "Content-type: application/x-www-form-urlencoded\r\n"
		 "Content-length: %ld\r\n\r\n"
		 "%s\n", page, host, strlen(poststr), poststr);

	write(sockfd, sendline, strlen(sendline));
	while ((n = read(sockfd, recvline, 4096)) > 0) {
		recvline[n] = '\0';
		printf("%s", recvline);
	}
	printf("\n");
	return n;

}
int main(int argc, char **argv) {
  bool usedp=false;//the flag for argument -p
  bool usedr=false;//the flag for argument -r
  bool usedurl=false;//;//the flag for argument <URL>
  char temp_url[128] = { 0 }; // its for the arguments inside -r
  char temp_text[128] = { 0 };//its for the arguments inside -p
  int error; //to check if theres any error in a fuction
  url_parser_url_t *parsed_url;
  char  read_message[1024];//the read response message
  char *send_message;//the send message
  struct sockaddr_in *my_addr;//for get
  struct sockaddr_in servaddr;//for post
  int socket_fd=0,read_approve=0;

	if (argc < 2) {	
		fprintf(stderr, "client [–p <text>] [–r n <pr1=value1 pr2=value2 ...>] <URL>.\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		     if(!strncmp(argv[i],"http",4)&&usedurl==false){
		  parsed_url = (url_parser_url_t *) malloc(sizeof(url_parser_url_t));
      usedurl=true;
		  error = parse_url(argv[i], true	, parsed_url);
		if (error != 0) {
			fprintf(stderr, "Invalid URL \"%s\".\n", argv[i]);
			continue;
		}
		//set up the TCP connection
	my_addr = (struct sockaddr_in*)calloc(1, sizeof(struct sockaddr_in));
	if (!my_addr)
	{
		free_parsed_url(parsed_url);
		exit(EXIT_FAILURE);
	}
	
	int len = strlen(parsed_url->host) + //the host length
	(parsed_url->path == NULL? 0 : strlen(parsed_url->path)) + //if there's a file path
	192; //for additional signs and commas
		send_message = (char*)calloc(len, sizeof(char));
	if (!send_message)
	{
		free(send_message);
		exit(EXIT_FAILURE);
	}
     }
     if (!strcmp(argv[i], "-r")&&usedr==false){
      
		int  n=atoi(argv[i+1]);//when theres no number or a string this function use it as zero so its a future its not a bug ;)
	//	printf("for debug purposr only !!!! : n : %d",n);
		strcpy(temp_url,"?");
			
		int check_count =0;	
		for(int j = 1;j<=n;j++){//im must check if the argument equalt to n and versversa
		//printf("this is before namevalue !!!!");
		
		if(namevalue_only(argv[i+2+n])==1){
			fprintf(stderr,"to many parameters\n");_Exit(EXIT_FAILURE);
		}else{
		error=namevalue_only(argv[i+1+j]);
		if (error==1){
			strcat(temp_url,argv[i+1+j]);//before this i must check if it name==value or not DONE
			strcat(temp_url, "&");
		
			check_count ++;}
			else{
				fprintf(stderr,"the flag -r accept just name=value or theres too few parameters\n");
				_Exit(EXIT_FAILURE);
			}
		}	
	}
	

	  char *p = temp_url;//here i just deleted the last char becaue its & 
	  p[strlen(p)-1] = 0;
       usedr=true;
    }
      if (!strcmp(argv[i], "-p")&&usedp==false){
       /*a function take the text and return the post request */
       
	   if(!strcmp(argv[i+1],"-r")){
		   fprintf(stderr,"-r will be considered as the text of -p flag and its number will yield the error.\n");_Exit(EXIT_FAILURE);}
		   else{
			   strcpy(temp_text,argv[i+1]);
		   }
        usedp=true;
     }
  }   
	
  
  if (usedp ==false){
    sprintf(send_message,"GET %s%s HTTP/1.0\r\nHost: %s\r\n\r\n",parsed_url->path,temp_url,parsed_url->host);
	
	printf("HTTP request =\n%s\nLEN = %ld\n", send_message, strlen(send_message));
	
	//send the request
	setup_connection(&socket_fd,parsed_url->host,parsed_url->port,my_addr);
	if (write(socket_fd, send_message, strlen(send_message)) < 0){
		perror("Send request failed");
		free_parsed_url(parsed_url);
		exit(EXIT_FAILURE);
	}
		
	//receive the response

	
	int len = 0;
	while (1){
		bzero(read_message, sizeof(read_message));
		read_approve = read(socket_fd, read_message, sizeof(read_message));//THE BUG IS HERE!!
		if (read_approve < 0) //failed receiving data
		{
			perror("Receiving response failed");
			free_parsed_url(parsed_url);
			exit(EXIT_FAILURE);
		}
						
		else if (read_approve > 0) //received data
		{
			printf("%s", read_message);
			len += read_approve;
		}
		else //read_approve = 0, no more to read
			break;
	}
	printf("\nTotal received response bytes: %d\n", len);
	
	//deallocating resources
	free_parsed_url(parsed_url);
	return EXIT_SUCCESS;}
	//=====================================end of get ========================================================
	else{
		char **pptr;
		
		sprintf(send_message,"POST %s%s HTTP/1.0\r\nHost: %s\r\n\rContent-length:%ld\n\n",parsed_url->path,temp_url,parsed_url->host,strlen(temp_text));
		
	
	printf("HTTP request =\n%s\n", send_message);
	printf("%s\n",temp_text);
char str[50];
	struct hostent *hptr;
	if ((hptr = gethostbyname(parsed_url->host)) == NULL) {
		fprintf(stderr, " gethostbyname error for host: %s: %s",
			parsed_url->host, hstrerror(h_errno));
		exit(1);
	}
	printf("hostname: %s\n", hptr->h_name);
	if (hptr->h_addrtype == AF_INET
	    && (pptr = hptr->h_addr_list) != NULL) {
		printf("address: %s\n",
		       inet_ntop(hptr->h_addrtype, *pptr, str,
				 sizeof(str)));
	} else {
		fprintf(stderr, "Error call inet_ntop \n");
	}

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(80);
	inet_pton(AF_INET, str, &servaddr.sin_addr);

	connect(socket_fd, (SA *) & servaddr, sizeof(servaddr));
	process_http(socket_fd, parsed_url->host, parsed_url->path, temp_text);
	close(socket_fd);
	}
		free_parsed_url(parsed_url);
	

}
