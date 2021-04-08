#include <iostream>
#include <string>
#include <exception>
#include <string_view>
#include <assert.h>
#include "SeriaBuffer.h"
#include <cstring>

namespace tiny_rpc
{

SeriaBuffer::SeriaBuffer()
  : tail_(buffer_), 
    head_(buffer_)
{
}

void SeriaBuffer::appendItem(int32_t item)
{
    if(head_ == tail_)
    {
        append('<');
        append(item);
        append('>');
    }
    else
    {
        assert(*(tail_ - 1) == '>');
        *(tail_ - 1) = ',';
        append(item);
        append('>');
    }
}

void SeriaBuffer::appendItem(int64_t item)
{
    if(head_ == tail_)
    {
        append('<');
        append(item);
        append('>');
    }
    else
    {
        assert(*(tail_ - 1) == '>');
        *(tail_ - 1) = ',';
        append(item);
        append('>');
    }
}

void SeriaBuffer::appendItem(double item)
{
    if(head_ == tail_)
    {
        append('<');
        append(item);
        append('>');
    }
    else
    {
        assert(*(tail_ - 1) == '>');
        *(tail_ - 1) = ',';
        append(item);
        append('>');
    }
}

void SeriaBuffer::appendItem(const char *src, size_t len)
{
    if(head_ == tail_)
    {
        append("<\"", 2);
        append(src, len);
        append("\">", 2);
    }
    else
    {
        assert(*(tail_ - 1) == '>');
        *(tail_ - 1) = ',';
        append("\"", 1);
        append(src, len);
        append("\">", 2);
    }
}

void SeriaBuffer::appendItem(const std::string & item)
{
    if(head_ == tail_)
    {
        append("<\"", 2);
        append(item.c_str(), item.size());
        append("\">", 2);
    }
    else
    {
        assert(*(tail_ - 1) == '>');
        *(tail_ - 1) = ',';
        append("\"", 1);
        append(item.c_str(), item.size());
        append("\">", 2);
    }
}

void SeriaBuffer::append(int32_t value)
{
    char local_buffer[32];
    promiseEnoughSpace(32);
    sprintf(local_buffer, "%d", value);
    int value_str_len = strlen(local_buffer);
    memcpy(tail_, local_buffer, value_str_len);
    tail_ += value_str_len;
    len_ += value_str_len;
}

void SeriaBuffer::append(int64_t value)
{
    char local_buffer[32];
    promiseEnoughSpace(32);
    sprintf(local_buffer, "%ld", value);
    int value_str_len = strlen(local_buffer);
    memcpy(tail_, local_buffer, value_str_len);
    tail_ += value_str_len;
    len_ += value_str_len;
}

void SeriaBuffer::append(double value)
{
    char local_buffer[32];
    promiseEnoughSpace(32);
    sprintf(local_buffer, "%lf", value);
    int value_str_len = strlen(local_buffer);
    memcpy(tail_, local_buffer, value_str_len);
    tail_ += value_str_len;
    len_ += value_str_len;
}

void SeriaBuffer::append(char value)
{
    promiseEnoughSpace(1);
    *tail_ = value;
    tail_ += 1;
    len_ += 1;
}

void SeriaBuffer::append(const char *src, size_t len)
{
    promiseEnoughSpace(len);
    memcpy(tail_, src, len);
    tail_ += len;
    len_ += len;
}

void SeriaBuffer::promiseEnoughSpace(size_t len)
{
    if(len_ + len <= bufferLen_)
        return;
    std::vector<char> local_extern_buffer;
    bufferLen_ = bufferLen_ * 2;
    if(!useExternBuffer_)
    {
        externBuffer_.reserve(2 * bufferLen_);
        memcpy(&externBuffer_[0], buffer_, len_);
        useExternBuffer_ = true;
    }
    else
    {
        local_extern_buffer.reserve(bufferLen_ * 2);
        memcpy(&local_extern_buffer[0], &externBuffer_[0], len_);
        externBuffer_.swap(local_extern_buffer);
    }
    tail_ = &externBuffer_[len_];
    head_ = &externBuffer_[0];
}

void SeriaBuffer::retrieveAll()
{
    externBuffer_.clear();
    externBuffer_.shrink_to_fit();
    tail_ = buffer_;
    head_ = buffer_;
    len_ = 0;
    bufferLen_ = kSeriaBufferSize;
    useExternBuffer_ = false;
}

template<typename T>
T SeriaBuffer::getValueSingle()
{
}

template<>
int SeriaBuffer::getValueSingle()
{
    char *left_border{nullptr}, *right_border{nullptr};
    char *current = head_;
    while(current != tail_)
    {
        if(*current == '<')
            left_border = current + 1;
        else if(*current == ',')
        {
            if(left_border)
            {
                right_border = current - 1;
                break;
            }
            else
            {
                left_border = current + 1;
            }
        }
        else if(*current == '>')
        {
            right_border = current - 1;
        }
        current++;
    }
    if(!right_border || right_border < left_border)
    {
        throw std::invalid_argument("convesation int error");
    }
    int value = std::stoi(std::string(left_border, right_border - left_border + 1));
    head_ = current;
    return value;
}

template<>
double SeriaBuffer::getValueSingle()
{
    char *left_border{nullptr}, *right_border{nullptr};
    char *current = head_;
    while(current != tail_)
    {
        if(*current == '<')
            left_border = current + 1;
        else if(*current == ',')
        {
            if(left_border)
            {
                right_border = current - 1;
                break;
            }
            else
            {
                left_border = current + 1;
            }
        }
        else if(*current == '>')
        {
            right_border = current - 1;
        }
        current++;
    }
    if(!right_border || right_border < left_border)
    {
        throw std::invalid_argument("convesation double error");
    }
    double value = std::stod(std::string(left_border, right_border - left_border + 1));
    head_ = current;
    return value;
}

template<> 
std::string SeriaBuffer::getValueSingle()
{
    char *left_border{nullptr}, *right_border{nullptr};
    char *str_left_border{nullptr}, *str_right_border{nullptr};
    char *current = head_;
    while(current != tail_)
    {
        if(str_left_border && !str_right_border)
        {
            if(*current == '\"')
                str_right_border = current;
        }
        else if(*current == '<')
            left_border = current + 1;
        else if(*current == '\"')
        {
            if(!str_left_border)
                str_left_border = current;
            if(str_right_border)
                throw std::invalid_argument("conversation string error");
        }
        else if(*current == ',')
        {
            if(left_border)
            {
                right_border = current - 1;
                break;
            }
            else
            {
                left_border = current + 1;
            }
        }
        else if(*current == '>')
        {
            right_border = current - 1;
            break;
        }
        current++;
    }
    if(!right_border || right_border < left_border)
    {
        throw std::invalid_argument("convesation string error");
    }
    head_ = current;
    return std::move(std::string(str_left_border + 1, str_right_border - str_left_border - 1));
}

template<>
int SeriaBuffer::getValue()
{
    return getValueSingle<int>();
}

template<>
double SeriaBuffer::getValue()
{
    return getValueSingle<double>();
}

template<>
std::string SeriaBuffer::getValue<std::string>()
{
    return getValueSingle<std::string>();
}

int main(int argc, char const *argv[])
{
    SeriaBuffer buffer;
    for(int i = 0; i < 10000; ++i)
    {
        buffer.appendItem(14.3, "zhangheng", 32, 50);
    }
    for(int i = 0; i < 10000; ++i)
    {
        // auto [v1, v2, v3, v4] = buffer.getValue<double, std::string, int, int>();
        // TODO: 合并 getValue 和 getValueSingle
        auto v1 = buffer.getValueSingle<double>();
        auto v2 = buffer.getValueSingle<std::string>();
        auto v3 = buffer.getValueSingle<int>();
        auto v4 = buffer.getValueSingle<int>();

        std::cout << i << " :"<< v1 << "  " << v2 << " " << v3 << "  " << v4 << std::endl;
    }
    std::cin.get();
    return 0;
}

}
