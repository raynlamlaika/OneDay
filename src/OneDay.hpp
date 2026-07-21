#ifndef ONEDAY_HPP
#define ONEDAY_HPP



// includes
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>


class OneDay
{
public:
    OneDay();
    OneDay(const OneDay &other);
    OneDay &operator=(const OneDay &other);
    ~OneDay();


    // the func
    void help();
    void Parcer(char **arg);

    void envsetup();



    //getters setters
    int getCpu() const;
    void setCpu(int cpu);
    int getMemory() const;
    void setMemory(int memory);
    std::string getNetwork() const;
    void setNetwork(std::string network);
    std::string getHostname() const;
    void setHostname(std::string hostname);

private:
    int _cpu;
    int _memory;
    std::string _network;
    std::string _hostname;
    std::string PATH;
};




#endif