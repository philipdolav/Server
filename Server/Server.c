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
int main(int argc, char* argv[])
{
	WSADATA wsa_data;
	int result;
	struct hostent* remotehost = 0;
	char host_name[100];
	struct in_addr addr;
	
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
	SOCKET Sender_s, reciever_s;
	//if ((Sender_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) // Create and init socket in TCP protocol
//	{
//		printf("Could not create socket : %d", WSAGetLastError());
	//	return 1;
	//}
	if ((Sender_s = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		//if ((server_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
		return 1;
	}
	struct sockaddr_in sender_addr;
	sender_addr.sin_family = AF_INET;
	//sender_addr.sin_port = htons(atoi(argv[1]));
	//sender_addr.sin_port = (unsigned short)PORT_s;
	sender_addr.sin_port = htons(atoi(PORT_s));
	//sender_addr.sin_addr.s_addr = inet_addr(argv[1]);
	sender_addr.sin_addr.s_addr = inet_addr(inet_ntoa(addr));
	//sender_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int server_addrss_len = sizeof(sender_addr);
	

	//Bind
	if (bind(Sender_s, (struct sockaddr*)&sender_addr, sizeof(sender_addr)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//wait for data to be sent by the channel WSA_FLAG_OVERLAPPED
	int ListenRes = listen(Sender_s, SOMAXCONN);
	printf(" IP Adress  %s Port adress:  %d listen: %d ", inet_ntoa(sender_addr.sin_addr), sender_addr.sin_port, ListenRes);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		return 1;
	}
	SOCKET sender;
	sender = accept(Sender_s, NULL,NULL);
	if (sender == INVALID_SOCKET)
	{
		printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
	}

	struct sockaddr_in reciever_addr;
	reciever_addr.sin_family = AF_INET;
	reciever_addr.sin_port = 9001;
	reciever_addr.sin_addr.s_addr = inet_addr(inet_ntoa(addr));







}