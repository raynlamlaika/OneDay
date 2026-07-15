#include "Sandbox.hpp"


Sandbox::Sandbox()
{
}

Sandbox::~Sandbox()
{
}

Sandbox::Sandbox(const Sandbox &other)
{
    *this = other;
}

Sandbox &Sandbox::operator=(const Sandbox &other)
{
    if (this != &other)
    {
        // Copy any necessary member variables here
        *this = other;
    }
    return *this;
}




void Sandbox::createCgroup(std::string cpuLimit, std::string memoryLimit)
{
    // start the creation of the cgroup
    namespace fs = std::filesystem;
    fs::path cgroupPath = "/sys/fs/cgroup/sandbox";
    if (!fs::exists(cgroupPath))
    {
        if (!fs::create_directory(cgroupPath))
        {
            throw std::runtime_error("Failed to create cgroup directory.");
        }
    }
    std::ofstream memory(cgroupPath / "memory.max");
    memory << memoryLimit;
    std::ofstream cpu(cgroupPath / "cpu.max");
    cpu << cpuLimit;
}

void Sandbox::setupNamespaces()
{
    // to setup namespaces, we will use the unshare system call
    // unshare(CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWUSER);

    // first: there is a severral namespaces that we can use, but for this project, we will use the following:
    // CLONE_NEWNS: mount namespace
    // CLONE_NEWUTS: hostname namespace
    // CLONE_NEWIPC: IPC namespace
    // CLONE_NEWPID: PID namespace
    // CLONE_NEWNET: network namespace
    // CLONE_NEWUSER: user namespace
    clone(childFunction, stack, CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS, nullptr);

    // unshare is buildin lunix that it makes the current process and its children to be in a new namespace
    // so we can use it to create a new namespace for our sandbox.


    // setns() Join an existing namespace.
}

void Sandbox::setupFilesystem()
{
}

void Sandbox::setupNetwork()
{
}

void Sandbox::setupHostname()
{
}

void Sandbox::setupSecurity()
{
}

void Sandbox::executeProgram()
{
}

void Sandbox::cleanup()
{
    // the clean up here is to remove the cgroup directory
    namespace fs = std::filesystem;
    fs::path cgroupPath = "/sys/fs/cgroup/sandbox";
    if (fs::exists(cgroupPath))
    {
        if (!fs::remove(cgroupPath)){throw std::runtime_error("Failed to remove cgroup directory.");}
        else
            {std::cout << "Cgroup directory removed successfully." << std::endl;}
    }
    else
    {
        std::cerr << "Cgroup directory does not exist." << std::endl;
    }
}

void Sandbox::run(std::string cpuLimit, std::string memoryLimit)
{
    try
    {
        createCgroup(cpuLimit, memoryLimit);
        setupNamespaces();
        // setupFilesystem();
        // setupNetwork();
        // setupHostname();
        // setupSecurity();
        // executeProgram();
        cleanup();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}