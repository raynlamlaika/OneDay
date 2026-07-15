#include "OneDay.hpp"
#include "Sandbox.hpp"



int main(int ac, char **av)
{
    if (ac < 2){std::cout << "Usage: ./OneDay OR --help" << std::endl;return 1;}
    try
    {
        OneDay oneDayObj;
        if (std::string(av[1]) == "--help")
        {
            oneDayObj.help();
            return 0;
        }
    
        oneDayObj.Parcer(av);
        // print the arguments
        std::cout << "CPU: " << oneDayObj.getCpu() << std::endl;
        std::cout << "Memory: " << oneDayObj.getMemory() << std::endl;
        std::cout << "Network: " << oneDayObj.getNetwork() << std::endl;
        std::cout << "Hostname: " << oneDayObj.getHostname() << std::endl;

        Sandbox sandboxObj;
        sandboxObj.run(std::to_string(oneDayObj.getCpu()), std::to_string(oneDayObj.getMemory()));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }




}









