#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

/* 基于muduo网络库开发服务器程序
1. 组合TcpServer对象
2. 创建EventLoop事件循环对象的指针
3. 明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4. 在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写事件的回调函数
5. 设置合适的服务端线程数量
*/

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr, const string &nameArg)
        : m_server(loop, listenAddr, nameArg), m_loop(loop)
    {
        // 给服务器注册用户链接的创建和断开回调
        m_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        // 给服务器注册用户读写事件的回调
        m_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
        // 设置服务器端的线程数量 1个I/O线程 三个worker线程
        m_server.setThreadNum(4);
    }
    // 开启事件循环
    void start()
    {
        m_server.start();
    }

private:
    // 专门处理用户的连接创建和断开 epoll listenfd accept
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << " state:online" << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << " state:offline" << endl;
            conn->shutdown();
            // m_loop->quit();
        }
    }
    // 专门处理用户的读写事件
    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << " time:" << time.toString() << endl;
        conn->send(buf);
    }
    TcpServer m_server;
    EventLoop *m_loop;
};

int main(){
    EventLoop loop; //epoll
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");

    server.start(); //listenfd  epoll_ctl=>epoll
    loop.loop(); //epoll_wait 以阻塞方式等待新用户连接，已连接用户的读写事件等
    return 0;
}