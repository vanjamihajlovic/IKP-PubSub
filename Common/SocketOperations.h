

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "Queue.h"


#define SAFE_DELETE_HANDLE(h) {if(h)CloseHandle(h);}

MessageStruct* GenerateMessageStruct(char* message, int len) {

	MessageStruct* messageStruct = (MessageStruct *)(malloc(sizeof(MessageStruct)));

	messageStruct->header = len;
	memcpy(messageStruct->message, message, len);

	return messageStruct;

}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}