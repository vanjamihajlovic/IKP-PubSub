#pragma once
#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "..\Common\SocketOperations.h"


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016
#define SERVER_SLEEP_TIME 50


bool appRunning = true;
bool serverStopped = false;

int SelectFunction(SOCKET, char);
void PrintMenu();
void ProcessInputAndGenerateMessage(char, char*);
int SendFunction(SOCKET, char*, int);
char* ReceiveFunction(SOCKET, char*);
bool AlreadySubscribed(char, int[], int);
int Connect(SOCKET);


int Connect(SOCKET connectSocket) {
	char* connectMessage = (char*)malloc(10 * sizeof(char));
	strcpy(connectMessage, "s:Connect");

	int messageDataSize = strlen(connectMessage) + 1;
	int messageSize = messageDataSize + sizeof(int);

	MessageStruct* messageStruct = GenerateMessageStruct(connectMessage, messageDataSize);

	int retVal = SendFunction(connectSocket, (char*)messageStruct, messageSize);
	free(messageStruct);
	free(connectMessage);

	return retVal;

}


bool AlreadySubscribed(char c, int subscribed[], int numOfSubscribedTopics) {
	for (int i = 0; i < numOfSubscribedTopics; i++) {
		if (subscribed[i] == c - '0') {
			return true;
		}
	}
	return false;
}


char* ReceiveFunction(SOCKET acceptedSocket, char* recvbuf) {

	int iResult;
	char* myBuffer = (char*)(malloc(DEFAULT_BUFLEN));

	int selectResult = SelectFunction(acceptedSocket, 'r');
	if (selectResult == -1) {
		memcpy(myBuffer, "ErrorS", 7);
		return myBuffer;
	}
	iResult = recv(acceptedSocket, recvbuf, 4, 0);

	if (iResult > 0)
	{
		int bytesExpected = *((int*)recvbuf);

		int recvBytes = 0;

		while (recvBytes < bytesExpected) {

			SelectFunction(acceptedSocket, 'r');
			iResult = recv(acceptedSocket, myBuffer + recvBytes, bytesExpected - recvBytes, 0);

			recvBytes += iResult;
		}
	}
	else if (iResult == 0)
	{
		memcpy(myBuffer, "ErrorC", 7);
	}
	else
	{
		memcpy(myBuffer, "ErrorR", 7);
	}
	return myBuffer;

}


int SendFunction(SOCKET connectSocket, char* message, int messageSize) {

	int selectResult = SelectFunction(connectSocket, 'w');
	if (selectResult == -1) {
		return -1;
	}
	int iResult = send(connectSocket, message, messageSize, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return SOCKET_ERROR;
	}
	else {

		int sentBytes = iResult;
		while (sentBytes < messageSize) {

			SelectFunction(connectSocket, 'w');
			iResult = send(connectSocket, message + sentBytes, messageSize - sentBytes, 0);
			sentBytes += iResult;
		}
	}

	return 1;
}


int SelectFunction(SOCKET listenSocket, char rw) {
	int iResult = 0;
	do {
		FD_SET set;
		timeval timeVal;

		FD_ZERO(&set);

		FD_SET(listenSocket, &set);

		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		if (!appRunning || serverStopped) {
			return -1;
		}


		if (rw == 'r') {
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		}
		else {
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		}



		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			continue;
		}


		if (iResult == 0)
		{

			Sleep(SERVER_SLEEP_TIME);
			continue;
		}
		break;

	} while (1);
	return 1;

}


void PrintMenu() {
	printf("\nChoose a topic to subscribe to: \n");
	printf("\t1.Weather\n");
	printf("\t2.Chess\n");
	printf("\t3.Gaming\n");
	printf("\t4.Animals \n");
	printf("\t5.Food \n\n");
	printf("Press X if you want to close connection\n");
}


void ProcessInputAndGenerateMessage(char input, char* message) {
	switch (input) {
	case '1':
		strcpy(message, "s:Weather");
		printf("You subscribed on Weather.\n");
		break;
	case '2':
		strcpy(message, "s:Chess");
		printf("You subscribed on Chess.\n");
		break;
	case '3':
		strcpy(message, "s:Gaming");
		printf("You subscribed on Gaming.\n");
		break;
	case '4':
		strcpy(message, "s:Animals");
		printf("You subscribed on Animals.\n");
		break;
	case '5':
		strcpy(message, "s:Food");
		printf("You subscribed on Food.\n");
		break;
	default:
		break;
	}
}