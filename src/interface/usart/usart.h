#ifndef USART_H
#define USART_H

#include <stdint.h>
#include <stdbool.h>

#include "../../helpers/socket/win_socket.h"

#define USART 1
#define USART_READ_BUFFER_SIZE 256

/* Defines the state of the inteface for handling gpio */
typedef enum _usart_interface_state{
	USART_UNINITIALIZED,
	USART_INITIALIZED,
	USART_ERROR
} usart_interface_state;

typedef enum _usart_interface_mode{
	USART_SYNCHRONOUS,
	USART_ASYNCHRONOUS
} usart_interface_mode;

typedef enum _usart_comm_mode_enum{
	USART_READ =0,
	USART_WRITE
} usart_comm_mode_enum;

/* Typedef callbacks */
typedef void (*usart_read_callback_function)(uint16_t* size, uint8_t* data);

typedef struct _usart_interface_t{
	socket_t* socket;

	usart_interface_mode mode;
	usart_interface_state interface_state;

	struct{
		uint8_t read_buffer[USART_READ_BUFFER_SIZE];
		uint16_t last_read_buffer_size;
	}syncrhonous;
	
	struct{
		usart_read_callback_function read_callback;
	}asynchronous;

} usart_interface_t;

/* Public functions */
void usart_init(usart_interface_mode mode,socket_t* socket);
uint8_t usart_get_mode();


/* USART interface functions */
void usart_read(uint16_t* size, uint8_t* data);
void usart_write(uint16_t size, uint8_t* data);

/* USART Asynchronous interface functions */
void usart_set_read_callback(usart_read_callback_function callback);
void usart_delete_callback();
void usart_handle_callback(uint16_t* size, uint8_t* data);

#endif /* USART_H_ */