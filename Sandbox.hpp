#ifndef SANDBOX_HPP 
#define SANDBOX_HPP

#include <filesystem>
#include <fstream>
#include <iostream>


class Sandbox
{
public:
    void run(std::string cpuLimit, std::string memoryLimit);
    Sandbox();
    ~Sandbox();
    Sandbox (const Sandbox &other);
    Sandbox &operator=(const Sandbox &other);

private:



    void createCgroup(std::string cpuLimit, std::string memoryLimit);

    void setupNamespaces();

    void setupFilesystem();

    void setupNetwork();

    void setupHostname();

    void setupSecurity();

    void executeProgram();

    void cleanup();
};


#endif