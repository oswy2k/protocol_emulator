#include "gpio.h"
#include <stdio.h>
/* Define global structure */
gpio_interface_t gpio_interface = {0};

/* Private functions */
static void gpio_pin_set_state(uint8_t bank, uint8_t pin, uint8_t state){
	if(state == GPIO_HIGH){
		gpio_interface.pin_state[bank] |= (1 << pin);
	}
	else{
		gpio_interface.pin_state[bank] &= ~(1 << pin);
	}
}

static bool gpio_pin_get_state(uint8_t bank, uint8_t pin){
	return (gpio_interface.pin_state[bank] & (1 << pin));
}

static void gpio_pin_set_mode(uint8_t bank, uint8_t pin, uint8_t mode){
	if(mode == GPIO_OUTPUT){
		gpio_interface.pin_mode[bank] |= (1 << pin);
	}
	else{
		gpio_interface.pin_mode[bank] &= ~(1 << pin);
	}
}

static bool gpio_pin_get_mode(uint8_t bank, uint8_t pin){
	return (gpio_interface.pin_mode[bank] & (1 << pin));
}

static void gpio_pin_set_interrupt(uint8_t bank, uint8_t pin, gpio_interrupt_mode mode, gpio_callback_function callback){

	if(callback == NULL || mode == GPIO_INTERRUPT_NONE){
		return;
	}

	/* Set interrupt mode and callback */
	gpio_interface.interrrupt_mode[bank][pin] = mode;
	gpio_interface.pin_callback[bank][pin] = callback;
}

static void gpio_pin_remove_interrupt(uint8_t bank, uint8_t pin){
	gpio_interface.interrrupt_mode[bank][pin] = GPIO_INTERRUPT_NONE;
	gpio_interface.pin_callback[bank][pin] = NULL;
}

static gpio_callback_function gpio_pin_get_interrupt(uint8_t bank, uint8_t pin){
	if(gpio_interface.pin_callback[bank][pin] == NULL){
		return NULL;
	}
	else{
		return gpio_interface.pin_callback[bank][pin];
	}
}

static uint8_t gpio_pin_get_interrupt_mode(uint8_t bank, uint8_t pin){
	return gpio_interface.interrrupt_mode[bank][pin];
}

/* Public Functions */
void gpio_init(socket_t* socket){

	/* Check if the gpio_interface interface is already initialized */
	if(gpio_interface.interface_state == GPIO_INITIALIZED){
		return;
	}

	/* Set all pins to low state, read mode without interrupt */
	for(int i = 0; i < GPIO_BANK_SIZE; i++){
		gpio_interface.pin_state[i] = 0;
		gpio_interface.pin_mode[i] = 0;

		/* Set all callbacks to null */
		for(int j = 0; j < 8; j++){
			gpio_interface.pin_callback[i][j] = (gpio_callback_function) NULL;
		}
	}

	gpio_interface.interface_state = GPIO_INITIALIZED;

	/* Set socket */
	gpio_interface.socket = socket;
}


void gpio_close(){
	/* Check if the gpio_interface interface is already initialized */
	if(gpio_interface.interface_state != GPIO_INITIALIZED){
		return;
	}

	gpio_interface.interface_state = GPIO_UNINITIALIZED;

	/* close socket */
	gpio_interface.socket = NULL;
}

/**********************************************************************/
/* 					Handle GPIO modifiers	 						  */
/**********************************************************************/
void gpio_configure(uint8_t size, gpio_config_t* config_data){

	if(gpio_interface.interface_state != GPIO_INITIALIZED){
		return;
	}

	gpio_config_t* config_pin = (gpio_config_t*)NULL;

	if(size == 0){
		return;
	}

	/* Configure size */
	queue_item_t item ={
		.size = sizeof(gpio_pin_t) + sizeof(uint8_t) + sizeof(uint8_t)
	};

	socket_error_t error;

	/* Copy data */
	item.buffer[0] = GPIO;
	item.buffer[1] = GPIO_CONFIGURE;

	for(int i = 0; i < size; i++){
		config_pin = (config_data + i);
		gpio_pin_set_state(config_pin->pin.bank, config_pin->pin.pin, config_pin->pin.state);
		gpio_pin_set_mode(config_pin->pin.bank, config_pin->pin.pin, config_pin->pin.mode);
		gpio_pin_set_interrupt(config_pin->pin.bank, config_pin->pin.pin, config_pin->interrupt_mode, config_pin->callback);

		memcpy(&item.buffer[2], config_pin, sizeof(gpio_pin_t));

		/* Write item */
		error = socket_write(gpio_interface.socket, &item, SOCKET_BLOCK);
		if(error != SOCKET_ERROR_NONE){
			printf("Error writing GPIO\n");
		}
	}
}

static void gpio_write_socket(uint8_t bank, uint8_t pin, uint8_t state){
	queue_item_t item = {
		.size = sizeof(gpio_pin_t) + sizeof(uint8_t) + sizeof(uint8_t)
	};

	socket_error_t error;

	/* Copy data */
	item.buffer[0] = GPIO;
	item.buffer[1] = GPIO_WRITE;

	gpio_pin_t config_pin = {
		.bank = bank,
		.pin = pin,
		.mode = GPIO_OUTPUT,
		.state = state
	};

	memcpy(&item.buffer[2], &config_pin, sizeof(gpio_pin_t));

	/* Write item */
	error = socket_write(gpio_interface.socket, &item, SOCKET_BLOCK);
	if(error != SOCKET_ERROR_NONE){
		printf("Error writing GPIO\n");
	}
}

