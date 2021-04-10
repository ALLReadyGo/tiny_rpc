#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

#pragma once
namespace tiny_rpc
{

class RpcClient : public boost::noncopyable
{
  public:
    using OnResultReciveCallback = std::function<void(long, const std::string &)>;
    
    RpcClient();

    explicit RpcClient(OnResultReciveCallback cb);

    RpcClient(const std::string &host, unsigned short port);
    
    RpcClient(OnResultReciveCallback cb, const std::string &host, unsigned short port);

    bool connect(size_t timeoutMs = 0);

    bool connect(const std::string &host, unsigned short port, 
                 size_t timeout = 0);
    
    void asynConnect(const std::string &host, unsigned short port);

    void close()
    {
        resetSocket();
        ios_.stop();
        threadPtr_->join();
    }

  private:

    void asynConnect();
    void asynReConnect();
    void resetSocket();
    void onConnectHandle(const boost::system::error_code& error);
    void connectTimeout(const boost::system::error_code& error);
    
    boost::asio::io_service ios_;
    boost::asio::io_context::work work_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::deadline_timer timer_;
    
    std::unique_ptr<std::thread> threadPtr_;
    std::atomic<bool> sockHasConnected_;

    bool callerWaked_ = false;
    std::mutex callerWakeCvMutex_;
    std::condition_variable callerWakeCv_;

    int reconnTimes_ = 0;      // 0 不重连， N 重连N次
    int reconnIntervalMs_ = 1000;
    std::string host_;
    unsigned short port_;
    int maxTimeout_;
    OnResultReciveCallback onResultReciveCallback_;
};



}