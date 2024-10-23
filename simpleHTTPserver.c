/************************************************************ SIMPLE HTTP SERVER *******************************************************************************/
/***************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************/
/**************************************************************@author:Riccardo Vecchi**************************************************************************/
/***************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*********GLOBALI E STRUTTURE*******************************/
#define PORT 8080
#define ROOT "//home//ric//Scrivania//RetiDiCalcolatori//C"
#define MIMENUM 2
typedef enum state {FALSE, TRUE} state;
/*********************************************************/

/** PROTOTIPI ************************************************/
char* get_url(const char* request);
void  send_dummy_response(int socket_fd, const char* request);
void  send_response(int socket_fd, const char* request);
void  send_right_response(int socket_fd, FILE* f, char* mime);
void  send_error(int socket_fd);
void  send_(int socket_fd, unsigned char* response, size_t lenght);
/************************************************************/



/*** FUNZIONI  ****************************************************************************************************************************/

void print_request(const char* request){
	printf("Client requested: %s", request);
}


void send_response(int socket_fd, const char* request){

	char* url = get_url(request);
	char  mimetype[30];
	char* MIMETYPES[] = {"text/html", "image/jpeg"};
	char requested_page[1024];
	strcpy(requested_page, ROOT);
	strcat(requested_page, "//");
	strcat(requested_page, url);

	FILE* requested_file = fopen(requested_page, "rb");

	if(requested_file == NULL){
		send_error(socket_fd);
		return;
	}
	//ciao

	if(strstr(url, "html"))
		strcpy(mimetype, "text/html");
	else
		strcpy(mimetype, "image/jpeg");

	printf("\n\nREQUESTED FILE: %s\n\n", requested_page);
	send_right_response(socket_fd, requested_file, mimetype);

}



void send_error(int socket_fd){

	char* headers = "HTTP/1.1 404 BAD REQUEST \r\n"
					"Content-type: text/html\r\n"
					"Server: localhost"
					"Connection: keep-alive\r\n";
	char page[1024];
	char response[1024];

	int s_flag = sprintf(page, "\r\n<html><center><h1> ERROR 404: PAGE NOT FOUND </h1></center></html>\r\n");
	if(s_flag < 0)
	{
		perror("error formatting response");
		exit(EXIT_FAILURE);
	}

	strcpy(response,headers);
	strcat(response,page);

	send_(socket_fd, response, strlen(response));
}



void send_right_response(int socket_fd, FILE* requested_file, char* mime){

	printf("%s MIME IS", mime);

	unsigned char* headers = "HTTP/1.1 200 OK \r\n"
			"Server: localhost\r\n"
			"Connection: keep-alive\r\n";

	unsigned char page[200000];
	unsigned char content_type[1024];
	unsigned char response[200000];
	long size;

	bzero(page, 200000);
	bzero(response,200000);
	bzero(content_type, 1024);

	sprintf(content_type,"Content-type:%s\r\n\r\n", mime);

	fseek(requested_file, 0L, SEEK_END);
	size = ftell(requested_file);
	rewind(requested_file);
	printf("\n\n file size is: %ld \n\n", size);


	//printf("\n\nFASE 1\n\n");


	size_t Content_type_lenght = strlen(content_type);
	size_t header_lenght = strlen(headers);
	size_t total_lenght = Content_type_lenght + header_lenght + size + 2;


	//printf("\n\nFASE 1.5\n\n");

	if((fread(page, 1, size*sizeof(unsigned char), requested_file)) != (size_t)size * sizeof(unsigned char)){
		perror("ERROR IN READING");
		exit(EXIT_FAILURE);
	}

	//printf("\n\nFASE 2\n\n");
	page[size] = '\r';
    page[size + 1] = '\n';

	memcpy(response, headers, strlen(headers));
	memcpy(response + strlen(response), content_type, strlen(content_type));
	memcpy(response + strlen(response), page, size + 2);
	

//	printf("%s", response);

	send_(socket_fd, response, 200000);
	//printf("\n\nFASE 3\n\n");

}



void send_(int socket_fd, unsigned char* response, size_t size){
	if((send(socket_fd, response, size, 0)) <0)
	{
		perror("error sending");
		exit(EXIT_FAILURE);
	}
}



