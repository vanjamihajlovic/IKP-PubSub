#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "..\Common\SocketOperations.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"
#define SERVER_SLEEP_TIME 50
#define NUMBER_OF_CLIENTS 40
#define INV_SOCKET 3435973836

bool appRunning = true;
int clientsCount = 0;
int numberOfPublishers = 0;

int numberOfConnectedSubs = 0;
int numberOfSubscribedSubs = 0;
ThreadArgument publisherThreadArgument;
ThreadArgument subscriberThreadArgument;

int SelectFunction(SOCKET, char);
void Subscribe(struct Queue*, SOCKET, char*);
void Publish(struct MessageQueue*, char*, char*,int);
void SubscriberShutDown(Queue*, SOCKET, struct Subscriber subscribers[]);
char* ReceiveFunction(SOCKET, char* );
int SendFunction(SOCKET, char*, int);
char Connect(SOCKET);
void AddTopics(Queue*);




void AddTopics(Queue* queue) {

	Enqueue(queue, "Weather");
	Enqueue(queue, "Chess");
	Enqueue(queue, "Gaming");
	Enqueue(queue, "Animals");
	Enqueue(queue, "Food");
}


char Connect(SOCKET acceptedSocket) {
	char recvbuf[DEFAULT_BUFLEN];
	char *recvRes;
	
	recvRes = ReceiveFunction(acceptedSocket, recvbuf);

	if (strcmp(recvRes, "ErrorC") && strcmp(recvRes, "ErrorR"))
	{
		char delimiter[] = ":";
	
		char *ptr = strtok(recvRes, delimiter);

		char *role = ptr;
		ptr = strtok(NULL, delimiter);

		if (!strcmp(role, "s")) {
			
			subscriberThreadArgument.ordinalNumber = numberOfConnectedSubs;
			subscriberThreadArgument.socket = acceptedSocket;
			subscriberThreadArgument.clientNumber = clientsCount;
			
			printf("\nSubscriber %d connected.\n", numberOfConnectedSubs+1);
			
			free(recvRes);
			return 's';
		}

		if (!strcmp(role, "p")) {
			publisherThreadArgument.ordinalNumber = numberOfPublishers;
			publisherThreadArgument.socket = acceptedSocket;
			publisherThreadArgument.clientNumber = clientsCount;
			
			printf("\nPublisher %d connected.\n", numberOfPublishers+1);
			
			free(recvRes);
			return 'p';
		}
		
	}
	else if (!strcmp(recvRes, "ErrorC"))
	{
		printf("\nConnection with client closed.\n");
		closesocket(acceptedSocket);
	}
	else if (!strcmp(recvRes, "ErrorR"))
	{
		printf("\nrecv failed with error: %d\n", WSAGetLastError());
		closesocket(acceptedSocket);
	}
	free(recvRes);
}




int SendFunction(SOCKET connectSocket, char* message, int messageSize) {

	int selectResult = SelectFunction(connectSocket, 'w');
	if (selectResult == -1) {
		return -1;
	}
	int iResult = send(connectSocket, message, messageSize, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("\nsend failed with error: %d\n", WSAGetLastError());
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
		return(myBuffer);
		
}


void SubscriberShutDown(Queue* queue, SOCKET acceptedSocket, struct Subscriber subscribers[]) {
	for (int i = 0; i < queue->size; i++)
	{
		for (int j = 0; j < queue->array[i].size; j++)
		{
			if (queue->array[i].subsArray[j] == acceptedSocket) {
				int size = queue->array[i].size;
				SOCKET temp = queue->array[i].subsArray[size];
				if (temp != INV_SOCKET) {
					queue->array[i].subsArray[size] = INV_SOCKET;
					queue->array[i].subsArray[j] = temp;
					queue->array[i].size--;
				}
				else {
					queue->array[i].subsArray[j] = INV_SOCKET;
					queue->array[i].size--;
				}
				
			}
		}

	}

	for (int i = 0; i < numberOfSubscribedSubs; i++)
	{
		if (subscribers[i].socket == acceptedSocket) {
			subscribers[i].socket = 0;
			SAFE_DELETE_HANDLE(subscribers[i].hSemaphore);
			subscribers[i].hSemaphore = 0;
		}
	}
}


void Subscribe(struct Queue* queue, SOCKET sub, char* topic) {
	for (int i = 0; i < queue->size; i++) {
		if (!strcmp(queue->array[i].topic, topic)) {
			int index = queue->array[i].size;
			queue->array[i].subsArray[index] = sub;
			queue->array[i].size++;
		}
	}
}


void Publish(struct MessageQueue* messageQueue, char* topic, char* message,int ordinalNumber) {
	
	struct TopicMessage item;
	memcpy(item.message, message, strlen(message)+1);
	memcpy(item.topic, topic, strlen(topic)+1);

	EnqueueMessageQueue(messageQueue, item);

	printf("\nPublisher %d published message: %s to topic: %s\n",ordinalNumber+1, item.message, item.topic);

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

		if (!appRunning)
			return -1;

		if (rw == 'r') {
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		}
		else {
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		}


		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "\nselect failed with error: %ld\n", WSAGetLastError());
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