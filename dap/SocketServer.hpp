#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "Socket.hpp"
#include <string>

using namespace std;
namespace dap
{
class SocketServer : public Socket
{
public:
    SocketServer();
    virtual ~SocketServer();

protected:
    /**
     * @throw clSocketException
     * @return port number
     */
    int CreateServer(const string& address, int port);

public:
    /**
     * @brief Create server using connection string
     * @return port number on success
     * @throw clSocketException
     */
    int Start(const string& connectionString);
    Socket::Ptr_t WaitForNewConnection(long timeout);
    /**
     * @brief same as above, however, return a pointer to the connection that should be freed by the caller
     */
    Socket* WaitForNewConnectionRaw(long timeout);
};
};     // namespace dap
#endif // CLSOCKETSERVER_H
