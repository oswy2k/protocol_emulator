#include "usart.h"
#include "stdio.h"

/* Define global structure */
usart_interface_t usart_interface = {0};

/* Public functions */
/* USART interface functions */
void usart_init(usart_interface_mode mode,socket_t* socket){
	/* Check if the usart_interface interface is already initialized */
	if(usart_interface.interface_state == USART_INITIALIZED){
		return;
	}

	usart_interface.interface_state = USART_INITIALIZED;

	/* Set mode */
	usart_interface.mode = mode;

	/* Start callback as null */
	usart_interface.asynchronous.read_callback = NULL;

	/* Set socket */
	usart_interface.socket = socket;
}

uint8_t usart_get_mode(){
	return usart_interface.mode;
}

/* Static function for writing to socket */
static void usart_write_socket(uint16_t size, uint8_t* data){
	queue_item_t item = {
		.size = size + sizeof(uint8_t) + sizeof(uint8_t)
	};

	socket_error_t error;

	item.buffer[0] = USART;
	item.buffer[1] = USART_WRITE;

	memcpy(&item.buffer[2], data, size);

	/* Write item based on usart mode */
	socket_read_t socket_op;
	switch(usart_get_mode())
	{
		case USART_SYNCHRONOUS:
			socket_op = SOCKET_BLOCK;
			break;
		case USART_ASYNCHRONOUS:
			socket_op = SOCKET_NONBLOCK;
			break;
	}

	error = socket_write(usart_interface.socket, &item, socket_op);
	if(error != SOCKET_ERROR_NONE){
		printf("Error writing USART\n");
	}
}

/* this case is only sync, becaus async is handled by interrupt */
static void usart_read_socket(uint16_t*size, uint8_t* data){
	queue_item_t item = {0};
	socket_error_t error;

	if(usart_get_mode() != USART_SYNCHRONOUS){
		printf("Invalid mode for read while asynchronous\n");
		return;
	}

	/* Read item */
	error = socket_read(usart_interface.socket, &item, SOCKET_BLOCK);
	if(error != SOCKET_ERROR_NONE){
		printf("Error writing USART\n");
	}

	/* Copy item to buffer */
	memcpy(data, &item.buffer, item.size);
	*size = item.size;
}

/* USART interface functions */
void usart_read(uint16_t* size, uint8_t* data){
	usart_read_socket(size, data);
}

void usart_write(uint16_t size, uint8_t* data){
	usart_write_socket(size, data);
}

/* USART Asynchronous interface functions */
void usart_set_read_callback(usart_read_callback_function callback){
	if(usart_interface.mode != USART_ASYNCHRONOUS){
		printf("Invalid mode for callback\n");
		return;
	}

	usart_interface.asynchronous.read_callback = callback;
}
void usart_delete_callback(){
	if(usart_interface.mode != USART_ASYNCHRONOUS){
		printf("Invalid mode for callback\n");
		return;
	}

	usart_interface.asynchronous.read_callback = NULL;
}

void usart_handle_callback(uint16_t* size, uint8_t* data){
	if(usart_interface.asynchronous.read_callback != NULL){
		usart_interface.asynchronous.read_callback(size, data);
	}
}