# Minimum version of CMake required to build this project - Required #
cmake_minimum_required(VERSION 3.31.5)

# Set the generator to MinGW Makefiles #
set(CMAKE_GENERATOR "Unix Makefiles")

# Set the C standard to C23 #
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)

# Name of the project - Required #
project(Protocol_Emulator VERSION 0.0.1 LANGUAGES C)

# Create variables for compilation #
set(PROJECT_SRC ${CMAKE_SOURCE_DIR}/src)
set(QUEUE_SRC ${CMAKE_SOURCE_DIR}/src/helpers/queue)
set(SOCKET_SRC ${CMAKE_SOURCE_DIR}/src/helpers/socket)
set(GPIO_SRC ${CMAKE_SOURCE_DIR}/src/interface/gpio)
set(USART_SRC ${CMAKE_SOURCE_DIR}/src/interface/usart)

# Make Executable from source file #
add_executable(simulator  ${PROJECT_SRC}/protocol_simulator.c)

# Create library for socket #
add_library(socket ${SOCKET_SRC}/win_socket.c)
target_link_libraries(simulator PRIVATE socket)

# Create library for queue #
add_library(queue ${QUEUE_SRC}/mutex_queue.c)
target_link_libraries(socket PRIVATE queue)

# Create library for gpio #
add_library(gpio ${GPIO_SRC}/gpio.c)
target_link_libraries(simulator PUBLIC gpio PRIVATE socket)

# Create library for usart 
add_library(usart ${USART_SRC}/usart.c)
target_link_libraries(simulator PUBLIC usart PRIVATE socket)


# Search for windows api #
if(WIN32)
	target_link_libraries(simulator PRIVATE wsock32 ws2_32)
endif()