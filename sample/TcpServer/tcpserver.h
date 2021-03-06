/****************************************************************************

Creator: liyangyao@gmail.com
Date: 2015/7/29

****************************************************************************/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QDebug>
#include <unordered_map>
#include <queue>
#include "libuvpp.h"


class LoopEx:public Loop
{
public:
    explicit LoopEx():
        Loop()
    {
        m_async.reset(new Async(this, std::bind(&LoopEx::onFunctor, this)));
    }

    void queueInLoop(const Callback &functor)
    {
        MutexLocker lock(&m_mutex);
        m_functors.push(functor);
        m_async->send();
    }

private:
    std::queue<Callback> m_functors;
    std::unique_ptr<Async> m_async;
    Mutex m_mutex;
    void onFunctor()
    {
        while (true)
        {
            Callback fun = nullptr;
            {
                MutexLocker lock(&m_mutex);
                if (!m_functors.empty())
                {
                    fun.swap(m_functors.front());
                    m_functors.pop();
                }
            }

            if (fun)
            {
                fun();
            }
            else{
                break;
            }
        }
    }
    DISABLE_COPY(LoopEx)
};

class Connection:public std::enable_shared_from_this<Connection>
{
public:
    typedef std::shared_ptr<Connection> Ptr;
    typedef std::function<void(const Ptr &)> Callback;
    typedef std::function<void(const Ptr &, const QByteArray &)> MessageCallback;

    explicit Connection(Loop *loop):
        m_tcp(loop)
    {
        static int id = 0;
        id++;
        m_id = id;
        qDebug()<<"Connection"<<m_id;

        m_tcp.setCloseCallback(std::bind(&Connection::handleClose, this));

    }

    ~Connection()
    {
        qDebug()<<"~Connection"<<m_id;
    }

    int id()
    {
        return m_id;
    }

    void shutdown()
    {
        m_tcp.shutdown();
    }

    void forceClose()
    {
        handleClose();
    }

    void write(const QByteArray &d)
    {
        m_tcp.write(d, nullptr);
    }

    MessageCallback messageCallback;
    std::shared_ptr<void> context;

private:
    void connectionEstablished()
    {
        m_tcp.read_start(std::bind(&Connection::handleRead, this, std::placeholders::_1));
    }

    void connectionDestroyed()
    {
        qDebug()<<"connectionDestroyed";
    }

    Callback closeCallback;
    friend class TcpServer;

private:
    Tcp m_tcp;
    int m_id;
    void handleClose()
    {
        //qDebug()<<"handleClose";
        Ptr guardThis(shared_from_this());
        closeCallback(guardThis);
    }

    void handleRead(const QByteArray &data)
    {
        //qDebug()<<"handleRead:"<<data;
        if (messageCallback)
        {
            messageCallback(shared_from_this(), data);
        }
    }
    DISABLE_COPY(Connection)
};

typedef std::function<void(const Connection::Ptr &)> ConnectionCallback;



class TcpServer
{
public:
    explicit TcpServer(LoopEx* loop):
        m_loop(loop),
        m_tcpServer(loop)
    {

    }
    bool listen(const char *ip, int port)
    {
        m_tcpServer.bind(ip, port);
        return m_tcpServer.listen(std::bind(&TcpServer::onNewConnection, this, std::placeholders::_1))>=0;
    }

    ConnectionCallback connectionCallback;
    ConnectionCallback connectionCloseCallback;
    Connection::MessageCallback messageCallback;
private:
    LoopEx* m_loop;
    Tcp m_tcpServer;
    std::unordered_map<int, Connection::Ptr> m_connections;

    void onNewConnection(int status)
    {
        Connection::Ptr connection(new Connection(m_loop));
        m_tcpServer.accept(&connection->m_tcp);
        connection->connectionEstablished();

        m_connections.insert(std::make_pair(connection->id(), connection));
        connection->closeCallback = std::bind(&TcpServer::onConnectionClose, this, std::placeholders::_1);
        connection->messageCallback = messageCallback;
        //qDebug()<<"onNewConnection";
        if (connectionCallback)
        {
            connectionCallback(connection);
        }
    }

    void onConnectionClose(const Connection::Ptr &connection)
    {
        m_loop->queueInLoop(std::bind(&Connection::connectionDestroyed, connection));
        m_connections.erase(connection->id());
        //qDebug()<<"onConnectionClose";
        if (connectionCloseCallback)
        {
            connectionCloseCallback(connection);
        }
    }
    DISABLE_COPY(TcpServer)
};

#endif // TCPSERVER_H