void gpio_pin_set(uint8_t bank, uint8_t pin){
	if(gpio_interface.interface_state != GPIO_INITIALIZED){
		return;
	}

	/* Check if pin is output */
	if(gpio_pin_get_mode(bank, pin) != GPIO_OUTPUT){
		printf("Invalid pin mode for set\n");
		return;
	}

	/* Set pin */
	gpio_pin_set_state(bank, pin, GPIO_HIGH);

	/* Send through socket*/
	gpio_write_socket(bank, pin, GPIO_HIGH);
}

void gpio_pin_reset(uint8_t bank, uint8_t pin){
	if(gpio_interface.interface_state != GPIO_INITIALIZED){
		return;
	}

	/* Check if pin is output */
	if(gpio_pin_get_mode(bank, pin) != GPIO_OUTPUT){
		printf("Invalid pin mode for set\n");
		return;
	}

	/* Set pin */
	gpio_pin_set_state(bank, pin, GPIO_LOW);

	/* Send through socket*/
	gpio_write_socket(bank, pin, GPIO_HIGH);
}

void gpio_pin_toggle(uint8_t bank, uint8_t pin){
	if(gpio_interface.interface_state != GPIO_INITIALIZED){
		return;
	}

	/* Check if pin is output */
	if(gpio_pin_get_mode(bank, pin) != GPIO_OUTPUT){
		printf("Invalid pin mode for set\n");
		return;
	}

	/* Get state*/
	bool state = gpio_pin_get_state(bank, pin);

	/* Set pin */
	gpio_pin_set_state(bank, pin, !state);

	/* Send through socket*/
	gpio_write_socket(bank, pin, !state);
}


bool gpio_pin_get(uint8_t bank, uint8_t pin){
	if(gpio_interface.interface_state != GPIO_INITIALIZED){
		return GPIO_LOW;
	}

	/* Check if pin is output */
	if(gpio_pin_get_mode(bank, pin) != GPIO_INPUT){
		printf("Invalid pin mode for read\n");
		return GPIO_LOW;
	}

	return gpio_pin_get_state(bank, pin);
}

/**********************************************************************/
/* 					Handle main loop functions 						  */
/**********************************************************************/
static void gpio_handle_write_packet(uint8_t size, void* data){
	gpio_pin_t* config_pin = (gpio_pin_t*) data;

	if(size != 4){
		printf("Invalid size for gpio read packet\n");
		return;
	}

	/* Check if pin is input */
	if(gpio_pin_get_mode(config_pin->bank, config_pin->pin) != GPIO_INPUT){
		printf("Invalid pin mode for read\n");
		return;
	}

	/* Check if there is a callback for this pin */
	gpio_callback_function callback = gpio_pin_get_interrupt(config_pin->bank, config_pin->pin);
	gpio_interrupt_mode interrupt_mode = gpio_pin_get_interrupt_mode(config_pin->bank, config_pin->pin);

	/* Check for actual pin value and new pin value */
	bool last_state = gpio_pin_get_state(config_pin->bank, config_pin->pin);
	bool new_state = config_pin->state;

	/* Set new state */
	gpio_pin_set_state(config_pin->bank, config_pin->pin, new_state);

	/* Check if there is a callback available */
	if(callback != NULL){
		/* Check for different types of interrupt handling */
		if(interrupt_mode == GPIO_INTERRUPT_RISING && last_state == GPIO_LOW && new_state == GPIO_HIGH){
			callback(config_pin->bank, config_pin->pin);
		}
		else if(interrupt_mode == GPIO_INTERRUPT_FALLING && last_state == GPIO_HIGH && new_state == GPIO_LOW){
			callback(config_pin->bank, config_pin->pin);
		}
		else if(interrupt_mode == GPIO_INTERRUPT_BOTH){
			callback(config_pin->bank, config_pin->pin);
		}
	}
}


static void gpio_handle_interrupt_packet(uint8_t size, void* data){
	gpio_pin_t* config_pin = (gpio_pin_t*) data;

	if(size != 4){
		printf("Invalid size for gpio interrupt packet\n");
		return;
	}

	/* Check if pin is input */
	if(gpio_pin_get_mode(config_pin->bank, config_pin->pin) != GPIO_INPUT){
		printf("Invalid pin mode for interrupt\n");
		return;
	}

	/* Check if there is a callback for this pin */
	gpio_callback_function callback = gpio_pin_get_interrupt(config_pin->bank, config_pin->pin);
	gpio_interrupt_mode interrupt_mode = gpio_pin_get_interrupt_mode(config_pin->bank, config_pin->pin);

	if(callback != NULL){
		callback(config_pin->bank, config_pin->pin);
	}
}

void gpio_handle_main_loop(uint16_t* size, uint8_t* data){

	if(gpio_interface.interface_state != GPIO_INITIALIZED){
		return;
	}

	switch(data[1]){
		case GPIO_WRITE:
			printf("Received write package\n");
			gpio_handle_write_packet((*size) -2, &data[2]);
			break;
		case GPIO_INTERRUPT:
			printf("Received interrupt package\n");
			gpio_handle_interrupt_packet((*size) -2, &data[2]);
			break;
		default:
			printf("Invalid mode for gpio packet\n");
			break;
	}
}
