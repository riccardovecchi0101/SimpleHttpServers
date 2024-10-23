########################### SIMPLE HTTP SERVER #################################################################################################################################
#########################+################################################################################################################
##################################################################################################################################################################

__author__='Riccardo Vecchi'

import os 
import socket
import signal

'''parametri configurazione server HTTP'''

MIMETYPES=['text/html', 'image/jpeg']

SERVER_HOST = 'localhost'
SERVER_PORT = 8080
ROOT_DIRECTORY = os.getcwd()

socket_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #AF_INET indica che utilizziamo socket ipv4
socket_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
socket_server.bind((SERVER_HOST,SERVER_PORT)) 					  #.bind() prende una tupla come parametro
socket_server.listen(1) 										  # metto il server in ascolto



###### DEPRECATED FUNCTIONS ###################################################################################################################


'''
def send_dummy_response(client_connection, client_address):
	##manda una risposta banale al client, manda semplicemente un hello world##
	print('client {} asked {}'.format(client_address, request))

	response = 'HTTP/1.1 200 OK\r\n' \
			   'Content-Type: text/html\r\n' \
			   'Connection: keep-alive\r\n' \
			   '\r\n<html><body><h1>Hello World</h1></body></html>\r\n'

	client_connection.sendall(response.encode())	
'''
'''
def send_requested_url(request, client_connection, client_address):
	#manda al client quello che ha chiesto
	url = parse_url(request)
	response = 'HTTP/1.1 200 OK\r\n'\
			   'Content-Typet: text/html\r\n'\
			   'Connection: keep-alive\r\n'\
			   f'\r\n<html><body><h1>you requested {url} </h1></body></html>\r\n'

	client_connection.sendall(response.encode())
'''
##################################################################################################################################################

def show_request(request, client_address):
	print('request {} from {}'.format(request, client_address))


def serve_page(request, client_connection, client_address):
	url = parse_url(request)

	if url == 'none':
		url = 'index.html'#se viene richiesta la root directory reindirizzo a index.htmò
	path = ROOT_DIRECTORY + '/' + url
	if(os.path.exists(path)): #controllo se esiste la pagina richiesta
		serve_correct_request(client_connection, path)
	else:
		serve_error_request(client_connection)


def serve_correct_request(client_connection, path):
	'''serve la pagina richiesta'''
	ext = path.partition('.')[2]

	for i in MIMETYPES:
		if ext in i:
			mime = i
			break

	with open(path, 'rb') as file:
		binary_data = file.read()

	response_headers = 'HTTP/1.1 200 OK\r\n'\
                       f'Content-Type: {mime}\r\n'\
                       'Connection: keep-alive\r\n'\
                       'Server: localhost\r\n'\
                       f'Content-Length: {len(binary_data)}\r\n'\
                       '\r\n'			

	response = response_headers.encode() + binary_data + '\n\r'.encode()
	send_response(response, client_connection)
	


def serve_error_request(client_connection):
	'''manda un error 404, page not found'''
	response = 'HTTP/1.1 400 BAD REQUEST\r\n'\
			   'Content-Type: text/html\r\n'\
			   'Connection: keep-alive\r\n'\
			   'Content-Lenght: 27\r\n'\
			   'Server: localhost\r\n'\
			   '\r\n<html><center><h1> ERROR 404: Page not found </h1></center></html>\r\n'

	send_response(response.encode(), client_connection)


def send_response(response, client_connection):
	'''risponde al client con una risposta già binaria'''
	client_connection.sendall(response)


def parse_url(request):
	#isola l'url
	url = request.partition('\n')[0]
	url = url.split()[1]
	url = url[1:]
	if url == "":
		url = "none"
	return url

if __name__ == '__main__':

###############################################################################################################################################################
################ CREAZIONE DEL SOCKET################################# (il socket ci serve per fare il binding porta , ip)

	print('LISTENING ON PORT {}'.format(SERVER_PORT))

	while True:

		client_connection, client_address = socket_server.accept()    #il server accetta una connessione (ritorna una tupla), connection è il nuovo oggetto della classe 
																	  #socket utilizzabile
		request = client_connection.recv(1024).decode('utf-8')		  #ricevo richiesta dal client

		#send_dummy_response(client_connection, request, client_address	
		#show_request(request, client_address)
		show_request(request,client_address)
		#send_requested_url(request,client_connection, client_address)
		processid = os.fork()

		if processid == 0: ##mi assicuro che sia il figlio a servire
			serve_page(request,client_connection,client_address)																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																												
			os.kill(os.getpid(), signal.SIGTERM)
		
		client_connection.close()			  


	socket_server.close() 
