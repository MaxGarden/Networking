#pragma once
#include "Networking/Network.h"

namespace Networking
{
    class IClient : public INetwork
    {
    public:
        virtual ~IClient() override = default;

        virtual IConnectionSharedPtr Connect(const std::string& address, unsigned short port, size_t timeoutInMiliSeconds = 2000u) = 0;
    };
}