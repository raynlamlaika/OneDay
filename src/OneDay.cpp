#include "OneDay.hpp"


OneDay::OneDay() : _cpu(0), _memory(0), _network(""), _hostname(""), PATH("")
{
}

OneDay::OneDay(const OneDay &other)
{
    *this = other;
}

OneDay& OneDay::operator=(const OneDay &other)
{
    if (this != &other)
    {
        _cpu = other._cpu;
        _memory = other._memory;
        _network = other._network;
        _hostname = other._hostname;
        PATH = other.PATH;
    }
    return *this;
}

OneDay::~OneDay()
{
}




void OneDay::help()
{
    std::cout << "Usage: ./OneDay OR --help" << std::endl;
    std::cout << "The program take one parameter possible args is:" << std::endl;
    std::cout << "  -h, --help     Show this help message and exit" << std::endl;
    std::cout << "  -c, --cpu Number of CPU cores to use" << std::endl;
    std::cout << "  -m, --memory Number of memory slots to use" << std::endl;
    std::cout << "  -n, --network Number of network interfaces to use" << std::endl;
    std::cout << "  -hn, --hostname Hostname to use" << std::endl;
}


void OneDay::Parcer(char **arg)
{
    if (arg == nullptr || arg[0] == nullptr)
        throw std::invalid_argument("Invalid arguments.");
    if (arg[1] != nullptr && std::string(arg[1]) == "--help")
    {
        help();
        return;
    }

    
    PATH = std::string(arg[1]);

    std::cout << "PATH = " << PATH << std::endl;

    for (size_t i = 2; arg[i] != nullptr; ++i)
    {
        std::string option(arg[i]);

        std::cout << "arg[" << i << "] = " << option << std::endl;

        if (option == "-c" || option == "--cpu")
        {
            if (arg[++i] == nullptr)
                throw std::invalid_argument("Missing value for --cpu.");

            setCpu(std::atoi(arg[i]));
        }
        else if (option == "-m" || option == "--memory")
        {
            if (arg[++i] == nullptr)
                throw std::invalid_argument("Missing value for --memory.");

            // If your member is an int, this accepts "512M" as 512.
            // Better would be to store the string and parse units later.
            setMemory(std::atoi(arg[i]));
        }
        else if (option == "-n" || option == "--network")
        {
            if (arg[++i] == nullptr)
                throw std::invalid_argument("Missing value for --network.");

            // network should be a string
            setNetwork(std::string(arg[i]));
        }
        else if (option == "-hn" || option == "--hostname")
        {
            if (arg[++i] == nullptr)
                throw std::invalid_argument("Missing value for --hostname.");

            setHostname(std::string(arg[i]));
        }
        else
        {
            throw std::invalid_argument("Unknown option: " + option);
        }
    }
}

void OneDay::envsetup()
{
    // Implementation for environment setup

    
}


// getters setters
int OneDay::getCpu() const{return _cpu;}
int OneDay::getMemory() const{return _memory;}
std::string OneDay::getHostname() const{return _hostname;}
std::string OneDay::getNetwork() const{return _network;}

void OneDay::setMemory(int memory){_memory = memory;}
void OneDay::setNetwork(std::string network){_network = network;}
void OneDay::setCpu(int cpu){_cpu = cpu;}
void OneDay::setHostname(std::string hostname){_hostname = hostname;}


