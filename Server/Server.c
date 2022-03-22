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
#define BUFFER_SIZE 3100

int rand_noise(char* buffer, int seed, double probability)
{
	/*
		Receives (char*) buffer, (int) seed, (double) probability, and "flips" every buffer's bit at the given probabilty.
		Return value: number of flipped bits.
	*/

	int probability_factor = 1 / probability;
	int mask = 1, flipp_bit_counter = 0;
	srand(seed);

	for (int i = 0; i < strlen(buffer); i++)
	{
		mask = 1;
		for (int j = 0; j < 8; j++) // noise is independent on every bit
		{
			/*
			if (pow(2, 16) == MAX_P)
			{
				if (rand() % 2 == 0)
				{
					continue;
				}
			}
			*/
			if ((rand() % probability_factor) == 0)
			{
				buffer[i] = buffer[i] ^ mask;
				flipp_bit_counter++;
			}
			mask *= 2; // shift left to next bit inside buffer[i]
		}
	}

	return flipp_bit_counter;
}

int determinist_noise(int n, char* buffer) 
{ 
	int flipp_bit_counter = 0, mask;

	for (int i = 0; i < strlen(buffer); i++)
	{
		mask = 1;
		for (int j = 0; j < 8; j++)
		{
			if ((8 * i + j) % n == 0) // bit number
			{
				buffer[i] = buffer[i] ^ mask;
				flipp_bit_counter++;
			}
			mask *= 2; // shift left to next bit inside buffer[i]
		}
	}

	return flipp_bit_counter;
}


int main(int argc, char* argv[])
{
	WSADATA wsa_data;
	int result;
	struct hostent* remotehost = 0;
	char host_name[100];
	struct in_addr addr;
	char continue_abort[10] = {0};
	SOCKET reciever, Sender_s, reciever_s, sender;
	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0) {
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	gethostname(host_name, sizeof(host_name));
	if (host_name == NULL) {
		printf("ERROR woth host name");
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
	printf(" IP Adress  %s \nPort adress:  %d \nlisten: %d \n", inet_ntoa(sender_addr.sin_addr), sender_addr.sin_port, Listen_s);
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
	printf(" IP Adress  %s Port adress:  %d listen: %d ", inet_ntoa(reciever_addr.sin_addr), reciever_addr.sin_port, Listen_r);
	if (Listen_r == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		return 1;
	}
	
	int buff_length, flipped_bits = 0, transmited_bytes =0;
	unsigned char buffer[BUFFER_SIZE + 2] = { 0 };

	//char* flag = argv[1];
	double probability;

	//if(!strcmp(flag,'-r'))
		//probability = atoi(argv[2]) / (pow(2, 16));

	do {
		sender = accept(Sender_s, NULL, NULL);
		if (sender == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		}
		reciever = accept(reciever_s, NULL, NULL);
		if (reciever == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		}
		do {
			buff_length = recv(sender, buffer, BUFFER_SIZE + 2, 0);
			if (buff_length > 0) {
				//printf("Bytes received: %d\n the strins is %s\n", buff_length, buffer);
				transmited_bytes += buff_length;
				//add noise and count the flipped bits
				if (send(reciever, buffer, strlen(buffer), 0) == SOCKET_ERROR)
				{
					printf("send() failed with error code : %d", WSAGetLastError());
					exit(EXIT_FAILURE);
				}

			}
			else if (buff_length == 0) {
				printf("Connection closed\n");
			}
			else
				printf("recv failed: %d\n", WSAGetLastError());

		} while (buff_length > 0);

		closesocket(sender);
		closesocket(reciever);
		printf("print something");
		printf("continue? (yes/no)\n");
		transmited_bytes = 0;
		scanf("%s", continue_abort);
	} while ((strcmp(continue_abort, "yes")|| strcmp(continue_abort, "Yes"))==0);
	closesocket(Sender_s);
	closesocket(reciever_s);
	return 0;
		//closeallsockets
	


		
		// Noise insert:
		//if (!strcmp(flag, '-r'))
			//flipped_bits += rand_noise(buffer, argv[3], probability);
	//	else if (!strcmp(flag, '-d'))
		//	flipped_bits += determinist_noise(argv[2], buffer);

		//Send to reciever:



	
}






