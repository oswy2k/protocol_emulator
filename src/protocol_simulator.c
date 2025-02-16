#include<stdio.h>

#include "protocol_simulator.h"
#include "interface/gpio/gpio.h"
#include "interface/usart/usart.h"
/* Global variables */
protocol_simulator_t g_protocol_simulator = {0};

/************************************************************/
/* 						Private functions 					*/
/************************************************************/
static void protocol_simulator_handle_data(uint16_t* size, uint8_t* data){

	switch(data[0]){
		case GPIO:
			gpio_handle_main_loop(size, data);
			socket_delete(&g_protocol_simulator.socket);
			break;
		case USART:
			printf("Received USART package\n");
			usart_handle_callback(size, data);
			break;
		case SPI:
			printf("Received SPI package\n");
			break;
		case I2C:
			printf("Received I2C package\n");
			break;
		default:
			g_protocol_simulator.error = SIMULATOR_ERROR_INVALID_DATA;
			break;
	}

}

/************************************************************/
/* 						Public functions 					*/
/************************************************************/
simulator_error protocol_simulator_init(const char *hostname, uint16_t port){
	
	socket_error_t error;

	/* Socket initialization */
	error =socket_init(&g_protocol_simulator.socket);
	if(error != SOCKET_ERROR_NONE){
		printf("Code:%i\n", error);
		return SIMULATOR_ERROR_SOCKET_INIT;
	}

	error = socket_connect(&g_protocol_simulator.socket, hostname, port);
	
	if(error != SOCKET_ERROR_NONE){
		printf("Code:%i\n", error);
		return SIMULATOR_ERROR_SOCKET_CONNECT;
	}

	/* Initilize interface modules */
	gpio_init(&g_protocol_simulator.socket);
	usart_init(USART_SYNCHRONOUS, &g_protocol_simulator.socket);

	g_protocol_simulator.state = SIMULATOR_INITIALIZED;

	return SIMULATOR_ERROR_NONE;
}

simulator_error protocol_simulator_close(){
	socket_error_t error;

	error = socket_disconnect(&g_protocol_simulator.socket);
	if(error != SOCKET_ERROR_NONE){
		printf("Code:%i\n", error);
		return SIMULATOR_ERROR_SOCKET_DISCONNECT;
	}

	g_protocol_simulator.state = SIMULATOR_UNINITIALIZED;

	return SIMULATOR_ERROR_NONE;
}


bool protocol_simulator_is_running(){
	
	if(socket_is_connected(&g_protocol_simulator.socket)==SOCKET_STATUS_CONNECTED){
		return true;
	}
	else{
		g_protocol_simulator.state = SIMULATOR_ERROR;
		return false;
	}
}

simulator_error protocol_emulator_loop_handle(){
	queue_item_t item;
	socket_error_t error;

	error = socket_peek(&g_protocol_simulator.socket, &item);

	if(error == SOCKET_ERROR_NO_DATA){
		return SIMULATOR_ERROR_NONE;
	}

	if((error != SOCKET_ERROR_NONE) ){
		printf("Error reading from socket\n");
		return SIMULATOR_ERROR_LOOP_HANDLE;
	}

	/* print data*/
	for(int i = 0; i < item.size; i++){
		printf("Data[%i]: %i\n", i, item.buffer[i]);
	}

	/* Handle package */
	protocol_simulator_handle_data(&item.size, item.buffer);
	return SIMULATOR_ERROR_NONE;
}

void protocol_simulator_error_verbose(simulator_error error){
	switch(error){
		case SIMULATOR_ERROR_NONE:
			return;
		case SIMULATOR_ERROR_SOCKET_INIT:
			printf("Socket initialization error\n");
			exit(SIMULATOR_ERROR_SOCKET_INIT);
		case SIMULATOR_ERROR_SOCKET_CONNECT:
			printf("Socket connection error\n");
			exit(SIMULATOR_ERROR_SOCKET_CONNECT);
		case SIMULATOR_ERROR_SOCKET_DISCONNECT:
			printf("Socket disconnection error\n");
			exit(SIMULATOR_ERROR_SOCKET_DISCONNECT);
		case SIMULATOR_ERROR_INVALID_DATA:
			printf("Invalid data error\n");
			exit(SIMULATOR_ERROR_INVALID_DATA);
		default:
			printf("Unknown error\n");
			exit(-1);
	}
}

/************************************************************/
/* 						Testing Area	 					*/
/************************************************************/
void test_gpio_callback(uint8_t bank, uint8_t pin){
	printf("GPIO Callback Bank:%i, Pin:%i\n", bank, pin);
}

int main(){
	
	protocol_simulator_error_verbose(protocol_simulator_init("localhost", 3000));

	gpio_config_t config_data[] = {
		{
			.pin = {
				.bank = GPIO_BANK_0,
				.pin = GPIO_PIN_0,
				.mode = GPIO_INPUT,
				.state = GPIO_HIGH
			},
			.interrupt_mode = GPIO_INTERRUPT_NONE,
			.callback = test_gpio_callback
		}
	};

	gpio_configure(1, config_data);

	uint8_t data_1[] = {0x00, 0x00, 0xff};
	usart_write(3, data_1);	

	uint8_t data_2[] = {0x00, 0x10, 0x0f};
	usart_write(3, data_2);	

	uint8_t data_3[] = {0x00, 0x11, 0x02};
	usart_write(3, data_3);	

	uint8_t data_4[] = {0x00, 0x20, 0x11};
	usart_write(3, data_4);	

	uint8_t data_5[] = {0x40, 0x99,0x18, 0x18, 0x99};
	usart_write(5, data_5);

	while(protocol_simulator_is_running()){
		protocol_simulator_error_verbose(protocol_emulator_loop_handle());

		Sleep(100);
	}

	protocol_simulator_error_verbose(protocol_simulator_close());
}