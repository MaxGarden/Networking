#pragma once
#include "Network.h"

namespace Networking
{
    using ConnectionID = size_t;

    class IConnection
    {
    public:
        using OnReceivedCallback = std::function<void(const Payload&)>;

    public:
        virtual ~IConnection() = 0;

        virtual bool IsConnected() const noexcept = 0;
        virtual const std::string& GetAddress() const noexcept = 0;

        virtual void SetOnReceivedCallback(OnReceivedCallback&& callback) = 0;

        virtual bool Send(const Payload& data) = 0;
        virtual void Close() = 0;
    };
}