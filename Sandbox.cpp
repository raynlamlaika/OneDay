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

void Sandbox::setupNamespaces(t_NamespaceConfig config)
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
    // clone(childFunction, stack, CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS, nullptr);

    // unshare is buildin lunix that it makes the current process and its children to be in a new namespace
    // so we can use it to create a new namespace for our sandbox.


    // setns() Join an existing namespace.
    // set u the flag for unshare() to create a new namespace for the current process and its children.
    int flags = 0;

    if (config.mount)
        flags |= CLONE_NEWNS;
    
    if (config.uts)
        flags |= CLONE_NEWUTS;
    if (config.network)
        flags |= CLONE_NEWNET;
    if (config.ipc)
        flags |= CLONE_NEWIPC;
    if (config.pid)
        flags |= CLONE_NEWPID;
    if (config.user)
        flags |= CLONE_NEWUSER;
    if (config.cgroup)
        flags |= CLONE_NEWCGROUP;
    

    if (unshare(flags) == -1)
    {
        throw std::runtime_error("Failed to unshare namespaces.");
    }
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

    std::cout << "UID  : " << getuid() << '\n';
    std::cout << "EUID : " << geteuid() << '\n';
    // the clean up here is to remove the cgroup directory
    namespace fs = std::filesystem;
    fs::path cgroupPath = "/sys/fs/cgroup/sandbox";

    if (geteuid() != 0)
    {
        std::cerr << "Warning: skipping cgroup cleanup because it requires root privileges." << std::endl;
        return;
    }

    if (fs::exists(cgroupPath))
    {
        std::error_code errorCode;
        fs::remove(cgroupPath, errorCode);
        if (errorCode)
        {
            std::cerr << "Warning: could not remove cgroup directory: "
                      << errorCode.message() << std::endl;
        }
        else
        {
            std::cout << "Cgroup directory removed successfully." << std::endl;
        }
    }
    else
    {
        std::cerr << "Cgroup directory does not exist." << std::endl;
    }
}
#include <unistd.h>

static std::string getHostname()
{
    char hostname[256] = {0};

    if (gethostname(hostname, sizeof(hostname)) == 0)
        return hostname;
    return "unknown";
}

static void printMetadata()
{
    std::cout << "\n========== SANDBOX METADATA ==========" << "\n";
    std::cout << "Process: pid=" << getpid()
              << " ppid=" << getppid() << '\n';
    std::cout << "Hostname: " << getHostname() << '\n';
    std::cout << "Namespaces: mount pid net uts ipc user cgroup" << '\n';
}

void Sandbox::run(std::string cpuLimit, std::string memoryLimit)
{
    try
    {
        printMetadata();
        createCgroup(cpuLimit, memoryLimit);
        t_NamespaceConfig nsConfig = {true, true, true, true, false, false, false};
        // later we will use the nsConfig to determine which namespaces to setup, but for now, we will just setup all of them.
        setupNamespaces(nsConfig);
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