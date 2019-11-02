#pragma once
#include "Network.h"
#include "Connection.h"

namespace Networking
{
    class IWebSocketClient : public INetwork
    {
    public:
        virtual ~IWebSocketClient() override = default;

        virtual IConnectionSharedPtr Connect(const std::string& url) = 0;
    };
}