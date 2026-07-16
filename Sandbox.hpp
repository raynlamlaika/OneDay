#ifndef SANDBOX_HPP 
#define SANDBOX_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

typedef struct t_NamespaceConfig
{
    bool mount;
    bool pid;
    bool network;
    bool uts;
    bool ipc;
    bool user;
    bool cgroup;
} t_NamespaceConfig;
class Sandbox
{
public:
    void run(std::string cpuLimit, std::string memoryLimit, std::string hostname);
    Sandbox();
    ~Sandbox();
    Sandbox (const Sandbox &other);
    Sandbox &operator=(const Sandbox &other);

private:


    void createCgroup(std::string cpuLimit, std::string memoryLimit);
    void setupNamespaces(t_NamespaceConfig config);

    void setupFilesystem();

    void setupNetwork();

    void setupHostname();

    void setupSecurity();

    void executeProgram();

    void cleanup();
};


#endif