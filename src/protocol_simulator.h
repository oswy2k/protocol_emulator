#ifndef PROTCOL_SIMULATOR_H
#define PROTCOL_SIMULATOR_H

#include "./helpers/socket/win_socket.h"

/* Enums */
typedef enum _comm_type_enum{
	GPIO=0,
	USART,
	SPI,
	I2C
} comm_type_enum;

typedef enum _simulator_error{
	SIMULATOR_ERROR_NONE = 0,
	SIMULATOR_ERROR_SOCKET_INIT,
	SIMULATOR_ERROR_SOCKET_CONNECT,
	SIMULATOR_ERROR_SOCKET_DISCONNECT,
	SIMULATOR_ERROR_LOOP_HANDLE,
	SIMULATOR_ERROR_INVALID_DATA
} simulator_error;

typedef enum _simulator_state{
	SIMULATOR_UNINITIALIZED,
	SIMULATOR_INITIALIZED,
	SIMULATOR_ERROR
} simulator_state;

/* structs */
typedef struct _protocol_simulator_t{
	simulator_state state;
	simulator_error error;
	socket_t socket;
} protocol_simulator_t;

/* Functions Typedef */
/* Public functions */
simulator_error protocol_simulator_init(const char *hostname, uint16_t port);
simulator_error protocol_simulator_close();
bool protocol_simulator_is_running();
void protocol_simulator_error_verbose(simulator_error error);

#endif //PROTCOL_SIMULATOR_H