#include <stdio.h>
#include "server.h"

DNSSrv::DNSSrv(unsigned short ListeningPort)
{
    iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
    }

    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    RecvAddr.sin_port = htons(ListeningPort);
    RecvAddr.sin_family = AF_INET;
}

DNSSrv::~DNSSrv()
{
    printf("Closing socket\n");
    iResult = closesocket(Socket);
    if (iResult == SOCKET_ERROR)
    {
        printf("Closing socket error\n");
    }

    printf("Exiting");
    WSACleanup();
}

int DNSSrv::SocketInit()
{
    Socket = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    iResult = bind(Socket, (SOCKADDR*) &RecvAddr, sizeof(RecvAddr));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    return 0;
}

int DNSSrv::Listen()
{
    int listen = recvfrom(Socket, RequestBfr, sizeof(RequestBfr), 
        NULL, (SOCKADDR*) &SenderAddr, &SenderAddrSize);
    if (listen == SOCKET_ERROR)
    {
        printf("Datagram error\n");
        return 1;
    }

    for (int i = 0; i < listen; i++)
    {
        printf("%c", RequestBfr[i]);
    }
    printf("\n");

    return 0;
}

int DNSSrv::Runtime()
{
    Listen();

    

    return 0;
}


