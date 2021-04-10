#include "RpcClient.h"
#include <assert.h>
#include <functional>
#include <iostream>

using namespace std::placeholders;

namespace tiny_rpc
{

RpcClient::RpcClient()
  : work_(ios_), socket_(ios_), timer_(ios_)
{

}

RpcClient::RpcClient(OnResultReciveCallback cb)
  : work_(ios_), socket_(ios_), timer_(ios_),
    onResultReciveCallback_(std::move(cb))
{

}

RpcClient::RpcClient(const std::string &host, unsigned short port)
  : work_(ios_), socket_(ios_), timer_(ios_),
    sockHasConnected_(false),
    host_(host), port_(port)
{
    threadPtr_ = std::unique_ptr<std::thread>(new std::thread(
        [this]() {
            ios_.run();
        }
    ));
}



RpcClient::RpcClient(OnResultReciveCallback cb, const std::string &host, unsigned short port)
  : socket_(ios_), work_(ios_), timer_(ios_),
    onResultReciveCallback_(std::move(cb)), host_(host), port_(port)
{

}

void RpcClient::connectTimeout(const boost::system::error_code& error)
{
    if(error)
    {
        if(error != boost::asio::error::operation_aborted)
            std::cout << error.message() << std::endl;
        return;
    }
    if(sockHasConnected_)
        return;
    resetSocket();
    {
        {
            std::lock_guard<std::mutex> lock(callerWakeCvMutex_);
            callerWaked_ = true;
        }
        callerWakeCv_.notify_one();
    }
}

bool RpcClient::connect(size_t timeoutMs)
{
    asynConnect();
    if(timeoutMs != 0)
    {
        timer_.expires_from_now(boost::posix_time::milliseconds(timeoutMs));
        timer_.async_wait(std::bind(&RpcClient::connectTimeout, this, _1));
    }
    {
        std::unique_lock<std::mutex> lock(callerWakeCvMutex_);
        callerWakeCv_.wait(lock, [this]() {
            return callerWaked_;
        });
    }
    return sockHasConnected_;
}


bool RpcClient::connect(const std::string &host, unsigned short port, 
                size_t timeoutMs)
{
    asynConnect(host, port);
    if(timeoutMs != 0)
    {
        timer_.expires_from_now(boost::posix_time::milliseconds(timeoutMs));
        timer_.async_wait(std::bind(&RpcClient::connectTimeout, this, _1));
    }
    {
        std::unique_lock<std::mutex> lock(callerWakeCvMutex_);
        callerWakeCv_.wait(lock);
    }
    return sockHasConnected_;
}

void RpcClient::asynConnect(const std::string &host, unsigned short port)
{
    host_ = host;
    port_ = port;
    asynConnect();
}

void RpcClient::asynConnect()
{
    auto addr = boost::asio::ip::address::from_string(host_);
    socket_.async_connect(boost::asio::ip::tcp::endpoint(addr, port_), 
                          std::bind(&RpcClient::onConnectHandle, this, _1));
}

void RpcClient::onConnectHandle(const boost::system::error_code& error)
{
    if(error)
    {
        assert(sockHasConnected_ == false);
        
        if(error != boost::asio::error::operation_aborted)
        {
            if(reconnTimes_ > 0)
            {
                reconnTimes_--;
                asynReConnect();
            }
            else
            {
                {
                    {
                        std::lock_guard<std::mutex> lock(callerWakeCvMutex_);
                        callerWaked_ = true;
                    }
                    callerWakeCv_.notify_one();
                }
            }
        }
        else
        {
            sockHasConnected_ = true;
            std::cout << "this called" << error.message() << std::endl;
            {
                {
                    std::lock_guard<std::mutex> lock(callerWakeCvMutex_);
                    callerWaked_ = true;
                }
                callerWakeCv_.notify_one();
            }
        }
    }
    else
    {
        sockHasConnected_ = true;
        {
            {
                std::lock_guard<std::mutex> lock(callerWakeCvMutex_);
                callerWaked_ = true;
            }
            callerWakeCv_.notify_one();
        }
    }
}

void RpcClient::asynReConnect()
{
    std::cout << "reconnect called" << std::endl;
    resetSocket();
    asynConnect();
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

void RpcClient::resetSocket()
{
    boost::system::error_code ignored_er;
    socket_.close(ignored_er);
    socket_ = decltype(socket_)(ios_);
    if (!socket_.is_open()) {
      socket_.open(boost::asio::ip::tcp::v4());
    }
}

}

int main(int argc, char const *argv[])
{
    tiny_rpc::RpcClient client("10.1.1.1", 8000);
    client.connect();
    client.close();
    return 0;
}


