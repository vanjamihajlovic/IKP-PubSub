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
#define MAX_MESSAGE_SIZE 250

bool serverStopped = false;
bool appRunning = true;

void EnterAndGenerateMessage(char*, char*);
bool ValidateMessage(char*);
int SelectFunction(SOCKET, char);
void PrintMenu();
void ProcessInput(char, char*);
int SendFunction(SOCKET, char*, int);
char* ReceiveFunction(SOCKET, char*);
int Connect(SOCKET);


/*Vrsi validaciju poruke koja ce da se publish-uje*/
bool ValidateMessage(char* publishMessage) {
	if (!strcmp(publishMessage, "\n")) {
		return false;
	}

	int messageLength = strlen(publishMessage);

	for (int i = 0; i < messageLength - 1; i++)
	{
		if (publishMessage[i] != ' ' && publishMessage[i] != '\t') {
			return true;
		}
	}
	return false;
}

/*Uspostavlja konekciju sa serverom*/
int Connect(SOCKET connectSocket) {
	char* connectMessage = (char*)malloc(10 * sizeof(char));
	strcpy(connectMessage, "p:Connect");

	int messageDataSize = strlen(connectMessage) + 1;
	int messageSize = messageDataSize + sizeof(int);

	MessageStruct* messageStruct = GenerateMessageStruct(connectMessage, messageDataSize);

	int retVal = SendFunction(connectSocket, (char*)messageStruct, messageSize);
	free(messageStruct);
	free(connectMessage);

	return retVal;

}

/*Salje poruku kroz socket, uveravanje da ce poruka sigurno biti poslata*/
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
		return 0;
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

/*Poruku generise u string koji se salje serveru*/
void EnterAndGenerateMessage(char* publishMessage, char* message)
{

	printf("Enter message you want to publish(max length: 250): \n");

	fgets(publishMessage, MAX_MESSAGE_SIZE, stdin);

	while (!ValidateMessage(publishMessage)) {

		printf("Message cannot be empty. Please enter your message again: \n");
		fgets(publishMessage, MAX_MESSAGE_SIZE, stdin);
	}


	if (strchr(publishMessage, '\n') == NULL) {
		int c;
		while ((c = fgetc(stdin)) != '\n' && c != EOF);
	}

	if ((strlen(publishMessage) > 0) && (publishMessage[strlen(publishMessage) - 1] == '\n'))
		publishMessage[strlen(publishMessage) - 1] = '\0';

	strcat(message, ":");
	strcat(message, publishMessage);

	printf("You published message: %s.\n", publishMessage);
}

/*Prima poruku kroz socket, uverava da ce poruka sigurno biti primljena*/
char* ReceiveFunction(SOCKET acceptedSocket, char* recvbuf) {

	int iResult;
	char* retVal = (char*)malloc(7 * sizeof(char));
	int selectResult = SelectFunction(acceptedSocket, 'r');
	if (selectResult == -1) {
		memcpy(retVal, "ErrorS", 7);
		return retVal;
	}
	iResult = recv(acceptedSocket, recvbuf, 4, 0);

	if (iResult == 0)
	{
		memcpy(retVal, "ErrorC", 7);
	}
	else if (iResult == SOCKET_ERROR)
	{
		memcpy(retVal, "ErrorR", 7);
	}
	return retVal;
}

/*Bira funkciju u neblokirajucem modu, ceka dok operacija slanje ili primanje nije slobodno*/
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

}

void PrintMenu() {
	printf("\nChoose a topic to publish to: \n");
	printf("\t1.Weather\n");
	printf("\t2.Chess\n");
	printf("\t3.Gaming\n");
	printf("\t4.Animals \n");
	printf("\t5.Food \n\n");
	printf("Press X if you want to close connection\n");
}

void ProcessInput(char input, char* message) {
	if (input == '1') {
		strcpy(message, "p:Weather");
	}
	else if (input == '2') {
		strcpy(message, "p:Chess");
	}
	else if (input == '3') {
		strcpy(message, "p:Gaming");
	}
	else if (input == '4') {
		strcpy(message, "p:Animals");
	}
	else if (input == '5') {
		strcpy(message, "p:Food");
	}
}