/*
void send_dummy_response(int socket_fd, const char* request){
	// manda una risposta banale //

	char* url = get_url(request);

	char* headers = "HTTP/1.1 200 OK \r\n"
					"Content-type: text/html\r\n"
					"Server: localhost"
					"Connection: keep-alive\r\n";
					
	char page[1024];
	char response[1024];

	int s_flag = sprintf(page, "\r\n<html><center><h1> you requested %s </h1></center></html>\r\n",url);
	if(s_flag < 0)
	{
		perror("error formatting response");
		exit(EXIT_FAILURE);
	}

	strcpy(response,headers);
	strcat(response,page);

	if((send(socket_fd, response, strlen(response), 0)) <0)
	{
		perror("error sending");
		exit(EXIT_FAILURE);
	}

} */

char* get_url(const char* request){
	//estrae l'url dalla richiesta

	char* url = malloc(1);
	size_t url_size = 0;

	/** ISOLO LA PRIMA RIGA DELLA RICHIESTA **/
	for(size_t i = 0; i < strlen(request); i++){
		if(request[i] != '\r'){
			url[url_size] = request[i];
			url_size++;
		}

		else{
			url[url_size] = 0;
			break;
		}
	}


	/** ESTRAGGO L'URL **/
	for(size_t i = 0, j = 0; i < url_size; i++){
		if(url[i] == '/'){
			for(size_t con = i + 1; url[con] != ' '; con++){
				url[j++] = url[con];
			}
			url[j] = 0;
			break;
		}
	}

	if(!strcmp(url,""))
		return "index.html";

	return url;

}


/***************************************************************************************************************************************************/




int main(int argc, char** argv){


	/*** CREAZIONE DEL SOCKET ****/
	int server_fd, new_socket;
	int opt = 1;
	struct sockaddr_in address;
	socklen_t address_len = sizeof(address);
	char* message = "Hello world! I'm the server"; //messaggio da inviare
	char buffer[1024]; //messaggio da ricevere 
	int valread;


	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  // AF_INET ---> ipv4,  SOCK_STREAM --> tcp
	{
		perror("failure in creating socket");
		exit(EXIT_FAILURE);
	}

	/*** FUNZIONE NECESSARIA PER FARE IN MODO CHE SI CHIUDA IL BINDING PORTA INDIRIZZO ***/
	if(setsockopt(server_fd, SOL_SOCKET,
					SO_REUSEADDR | SO_REUSEPORT, &opt,
					sizeof(opt))) {
	perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	
	/*** PARAMETRI DA SETTARE  ALLA STRUTTURA IN MODO CHE IL SOCKET FACCIA IL BINDING INDIRIZZO PORTA ***/
	address.sin_family = AF_INET;
	//address.sin_addr.s_addr = INADDR_ANY; //qualsiasi indirizzo (in caso di server web)
	address.sin_addr.s_addr = inet_addr("127.0.0.1");// ci interessa solo localhost;
	address.sin_port = htons(PORT); 

	/*** FUNZIONE DI BINDING ***/
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	/**** SERVIZIO **************************************************************************************/

	printf("\n\nlistening on port: %d\n\n", PORT);

	if (listen(server_fd, 3) < 0){//3 è il valore di backlog, ovvero il numero di richieste contemporanee che possono stare in coda
		perror("listening failed");
		exit(EXIT_FAILURE);
	}

	/** prende il file descriptor del socket e crea un connected soket e ritorna un nuovo file descriptor **/

	while(TRUE){

		if((new_socket
			 = accept(server_fd, (struct sockaddr*)&address,
			 		  &address_len))
			< 0) {
			perror("accept error");
			exit(EXIT_FAILURE);
		}

		bzero(buffer,1024);


		if((valread = read(new_socket, buffer, 1024 - 1)) < 0) //** legge dal file descriptor new socket salva nel buffer 1023 byte
		{
			perror("reading failed");
			exit(EXIT_FAILURE);
		} 

		print_request(buffer);

		send_response(new_socket,buffer);

		/** chiusura dei socket **/ 
		close(new_socket);
	}

	close(server_fd);
	
	return 0;
}