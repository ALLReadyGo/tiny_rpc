#include <tuple>
#include <iostream>
#include <vector>

using namespace std;

template<typename T>
void seriaSingle(std::ostream &os, T &&t)
{
    os << t;
}

template<typename T, typename...Args>
void seriaSingle(std::ostream &os, T &&t, Args&&...args)
{
    os << t << ',';
    seriaSingle(os, std::forward<Args>(args)...);
}

template<typename...Args>
void seriaArgs(std::ostream &os, Args&&...args)
{
    os << '<';
    seriaSingle(os, std::forward<Args>(args)...);
    os << '>';
}

int main(int argc, char const *argv[])
{
    string str = " -231";
    cout << stoi(str);
    return 0;
}
