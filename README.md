# Networking
## Budowanie
Wymagane jest zainstalowane oprogramowanie cmake. 
Aby wygenerować projekt dla systemu:
 - Windows
należy użyć skryptu **generate_vs_2019_x64_project.bat**.
 - MacOS
należy użyć skryptu **generate_xcode_project**.
## Interfejs reprezentujący połączenie
```
namespace Networking
{
    using ConnectionID = size_t;

    class IConnection
    {
    public:
        using OnClosedCallback = std::function<void()>;
        using OnReceivedCallback = std::function<void(const Payload&)>;

    public:
        virtual ~IConnection() = default;

        virtual bool IsConnected() const noexcept = 0;
        virtual const std::string& GetAddress() const noexcept = 0;

        virtual void SetOnClosedCallback(OnClosedCallback&& callback) = 0;
        virtual void SetOnReceivedCallback(OnReceivedCallback&& callback) = 0;

        virtual bool Send(const Payload& data) = 0;
        virtual void Close() = 0;
    };
}
```

Udostępnia metody umożliwiające:
- sprawdzenie stanu połączenia:
```
virtual bool IsConnected() const noexcept
```
- pobranie adresu
```
virtual const std::string& GetAddress() const noexcept 
```
- ustawienie wywołań zwrotnych na zamknięcie połącznienia lub jego zamknięcie:
```
virtual void SetOnReceivedCallback(OnReceivedCallback&& callback)
```
- wysłanie danych
```
virtual bool Send(const Payload& data)
```
- zamknięcie połączenia
```
virtual void Close()
```

## Interfejs zawierający metode wspólne dla serwera oraz klienta
```
namespace Networking
{
    using Payload = std::vector<byte>;

    class INetwork
    {
    public:
        using OnConnectionClosedCallback = std::function<void(IConnectionSharedPtr connection)>;
        
    public:
        virtual ~INetwork() = default;

        virtual bool Initialize() = 0;
        virtual void Finalize() = 0;

        virtual void SetOnConnectionClosedCallback(OnConnectionClosedCallback&& callback) = 0;

        virtual bool Send(const IConnectionSharedPtr& connection, const Payload& data) = 0;
        virtual void CloseConnection(const IConnectionSharedPtr& connection) = 0;

        virtual void Update() = 0;
    };
}
```

Udostępnia metody umożliwiające:
- iniicjalizację
```
virtual bool Initialize()
```
- finalizację
```
virtual bool Finalize()
```
- ustawienie wywołania zwrotnego na zakończenie połączenia należacego do serwera/klienta
```
virtual void SetOnConnectionClosedCallback(OnConnectionClosedCallback&& callback)
```
- wysłanie danych
```
virtual bool Send(const IConnectionSharedPtr& connection, const Payload& data)
```
- zamknięcie połączenia
```
virtual void CloseConnection(const IConnectionSharedPtr& connection)
```
- aktualizację
```
virtual void Update()
```

## Interfejs reprezentujący serwer:
```
namespace Networking
{
    class IServer : public INetwork
    {
    public:
        using OnClientConnectedCallback = std::function<void(IConnectionSharedPtr connection)>;

    public:
        virtual ~IServer() = default;

        virtual void SetOnClientConnectedCallback(OnClientConnectedCallback&& callback) = 0;
    };
}
```
Udostępnia metodę umożliwiającą ustawienie wywołania zwrotnego na zakończenie połączenie przez klienta
```
virtual void SetOnClientConnectedCallback(OnClientConnectedCallback&& callback)
```

## Interfejs reprezentujący klienta
```
namespace Networking
{
    class IClient : public INetwork
    {
    public:
        virtual ~IClient() override = default;

        virtual IConnectionSharedPtr Connect(const std::string& address, unsigned short port, size_t timeoutInMiliSeconds = 2000u) = 0;
    };
}
```
Udostępnia metodę umożliwiająca połączenie się z serwerem
```
virtual IConnectionSharedPtr Connect(const std::string& address, unsigned short port, size_t timeoutInMiliSeconds = 2000u)
```
