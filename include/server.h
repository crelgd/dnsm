#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_BUFLEN 512
#define PORT 50009

class DNSSrv
{
public:
    DNSSrv(unsigned short ListeningPort);
    ~DNSSrv();

    int SocketInit();
    int Runtime();
private:
    int Listen();

private:
    WSADATA wsadata;
    int iResult;
    sockaddr_in RecvAddr;
    sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);
    int RecvAddrSize = sizeof(RecvAddr);
    SOCKET Socket;

    char RequestBfr[DEFAULT_BUFLEN] = {};
};