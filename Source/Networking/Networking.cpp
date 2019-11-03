#include "pch.h"
#include "Networking/Networking.h"
#if defined(_WIN32)
#include <WinSock2.h>
#endif

bool Networking::Initialize()
{
#if defined(_WIN32)
    WSADATA wsaData;

    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#endif

    return true;
}

void Networking::Finalize()
{
#if defined(_WIN32)
    WSACleanup();
#endif
}