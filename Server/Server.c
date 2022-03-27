//This Header file contains all of includes which are being used in main.c
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <math.h>
#include <crtdbg.h>
#include <string.h>
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdbool.h>
#define PORT_s "8888"
#define PORT_r "8889"
#define BUFFER_SIZE 1488
int Seed;
int rand_noise(char* buffer, double probability)
{
	/*
		Receives (char*) buffer, (int) seed, (double) probability, and "flips" every buffer's bit at the given probabilty.
		Return value: number of flipped bits.
	*/

	int probability_factor = 1 / probability;
	int mask = 1, flipped_bits_counter = 0;

	for (int i = 0; i < strlen(buffer); i++)
	{
		mask = 1;
		for (int j = 0; j < 8; j++) // noise is independent on every bit
		{
			mask *= 2; // shift left mask to next bit
			
			/* rand() returns a number between [0,2^15] so in case that int(2^16/probability) is 1
						we add another rand of probabilty 0.25 */
			if (pow(2, 16) == probability_factor)
			{
				if (rand() % 2 == 0)
				{
					continue;
				}
			}
			
			if ((rand() % probability_factor) == 0)
			{
				buffer[i] = buffer[i] ^ mask;
				flipped_bits_counter++;
			}
		}
	}

	return flipped_bits_counter;
}

int determinist_noise(int n, char* buffer) 
{ 
	int flipp_bit_counter = 0, mask;

	for (int i = 0; i < strlen(buffer); i++)
	{
		mask = 1;
		
		for (int j = 0; j < 8; j++)
		{
			mask *= 2;
			if ((8 * i + j) % n == 0) // bit number
			{
				buffer[i] = buffer[i] ^ mask;
				flipp_bit_counter++;
			}
			//mask *= 2; // shift left to next bit inside buffer[i]
		}
	}

	return flipp_bit_counter;
}


int main(int argc, char* argv[])
{
	WSADATA wsa_data;
	int result, flipped = 0; ///// need to check when to reset flipped
	struct hostent* remotehost = 0;
	char host_name[100];
	struct in_addr addr;
	char continue_abort[10] = {0};

	char* flag = argv[1];
	if (!strcmp(flag, "-r")) {
		Seed = atoi(argv[3]);
		srand(Seed);
	}
	SOCKET reciever, Sender_s, reciever_s, sender;
	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0) {
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	gethostname(host_name, sizeof(host_name));
	if (host_name == NULL) {
		printf("ERROR with host name");
	}
	remotehost = gethostbyname(host_name);
	if (remotehost == NULL) {
		printf("ERROR with remotehost name");
	}
	else {
		addr.s_addr = *(u_long*)remotehost->h_addr_list[0];
		printf("IP Adress  %s Port adress: ", inet_ntoa(addr));
	}
	// Create and init socket
	
	if ((Sender_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) // Create and init socket in TCP protocol
	{
		printf("Could not create socket : %d", WSAGetLastError());
		return 1;
	}
	if ((Sender_s = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
		return 1;
	}
	struct sockaddr_in sender_addr;
	sender_addr.sin_family = AF_INET;
	//sender_addr.sin_port = htons(atoi(argv[1]));
	//sender_addr.sin_port = (unsigned short)PORT_s;
	sender_addr.sin_port = htons(atoi(PORT_s));
	//sender_addr.sin_port = atoi(PORT_s);
	//sender_addr.sin_addr.s_addr = inet_addr(argv[1]);
	sender_addr.sin_addr.s_addr = inet_addr(inet_ntoa(addr));
	//sender_addr.sin_addr.s_addr = htonl(INADDR_ANY);


	//Bind and connect sender:
	if (bind(Sender_s, (struct sockaddr*)&sender_addr, sizeof(sender_addr)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//wait for data to be sent by the channel WSA_FLAG_OVERLAPPED
	int Listen_s = listen(Sender_s, SOMAXCONN);
	printf(" IP Adress  %s \nPort adress:  %s \nlisten: %d \n", inet_ntoa(sender_addr.sin_addr), PORT_s, Listen_s);
	if (Listen_s == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		return 1;
	}

	if ((reciever_s = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		//if ((server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d\n", WSAGetLastError());
		return 1;
	}

	struct sockaddr_in reciever_addr;
	reciever_addr.sin_family = AF_INET;
	reciever_addr.sin_port = htons(atoi(PORT_r));
	reciever_addr.sin_addr.s_addr = inet_addr(inet_ntoa(addr));

	// Bind and connect reciever:
	if (bind(reciever_s, (struct sockaddr*)&reciever_addr, sizeof(reciever_addr)) == SOCKET_ERROR)
	{
		printf("Bind2 failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//wait for data to be sent by the channel WSA_FLAG_OVERLAPPED
	int Listen_r = listen(reciever_s, SOMAXCONN);
	printf("Reciever IP Adress  %s\nPort adress:  %s\nlisten: %d ", inet_ntoa(reciever_addr.sin_addr), PORT_r, Listen_r);
	if (Listen_r == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		return 1;
	}
	
	int buff_length, flipped_bits = 0, buff_size = 0, packet_count = 0;
	unsigned char buffer[BUFFER_SIZE + 2] = { 0 };

	double probability;

	if(!strcmp(flag,"-r"))
		probability = atoi(argv[2]) / (pow(2, 16));

	do {
		sender = accept(Sender_s, NULL, NULL);
		if (sender == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		}
		reciever = accept(reciever_s, NULL, NULL); // do we need to have a timeout
		if (reciever == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		}
		do {
			// Receives data from sender, adds noise and sends noised data to receiver.
			buff_length = recv(sender, buffer, BUFFER_SIZE + 2, 0);
			buff_size += buff_length;
			packet_count++;
			if (buff_length > 0) {
				for (int i = buff_length; i < BUFFER_SIZE; i++)
				{
					buffer[i] = '\0';
				}
				if (!strcmp(flag, "-r")) {
					flipped += rand_noise(buffer, probability);
				}
				else {
					flipped += determinist_noise(atoi(argv[2]), buffer);
				}
				if (send(reciever, buffer, strlen(buffer), 0) == SOCKET_ERROR)
				{
					printf("send() failed with error code : %d", WSAGetLastError());
					exit(EXIT_FAILURE);
				}

			}
			else if (buff_length == 0) {
			}
			else
				printf("recv failed: %d\n", WSAGetLastError());

		} while (buff_length > 0);

		/*for (int i = 0; i < buff_size; i++)
		{
			buffer[i] = '\0';
		}*/
		closesocket(sender);
		closesocket(reciever);
		printf("retransmitted %d bytes, flipped %d bits\n", buff_size ,flipped);
		// resets counters
		buff_size = 0;
		flipped = 0;
		packet_count = 0;
		printf("continue? (yes to continue / anything else to finish)\n");
		scanf("%s", continue_abort);
	} while ((!strcmp(continue_abort, "yes")|| !strcmp(continue_abort, "Yes")));
	closesocket(Sender_s);
	closesocket(reciever_s);
	return 0;
		//closeallsockets

}






