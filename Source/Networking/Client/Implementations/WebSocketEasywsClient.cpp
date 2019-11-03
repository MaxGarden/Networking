#include "pch.h"
#include "Networking/Client/WebSocketClient.h"
#include "ConnectionInternal.h"
#include "easywsclient/easywsclient.hpp"

using namespace Networking;

class WebSocketEasywsClient final : public IWebSocketClient
{
public:
    WebSocketEasywsClient() = default;
    virtual ~WebSocketEasywsClient() override final;

    virtual bool Initialize() override final;
    virtual void Finalize() override final;

    virtual IConnectionSharedPtr Connect(const std::string& address, unsigned short port, size_t timeoutInMiliSeconds = 2000u) override final;

    virtual void SetOnConnectionClosedCallback(OnConnectionClosedCallback&& callback) override final;

    virtual bool Send(const IConnectionSharedPtr& connection, const Payload& data) override final;
    virtual void CloseConnection(const IConnectionSharedPtr& connection) override final;

    virtual void Update() override final;

private:
    struct ConnectionData
    {
        IConnectionInternalSharedPtr Connection;
        std::unique_ptr<easywsclient::WebSocket> WebSocket;
    };

private:
    std::map<ConnectionID, ConnectionData> m_connections;
    OnConnectionClosedCallback m_onCloseConnectionCallback;
};

WebSocketEasywsClient::~WebSocketEasywsClient()
{
    Finalize();
}

bool WebSocketEasywsClient::Initialize()
{
    return true;
}

void WebSocketEasywsClient::Finalize()
{
    std::vector<IConnectionInternalSharedPtr> connections;
    connections.reserve(m_connections.size());

    for (const auto& pair : m_connections)
        connections.emplace_back(pair.second.Connection);

    for (const auto& connection : connections)
        CloseConnection(connection);
}

static std::string BuildUrl(const std::string& address, unsigned short port)
{
    char buffer[64];
#ifdef _MSC_VER
    sprintf_s(buffer, "ws://%s:%d/", address.c_str(), port);
#else
    sprintf(buffer, "ws://%s:%d/", address.c_str(), port);
#endif
    return std::string{ buffer };
}

IConnectionSharedPtr WebSocketEasywsClient::Connect(const std::string& address, unsigned short port, size_t timeoutInMiliSeconds)
{
    const auto url = BuildUrl(address, port);

    ConnectionData connectionData;
    connectionData.WebSocket.reset(easywsclient::WebSocket::from_url(url, std::string{}, timeoutInMiliSeconds));

    if (!connectionData.WebSocket)
        return nullptr;

    connectionData.Connection = IConnectionInternal::Create(*this, address);
    const auto& connection = connectionData.Connection;

    if (m_connections.find(connection->GetID()) != m_connections.end())
    {
        NETWORKING_ASSERT(false && "Id of connection must be unique");
        return nullptr;
    }

    const auto connectionID = connection->GetID();
   
    const auto pair = m_connections.insert(std::pair<ConnectionID, ConnectionData>(connectionID, std::move(connectionData)));
    return pair.first->second.Connection;
}

void WebSocketEasywsClient::SetOnConnectionClosedCallback(OnConnectionClosedCallback&& callback)
{
    NETWORKING_ASSERT(!m_onCloseConnectionCallback);
    m_onCloseConnectionCallback = std::move(callback);
}

bool WebSocketEasywsClient::Send(const IConnectionSharedPtr& connection, const Payload& data)
{
    if (!connection)
        return false;

    const auto connectionID = static_cast<IConnectionInternal*>(connection.get())->GetID();
    const auto iterator = m_connections.find(connectionID);

    if (iterator == m_connections.end())
        return false;

    if (const auto& socket = iterator->second.WebSocket)
    {
        socket->sendBinary(data);
        socket->poll();
        return true;
    }

    return false;
}

void WebSocketEasywsClient::CloseConnection(const IConnectionSharedPtr& connection)
{
    if (!connection)
        return;

    const auto connectionID = static_cast<IConnectionInternal*>(connection.get())->GetID();
    const auto iterator = m_connections.find(connectionID);
    if (iterator == m_connections.end())
        return;

    if (const auto& socket = iterator->second.WebSocket)
        socket->close();
}

void WebSocketEasywsClient::Update()
{
    for (auto iterator = m_connections.begin(); iterator != m_connections.end();)
    {
        const auto& socket = iterator->second.WebSocket;
        const auto& connection = iterator->second.Connection;

        if (!socket || !connection)
            continue;

        socket->poll();
        socket->dispatchBinary([&connection](const std::vector<uint8_t>& message)
        {
            connection->OnReceived(message);
        });

        if (socket->getReadyState() == easywsclient::WebSocket::CLOSED)
        {
            connection->OnClosed();

            if (m_onCloseConnectionCallback)
                m_onCloseConnectionCallback(connection);

            iterator = m_connections.erase(iterator);
        }
        else
            ++iterator;
    }
}

IWebSocketClientUniquePtr IWebSocketClient::CreateEasyWs()
{
    return std::make_unique<WebSocketEasywsClient>();
}