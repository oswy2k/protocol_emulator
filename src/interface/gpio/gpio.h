#ifndef GPIO_H
#define GPIO_H

/*Includes*/
#include <stdint.h>
#include <stdbool.h>

#include "../../helpers/socket/win_socket.h"

/* GPIO MACROS */
#define GPIO 0

/* GPIO enum */
typedef enum _gpio_bank_enum{
	GPIO_BANK_0 = 0,
	GPIO_BANK_SIZE
} gpio_bank_enum;

/* Defines pin to select inside register */
typedef enum _gpio_pin_enum{
	/* GPIO */
	GPIO_PIN_NONE = 0xff,
	GPIO_PIN_0 = 0,
	GPIO_PIN_1 = 1,
	GPIO_PIN_2 = 2,
	GPIO_PIN_3 = 3,
	GPIO_PIN_4 = 4,
	GPIO_PIN_5 = 5,
	GPIO_PIN_6 = 6,
	GPIO_PIN_7 = 7,
} gpio_pin_enum;

/* Defines hte mode of the gpio pin */
typedef enum _gpio_pin_mode{
	GPIO_INPUT = 0,
	GPIO_OUTPUT = 1
} gpio_pin_mode;

/* Defines the state of the gpio pin */
typedef enum _gpio_pins_state{
	GPIO_LOW = 0,
	GPIO_HIGH = 1
}	gpio_pin_state;

typedef enum _gpio_comm_mode_enum{
	GPIO_READ =0,
	GPIO_WRITE,
	GPIO_INTERRUPT,
	GPIO_CONFIGURE
} gpio_comm_mode_enum;

/* Defines the state of the inteface for handling gpio */
typedef enum _gpio_interface_state{
	GPIO_UNINITIALIZED,
	GPIO_INITIALIZED,
	GPIO_ERROR
} gpio_interface_state;

/* Defines the interrupt mode for each pin */
typedef enum _gpio_interrupt_mode{
	GPIO_INTERRUPT_NONE,
	GPIO_INTERRUPT_RISING,
	GPIO_INTERRUPT_FALLING,
	GPIO_INTERRUPT_BOTH
} gpio_interrupt_mode;

/* Typedef callbacks */
typedef void (*gpio_callback_function)(uint8_t bank,uint8_t pin);

/* GPIO Pin struct */
typedef struct _gpio_pin_t{
	uint8_t bank;
	uint8_t pin;
	bool mode;
	bool state;
} gpio_pin_t;

/* GPIO Pin confic struct */
typedef struct _gpio_config_t{
	gpio_pin_t pin;
	uint8_t interrupt_mode;
	gpio_callback_function callback;
} gpio_config_t;

/* GPIO Interface struct */
typedef struct _gpio_interface_t{

	socket_t* socket;

	uint8_t pin_state[GPIO_BANK_SIZE];
	uint8_t pin_mode[GPIO_BANK_SIZE];
	struct{
		gpio_callback_function pin_callback[GPIO_BANK_SIZE][8];
		uint8_t interrrupt_mode[GPIO_BANK_SIZE][8];
	};

	gpio_interface_state interface_state;
} gpio_interface_t;

/* Public functions */

/* Gpio interface functions */
void gpio_init(socket_t* socket);
void gpio_close();

/* Interface functions */
void gpio_configure(uint8_t size, gpio_config_t* config_data);

/* GPIO Functions */
void gpio_pin_set(uint8_t bank, uint8_t pin);
void gpio_pin_reset(uint8_t bank, uint8_t pin);
void gpio_pin_toggle(uint8_t bank, uint8_t pin);
bool gpio_pin_get(uint8_t bank, uint8_t pin);



/* handle packets recieved automatically */
void gpio_handle_main_loop(uint16_t* size, uint8_t* data);

#endif // GPIO_H