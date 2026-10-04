#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <cstdlib>
#include <vector>

#define DEFAULT_BUFLEN 512

unsigned char ToByte(std::vector<bool>& bit, size_t size)
{   
    if (size > 8)
    {
        return NULL;
    }

    unsigned char result = 0x00;

    for (int i = 0; i < size; i++)
    {
        /* printf("OFFSET = %d\n", size - i - 1); */

        if (bit[i] > 0)
        {
            result += pow(2, size-i - 1);
            /* printf("RESULT += 2^%d = %d | 2^%d = %d\n", size-i-1, result, size-i-1, pow(2, size-i- 1)); */
        }
    }

    return result;
}

std::vector<unsigned char> GenByte(bool* bits, size_t size)
{
    std::vector<unsigned char> bytes;
    std::vector<bool> bit;
    int track = 0;
 
    for (int i = 0; i < size; i++)
    {
        bit.push_back(bits[i]);

        if (bit.size() >= 8)
        {
            bytes.push_back(ToByte(bit, 8));
            bit.clear();
        }
    }

    return bytes;
}

/* int GenFrame()
{

    
} */

int main(int argc, char* argv[])
{
    WSADATA wsadata;
    int iResult;
    unsigned short Port = 50009;

    sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);

    bool request[] = {
        0,0,0,1,1,1,0,0,0,1,1,0,1,1,0,1,
        0,            // QR
        0,0,0,0,      //Opcode
        0,            // AA
        0,            // TC
        1,            // RD
        0,            // RA
        0,0,0,        // Z
        0,0,0,0,      // RCODE
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1, //QDCOUND
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

        0,0,        // QNAME
        0,0,0,1,1,0,    // google
        0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1,
        0,0,
        0,0,0,0,1,1,    // com
        0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1,

        0,0,0,0,0,0,0,0,

        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,1,  // QTYPE
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,1,  // IN
    };

    std::vector<unsigned char> byte = GenByte(request, sizeof(request));

    printf("\n");

    if (argc < 4)
    {
        printf("<ip> <port>");
        return 0;
    }

    iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    sockaddr_in addr;

    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    int AddrSize = sizeof(addr);

    SOCKET Socket = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    int recvbuflen = DEFAULT_BUFLEN;

    char recvbuf[DEFAULT_BUFLEN];

    char sendbuf[DEFAULT_BUFLEN];
    for (int i = 0; i < byte.size(); i++)
    {
        sendbuf[i] = byte[i];
    }

    for (int i  = 0; i < byte.size(); i++)
    {
        printf("%02X", sendbuf[i]);
    }

    // Send an initial buffer
    iResult = sendto(Socket, sendbuf, byte.size(), 0, (SOCKADDR*)&addr, AddrSize);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(Socket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    do {
        iResult = recvfrom(Socket, recvbuf, DEFAULT_BUFLEN, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);
            for (int i = 0; i < iResult; i++)
            {
                printf("%02X", recvbuf[i]);
            }
        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed: %d\n", WSAGetLastError());
    } while (iResult > 0);

    printf("Closing socket\n");
    iResult = closesocket(Socket);
    if (iResult == SOCKET_ERROR)
    {
        printf("Closing socket error\n");
        return 1;
    }

    printf("Exiting");
    WSACleanup();

    return 0;
}

    