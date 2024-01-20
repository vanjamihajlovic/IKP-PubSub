#include "Publisher.h"

HANDLE publisherSendThread, publisherReceiveThread;
DWORD publisherSendID, publisherReceiveID;

HANDLE publisherStressSendThread;
DWORD publisherStressID;


// Function to simulate a publisher sending messages
DWORD WINAPI PublisherStressTest(LPVOID lpParam) {
	SOCKET connectSocket = *(SOCKET*)lpParam;
	int i = 0;
	while (appRunning && !serverStopped) {
		char topic[20];

		timeval interval = { 1 , 0 };
		FD_SET write;
		if (i >= 100) break;
		while (i <= 100) {
			char* poruka = (char*)malloc(270 * sizeof(char));

			FD_ZERO(&write);
			FD_SET(connectSocket, &write);
			int iResult = select(0, NULL, &write, NULL, &interval);
			if (iResult != SOCKET_ERROR && FD_ISSET(connectSocket, &write)) {
				ProcessInput('0' + (rand() % 5 + 1), poruka);

				strcat(poruka, ":");
				strcat(poruka, "Stress message");

				int messageDataSize = strlen(poruka) + 1;
				int messageSize = messageDataSize + sizeof(int);

				MessageStruct* messageStructToSend = GenerateMessageStruct(poruka, messageDataSize);
				int sendResult = SendFunction(connectSocket, (char*)messageStructToSend, messageSize);

				printf("You published message: %s.\n", poruka);

				free(messageStructToSend);
				free(poruka);
				if (sendResult == -1)
					break;
				Sleep(10);
				++i;
				printf("%d", i);
			}
		}
	}
	printf("Pritisnite ENTER da nastavite: \n");
	return 1; 
}


/*Funkcija se izvrsava i kreira u thread-u na pocetku main-a,slanje poruka ka serveru */
DWORD WINAPI PublisherSend(LPVOID lpParam) {
	SOCKET connectSocket = *(SOCKET*)lpParam;

	while (appRunning && !serverStopped) {

		PrintMenu();
		char input = _getch();

		char* message = (char*)malloc(270 * sizeof(char));

		if (input == '1' || input == '2' || input == '3' || input == '4' || input == '5') {

			ProcessInput(input, message);

			char* publishMessage = (char*)malloc(250 * sizeof(char));

			EnterAndGenerateMessage(publishMessage, message);

			int messageDataSize = strlen(message) + 1;
			int messageSize = messageDataSize + sizeof(int);

			MessageStruct* messageStructToSend = GenerateMessageStruct(message, messageDataSize);
			int sendResult = SendFunction(connectSocket, (char*)messageStructToSend, messageSize);
			free(messageStructToSend);
			free(message);
			free(publishMessage);
			if (sendResult == -1)
				break;


		}
		else if (input == 'x' || input == 'X') {
			strcpy(message, "p:shutDown");
			int messageDataSize = strlen(message) + 1;
			int messageSize = messageDataSize + sizeof(int);

			MessageStruct* messageStruct = GenerateMessageStruct(message, messageDataSize);
			int sendResult = SendFunction(connectSocket, (char*)messageStruct, messageSize);
			free(messageStruct);
			free(message);
			if (sendResult == -1) {
				break;
			}

			appRunning = false;
			closesocket(connectSocket);
			break;
		}
		else {
			printf("Invalid input.\n");
			free(message);
			continue;
		}

	}
	return 1;
}

/*Funkciju koristi thread, za slanje*/
DWORD WINAPI PublisherReceive(LPVOID lpParam) {
	int iResult = 0;
	SOCKET connectSocket = *(SOCKET*)lpParam;
	char recvbuf[DEFAULT_BUFLEN];

	while (appRunning && !serverStopped)
	{
		char* recvRes;

		recvRes = ReceiveFunction(connectSocket, recvbuf);

		if (!strcmp(recvRes, "ErrorS")) {
			closesocket(connectSocket);
			appRunning = false;
			serverStopped = true;
			free(recvRes);
			break;
		}
		else if (!strcmp(recvRes, "ErrorC"))
		{
			printf("\nConnection with server closed.\n");
			printf("Press any key to close this window . . .");
			closesocket(connectSocket);
			appRunning = false;
			serverStopped = true;
			free(recvRes);
			break;
		}
		else if (!strcmp(recvRes, "ErrorR"))
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			appRunning = false;
			serverStopped = true;
			free(recvRes);
			break;
		}
		free(recvRes);

	}

	return 1;
}

int __cdecl main(int argc, char** argv)
{
	SOCKET connectSocket = INVALID_SOCKET;
	int iResult;

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DEFAULT_PORT);

	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}


	int connectResult = Connect(connectSocket);
	if (connectResult == -1) {
		serverStopped = true;
		appRunning = false;
	}

	printf("Unesite s ako zelite da uradite stress test:");
	char stres = _getch();
	if (stres == 's' || stres == 'S') {
		publisherSendThread = CreateThread(NULL, 0, &PublisherStressTest, &connectSocket, 0, &publisherStressID);
	}
	if (publisherStressSendThread)
		WaitForSingleObject(publisherStressSendThread, INFINITE);
	SAFE_DELETE_HANDLE(publisherStressSendThread);
	printf("Stress test finished");


	publisherSendThread = CreateThread(NULL, 0, &PublisherSend, &connectSocket, 0, &publisherSendID);
	publisherReceiveThread = CreateThread(NULL, 0, &PublisherReceive, &connectSocket, 0, &publisherReceiveID);

	while (appRunning || !serverStopped) {

	}



	if (publisherSendThread)
		WaitForSingleObject(publisherSendThread, INFINITE);
	if (publisherReceiveThread)
		WaitForSingleObject(publisherReceiveThread, INFINITE);

	SAFE_DELETE_HANDLE(publisherSendThread);
	SAFE_DELETE_HANDLE(publisherReceiveThread);

	closesocket(connectSocket);

	WSACleanup();

	return 0;
}

