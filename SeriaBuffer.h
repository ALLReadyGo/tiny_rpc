#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <tuple>

namespace tiny_rpc
{
class SeriaBuffer
{
  public:
    static constexpr size_t kSeriaBufferSize = 1024;

    SeriaBuffer();

    template<typename T, typename...Args>
    void appendItem(T &&t, Args &&...args)
    {
        appendItem(std::forward<T>(t));
        appendItem(std::forward<Args>(args)...);
    }

    template<size_t N>
    void appendItem(const char (&str)[N])
    {
        assert(strlen(str) == N - 1);
        appendItem(str, N - 1);
    }
    void appendItem(int32_t item);
    void appendItem(int64_t item);
    void appendItem(double item);
    void appendItem(const char *src, size_t len);
    void appendItem(const std::string & item);

    const char *head() const
    {
        return head_;
    }

    int len() const
    {
        return len_;
    }

    template<typename T>
    T getValue()
    {
    }

    template<typename...Args>
    std::tuple<Args...> getValue()
    {
        std::tuple<Args...> tp;
        genTuple<std::tuple<Args...>, 0, Args...>(tp);
        return tp;
    }


    template<typename TP, size_t N>
    void genTuple(TP &tp)
    {
        return;
    }

    template<typename TP, size_t N, typename T, typename...Args>
    void genTuple(TP &tp)
    {
        std::get<N>(tp) = getValueSingle<std::remove_reference_t<T>>();
        genTuple<TP, N + 1, Args...>(tp);
    }


    template<typename T>
    T getValueSingle();

  private:
    void append(int32_t value);
    void append(int64_t value);
    void append(double value);
    void append(char value);
    void append(const char *src, size_t len);
    void promiseEnoughSpace(size_t len);
    void retrieveAll();
    char buffer_[kSeriaBufferSize];
    char *tail_;
    char *head_;
    std::vector<char> externBuffer_;
    int len_ = 0;
    int bufferLen_ = kSeriaBufferSize;
    bool useExternBuffer_ = false;
};
}