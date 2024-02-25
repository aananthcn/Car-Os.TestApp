/*
 * Created on Wed Feb 21 2024 10:33:56 AM
 *
 * The MIT License (MIT)
 * Copyright (c) 2024 Aananth C N
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

#include <TcpIp.h>
#include <tcpip_extensions.h>

#include <string.h>


static u8 TcpState = TCP_CLOSED;


int tcp_socket_init(void) {
	int retval = -1;
	uint16 port = 1000; // change this if you want.
	Std_ReturnType tcp_retstat;



	if (TcpState == TCP_CLOSED) {
		tcp_retstat = TcpIp_Bind(0, 0, &port);
		if (tcp_retstat == E_OK) {
			TcpState = TCP_BIND;
		}
	}

	if ((TcpState == TCP_BIND) || (TcpState == TCP_LISTEN)) {
		tcp_retstat = TcpIp_TcpListen(0, 0); // call once, this will return immediately, but an accept call back will be called.
		if (tcp_retstat == E_OK)  {
			TcpState = TCP_ACCEPT;
		}
		else {
			TcpState = TCP_LISTEN;
		}
	}

	if (TcpState == TCP_ACCEPT) {
		retval = 0;
	}

	return retval;
}


void tcp_socket_main(void) {
	uint8_t eth_data[1500];
	int data_len;
	char reply[] = "Car-OS # Hey \"tcp_client\", you sent me: \"";
	uint8_t reply_len;

	// frame a response
	strcpy(eth_data, reply);
	reply_len = strlen(reply);

	if (TcpState < TCP_ACCEPT) {
		tcp_socket_init();
	}

	if (TcpState >= TCP_ACCEPT) {
		data_len = TcpIp_recv(eth_data+reply_len);

		// do an echo with a Car-OS reply!
		eth_data[data_len+reply_len] = '\"';
		if(data_len > 0) {
			TcpIp_send(eth_data, data_len+reply_len+1);
		}
	}
}
