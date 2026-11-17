#include "server.h"

std::vector<char> RequestOUT::RequestDNS(char* request, size_t request_size)
{
    if (request_size < 0)
        return {};

    std::vector<char> response(2048);

    if (Configure() == 1)
        return {};

    HRESULT iResult = send(Socket, request, (int)request_size, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(Socket);
        WSACleanup();
        return {};
    }

    iResult = recv(Socket, response.data(), response.size(), 0);

    if (Close() == 1)
        return {};

    return response;
}

int RequestOUT::Configure()
{
    addrinfo  sockddr = {};
    PADDRINFOA result;

    sockddr.ai_family = AF_INET;
    sockddr.ai_protocol = IPPROTO_UDP;
    sockddr.ai_socktype = SOCK_DGRAM;

    HRESULT iResult;

    iResult = getaddrinfo("1.1.1.1", "53", &sockddr, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    auto ptr = result;

    Socket = socket(
        ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol
    );

    if (Socket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = connect(Socket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(Socket);
        Socket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (Socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    return 0;
}

int RequestOUT::Shutdown()
{
    HRESULT iResult = shutdown(Socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(Socket);
        WSACleanup();
        return 1;
    }
}

int RequestOUT::Close()
{
    closesocket(Socket);

    return 0;
}