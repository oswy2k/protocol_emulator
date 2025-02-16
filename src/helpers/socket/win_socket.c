#include "win_socket.h"

#include <stdio.h>
/* Private Functions */
static void* read_thread_function(void *socket){

	socket_t *socket_ptr = (socket_t*) socket;

	queue_item_t item = {0};
	while(socket_ptr->status != SOCKET_STATUS_DISCONNECTED){
		if(queue_is_full(&(socket_ptr->queues.read)) == false){
			recv(socket_ptr->connection_socket,(char *) &item, 1024, 0);

			if(item.size == 0xffff){
				printf("Socket disconnected\n");
				socket_ptr->status = SOCKET_STATUS_DISCONNECTED;
				break;
			}

			queue_enqueue(&(socket_ptr->queues.read), &item);
		}
		else{
			Sleep(1);
		}
	}

	return NULL;
}

static void* write_thread_function(void *socket){

	socket_t *socket_ptr = (socket_t*) socket;

	queue_item_t item = {0};
	uint16_t count = 0;

	while(socket_ptr->status != SOCKET_STATUS_DISCONNECTED){
		queue_count(&(socket_ptr->queues.write), &count);
		if(count >0){
			queue_dequeue(&(socket_ptr->queues.write), &item);
			send(socket_ptr->connection_socket,(char *) &item, sizeof(item.size) + item.size, 0);
			Sleep(1);
		}
		else{
			Sleep(1);
		}
	}

	/* Socket close connection condition */
	item.size = 0xffff;
	item.buffer[0] = 0;
	send(socket_ptr->connection_socket,(char *) &item, sizeof(item.size), 0);

	return NULL;
}

/* Public Functions */
socket_error_t socket_init(socket_t *socket){

	int wsa_response = 0;			//Get Data from wsa response codes

	/* Guard in case the socket is already initialized */
	if(socket->status != SOCKET_STATUS_UNINITIALIZED){
		closesocket(socket->connection_socket);
		WSACleanup();
		return SOCKET_ERROR_ALREADY_INITIALIZED;
	}

	/* Initialize DLL for sockets*/
	wsa_response = WSAStartup(MAKEWORD(2, 0), &socket->socket_data);
	if(wsa_response){
		printf("WSA Error:%d\n", WSAGetLastError());
		WSACleanup();
		return SOCKET_ERROR_DLL_STARTUP;
	}

	return SOCKET_ERROR_NONE;
}

void socket_get_hostname(char* hostname){
	gethostname(hostname, 64);
}

socket_error_t socket_connect(socket_t *socket_ptr, const char *host, const int port){

	/* Get ip for our server */
	socket_ptr->host_ptr = gethostbyname(host);
	if(socket_ptr->host_ptr == NULL){
		printf("WSA Error:%d\n", WSAGetLastError());
		WSACleanup();
		return SOCKET_ERROR_HOST_NOT_FOUND;
	}

	/* Create socket with IPV4 and TCP*/
	socket_ptr->connection_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_ptr->connection_socket == INVALID_SOCKET){
		printf("WSA Error:%d\n", WSAGetLastError());
		WSACleanup();
		return SOCKET_ERROR_INVALID_CONFIGURATION;
	}

	/* Set up socket address */
	memset(&socket_ptr->socket_addr, 0x00, sizeof(SOCKADDR_IN));
	socket_ptr->socket_addr.sin_addr.S_un.S_addr = (u_long ) inet_addr("127.0.0.1");
	socket_ptr->socket_addr.sin_family = socket_ptr->host_ptr->h_addrtype;
	socket_ptr->socket_addr.sin_port = htons(port);

	/* Connect to server */
	if(connect(socket_ptr->connection_socket, (SOCKADDR*)&socket_ptr->socket_addr, sizeof(SOCKADDR_IN))==SOCKET_ERROR){
		closesocket(socket_ptr->connection_socket);
		printf("WSA Error:%d\n", WSAGetLastError());
		WSACleanup();
		return SOCKET_ERROR_CONNECTION_FAILED;
	}
	else{
		socket_ptr->status = SOCKET_STATUS_CONNECTED;
	}

	/* Initialize read and write queues */
	queue_init(&socket_ptr->queues.read);
	queue_init(&socket_ptr->queues.write);

	/* Set up threads for reading and writing */
	pthread_create(&socket_ptr->threads.read, NULL, read_thread_function, (void*) socket_ptr);
	pthread_create(&socket_ptr->threads.write, NULL, write_thread_function, (void*) socket_ptr);

	return SOCKET_ERROR_NONE;
}

socket_error_t socket_disconnect(socket_t *socket){

	/* Notify to threads that they must close */
	socket->status = SOCKET_STATUS_DISCONNECTED;

	/* Sleep to give time external socket to close */
	Sleep(1000);

	/* Close socket connection*/
	closesocket(socket->connection_socket);

	/* Close threads */
	pthread_join(socket->threads.read, NULL);
	pthread_join(socket->threads.write, NULL);

	/* Clean up */
	WSACleanup();
}

socket_status_t socket_is_connected(socket_t *socket){
	if(socket->status == SOCKET_STATUS_CONNECTED){
		return SOCKET_STATUS_CONNECTED;
	}
	else{
		return SOCKET_STATUS_DISCONNECTED;
	}
}

socket_error_t socket_peek(socket_t *socket, queue_item_t *item){

	if(queue_is_empty(&(socket->queues.read)) == true){
		return SOCKET_ERROR_NO_DATA;
	}

	/* Peek item */
	queue_peek(&(socket->queues.read), item);

	return SOCKET_ERROR_NONE;
}

socket_error_t socket_delete(socket_t *socket){

	if(queue_is_empty(&(socket->queues.read)) == true){
		return SOCKET_ERROR_NO_DATA;
	}

	/* Dequeue item */
	queue_dequeue(&(socket->queues.read), NULL);

	return SOCKET_ERROR_NONE;
}


socket_error_t socket_read(socket_t *socket, queue_item_t *item, socket_read_t block){

	switch(block){
		case SOCKET_BLOCK:
			while(queue_is_empty(&(socket->queues.read)) == true && socket->status != SOCKET_STATUS_DISCONNECTED){
				Sleep(10);
			}
			break;
		case SOCKET_NONBLOCK:
			if(queue_is_empty(&(socket->queues.read)) == true && socket->status != SOCKET_STATUS_DISCONNECTED){
				return SOCKET_ERROR_NO_DATA;
			}
			break;
	}

	/* Dequeue item */
	queue_dequeue(&(socket->queues.read), item);

	return SOCKET_ERROR_NONE;
}

socket_error_t socket_write(socket_t *socket, queue_item_t *item, socket_read_t block){

	switch(block){
		case SOCKET_BLOCK:
			while(queue_is_full(&(socket->queues.write)) == true && socket->status != SOCKET_STATUS_DISCONNECTED){
				Sleep(10);
			}
			break;
		case SOCKET_NONBLOCK:
			if(queue_is_full(&(socket->queues.write)) == true && socket->status != SOCKET_STATUS_DISCONNECTED){
				return SOCKET_ERROR_QUEUE_FULL;
			}
			break;
	}

	/* Enqueue item */
	queue_enqueue(&(socket->queues.write), item);

	return SOCKET_ERROR_NONE;

}