#pragma once
#include "Connection.h"

namespace Networking
{
    class IConnectionInternal : public IConnection
    {
    public:
        static IConnectionSharedPtr Create(INetwork& network, const std::string& address);
    };
}