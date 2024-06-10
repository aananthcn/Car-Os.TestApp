/*
 * Created on Wed Feb 21 2024 10:33:56 AM
 *
 * The MIT License (MIT)
 * Copyright (c) 2024 Aananth C N, Krishnaswamy D
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <socket.h>

#include <string.h>

typedef enum server_sock_state {
	SERVER_SOCK_STATE_INIT 		= 0,
	SERVER_SOCK_STATE_BIND 		= 1,
	SERVER_SOCK_STATE_LISTEN 	= 2,
	SERVER_SOCK_STATE_ACCEPT 	= 3,
	SERVER_SOCK_STATE_LOOP 		= 4
} server_sock_state_t;

server_sock_state_t state = SERVER_SOCK_STATE_INIT;

// This function is cyclically called from the Ethernet_Tasks defined in ethernet_test.c
//
// This is basically a different form of Echo server, which will first expect some data
// to be sent by a client, so that it can respond to it.
void tcp_server_socket_main(void) {
	uint8_t eth_data[1500];
	int data_len;
	char reply[] = "Car-OS # Hey \"tcp_client\", you sent me: \"";
	uint8_t reply_len;

	// frame a response on the same mem buffer
	strcpy(eth_data, reply);
	reply_len = strlen(reply);

	// initialize socket if it isn't initialized
	if(SERVER_SOCK_STATE_INIT == state) {
		if (0 != socket(0)) {
			state = SERVER_SOCK_STATE_BIND;
		}
	}

	if(SERVER_SOCK_STATE_BIND == state) {
		if(0 != bind(0)) {
			state = SERVER_SOCK_STATE_LISTEN;
		}
	}

	if(SERVER_SOCK_STATE_LISTEN == state) {
		if(0 != listen(0)) {
			state = SERVER_SOCK_STATE_ACCEPT;
		}
	}

	if(SERVER_SOCK_STATE_ACCEPT == state) {
		if(0 == accept(0)) {
			state = SERVER_SOCK_STATE_LOOP;
		}
	}

	if(SERVER_SOCK_STATE_LOOP == state) {
		// cyclically receive and send on an initialized socket
		data_len = recv(0, eth_data, 1500);

		// do an echo with a Car-OS reply!
		eth_data[data_len+reply_len] = '\"';
		if(data_len > 0) {
			send(0, eth_data, data_len+reply_len+1);
		}
	}
}
