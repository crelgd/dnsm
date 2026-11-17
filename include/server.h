#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include "FileRecord.h"

#define DEFAULT_BUFLEN 512
#define PORT 53

#define CONFIG_FILE "config.ini"

class RequestOUT
{
public:
    std::vector<char> RequestDNS(char* request, size_t request_size);

private:
    int Configure();
    int Close();
    int Shutdown();

private:
    SOCKET Socket;
};

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
    SOCKET Socket;

    char RequestBfr[DEFAULT_BUFLEN] = {};

    int listenVAL;

    fsdns::DNSRecord file;
    std::vector<DNSRECORD> crecord;

    RequestOUT requestS;
};
