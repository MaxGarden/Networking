#include "pch.h"
#include "ConnectionInternal.h"

using namespace Networking;

class ConnectionInternal final : public IConnectionInternal, public std::enable_shared_from_this<ConnectionInternal>
{
public:
    ConnectionInternal(INetwork& network, const std::string& address);
    virtual ~ConnectionInternal() override final = default;

    virtual bool IsConnected() const noexcept override final;
    virtual const std::string& GetAddress() const noexcept override final;
    virtual const ConnectionID& GetID() const noexcept override final;

    virtual void SetOnClosedCallback(OnClosedCallback&& callback) override final;
    virtual void SetOnReceivedCallback(OnReceivedCallback&& callback) override final;

    virtual bool Send(const Payload& data) override final;
    virtual void Close() override final;

    virtual void OnClosed() override final;
    virtual void OnReceived(const Payload& data) override final;

private:
    INetwork& m_network;
    const std::string m_address;
    const ConnectionID m_id;

    bool m_isConnected = true;

    OnClosedCallback m_onClosedCallback;
    OnReceivedCallback m_onReceivedCallback;
}; 

static ConnectionID GetFreeConnectionID()
{
    static ConnectionID id = 0;
    return id++;
}

ConnectionInternal::ConnectionInternal(INetwork& network, const std::string& address) :
    m_network{ network },
    m_address{ address },
    m_id{ GetFreeConnectionID() }
{
}

bool ConnectionInternal::IsConnected() const noexcept
{
    return m_isConnected;
}

const std::string& ConnectionInternal::GetAddress() const noexcept
{
    return m_address;
}

const ConnectionID& ConnectionInternal::GetID() const noexcept
{
    return m_id;
}

void ConnectionInternal::SetOnClosedCallback(OnClosedCallback&& callback)
{
    NETWORKING_ASSERT(!m_onClosedCallback);
    m_onClosedCallback = std::move(callback);
}

void ConnectionInternal::SetOnReceivedCallback(OnReceivedCallback&& callback)
{
    NETWORKING_ASSERT(!m_onReceivedCallback);
    m_onReceivedCallback = std::move(callback);
}

bool ConnectionInternal::Send(const Payload& data)
{
    if (!IsConnected())
        return false;

    return m_network.Send(shared_from_this(), data);
}

void ConnectionInternal::Close()
{
    if (!m_isConnected)
        return;

    m_network.CloseConnection(shared_from_this());
}

void ConnectionInternal::OnClosed()
{
    m_isConnected = false;
    if (m_onClosedCallback)
        m_onClosedCallback();
}

void ConnectionInternal::OnReceived(const Payload& data)
{
    NETWORKING_ASSERT(m_onReceivedCallback);
    if (m_onReceivedCallback)
        m_onReceivedCallback(data);
}

IConnectionInternalSharedPtr IConnectionInternal::Create(INetwork& network, const std::string& address)
{
    return std::make_shared<ConnectionInternal>(network, address);
}