/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

class WebServer {
public:
    // 构造函数，用于初始化 WebServer 类的各个成员变量
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    // 析构函数，用于释放资源
    ~WebServer();

    // 启动 Web 服务器
    void Start();

private:
    // 初始化 socket，返回是否成功
    bool InitSocket_(); 

    // 初始化事件模式
    void InitEventMode_(int trigMode);

    // 添加客户端连接
    void AddClient_(int fd, sockaddr_in addr);

    // 处理监听事件
    void DealListen_();

    // 处理写事件
    bool DealWrite_(HttpConn* client, int p);

    // 处理读事件
    bool DealRead_(HttpConn* client, int p);

    // 发送错误信息
    void SendError_(int fd, const char*info);

    // 延长客户端连接的时间
    void ExtentTime_(HttpConn* client);

    // 关闭连接
    void CloseConn_(HttpConn* client);

    // 读事件处理函数
    void OnRead_(HttpConn* client);

    // 写事件处理函数
    void OnWrite_(HttpConn* client);

    // 主处理函数
    void OnProcess(HttpConn* client);

    // 最大文件描述符数量
    static const int MAX_FD = 65536;

    // 设置文件描述符为非阻塞模式
    static int SetFdNonblock(int fd);

    // 服务器端口
    int port_;

    // 是否开启延迟关闭连接
    bool openLinger_;

    // 超时时间（毫秒）
    int timeoutMS_;  

    // 是否关闭服务器
    bool isClose_;

    // 监听的文件描述符
    int listenFd_;

    // 服务器资源目录
    char* srcDir_;

    // 监听事件类型
    uint32_t listenEvent_;

    // 连接事件类型
    uint32_t connEvent_;

    // 堆定时器
    std::unique_ptr<HeapTimer> timer_;

    // 线程池
    std::unique_ptr<ThreadPool> threadpool_;

    // Epoll 对象
    std::unique_ptr<Epoller> epoller_;

    // 存储客户端连接的哈希表
    std::unordered_map<int, HttpConn> users_;
};

#endif //WEBSERVER_H