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
    (void)other;
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

void Sandbox::setupNamespaces(t_NamespaceConfig config, std::string hostname)
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
    if (flags == 0)
        throw std::runtime_error("No namespaces specified for unshare.");
    

    if (unshare(flags) == -1)// unshare it will create a new namespace for the current process and its children.
    {
        throw std::runtime_error("Failed to unshare namespaces.");
    }
    if (sethostname(hostname.c_str(), hostname.length()) == -1)
        perror("sethostname");
    if (mount("tmpfs", MOUNT_FILE, "tmpfs",  MS_REC | MS_PRIVATE , nullptr) == -1)
        perror("mount");

    // system("findmnt -o TARGET,PROPAGATION");

    // pause();
}

void Sandbox::setupFilesystem()
{
    namespace fs = std::filesystem;
 
    if (!fs::exists(ROOTFS_PATH) || !fs::is_directory(ROOTFS_PATH))
        throw std::runtime_error("Rootfs directory does not exist: " ROOTFS_PATH);
 
    // chdir BEFORE chroot: chroot() alone only changes what "/" resolves to,
    // it does not move the process's cwd. If we chroot'd without first
    // being inside ROOTFS_PATH, a process could still reach outside the
    // jail via relative paths (the classic chroot escape).
    if (chdir(ROOTFS_PATH) == -1)
        throw std::runtime_error("chdir to rootfs failed: " + std::string(strerror(errno)));
 
    if (chroot(".") == -1)// chroot for changing the root directory of the current process to the specified path. After a successful call to chroot, the process will see the specified directory as its root directory ("/"). This is often used for creating isolated environments, such as sandboxes or containers, where the process is restricted to a specific portion of the filesystem.
        throw std::runtime_error("chroot failed: " + std::string(strerror(errno)));
 
    // Re-anchor cwd to the new root so relative paths behave as expected
    // for whatever runs next (e.g. execve).
    if (chdir("/") == -1)
        throw std::runtime_error("chdir to / after chroot failed: " + std::string(strerror(errno)));
    
    std::ofstream procs(FILE_PATH);
    procs << getpid();

}

void Sandbox::setupNetwork()
{
    // We are already inside a new network namespace.

    // 1. Bring up loopback.
    system("ip link set lo up");

    // 2. Print interfaces.
    system("ip link");
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
            std::cout << "Cgroup directory removed successfully." << std::endl;
    }
    else
        std::cerr << "Cgroup directory does not exist." << std::endl;
    if (umount(MOUNT_FILE) == -1)
        perror("umount");
    if (rmdir(MOUNT_FILE) == -1)
        perror("rmdir");
}


static std::string getHostname()
{
    char hostname[256] = {0};

    if (gethostname(hostname, sizeof(hostname)) == 0)
        return hostname;
    return "unknown";
}

// static void printMetadata()
// {
//     std::cout << "\n========== SANDBOX METADATA ==========" << "\n";
//     std::cout << "Process: pid=" << getpid()
//               << " ppid=" << getppid() << '\n';
//     std::cout << "Hostname: " << getHostname() << '\n';
//     std::cout << "Namespaces: mount pid net uts ipc user cgroup" << '\n';
// }

int Sandbox::child(void *arg)
{
    Sandbox *sandbox = static_cast<Sandbox *>(arg);
    t_NamespaceConfig nsConfig = 
    {// explaination of every flag : 
        true,  // mount: create a new mount namespace
        true,  // pid: create a new PID namespace
        true,  // net: create a new network namespace
        true,  // uts: create a new UTS namespace
        false, // ipc: create a new IPC namespace
        false, // user: create a new user namespace
        false  // cgroup: create a new cgroup namespace
    };

    std::cout << "Child process started." << std::endl;
    (void) sandbox; // Suppress unused variable warning
    // std::string cpuLimit = sandbox.getCpuLimit(); // Assuming you have a method to get CPU limit
    // std::string memoryLimit = sandbox.getMemoryLimit(); // Assuming you have a method to get memory limit
    // std::string hostname = sandbox.getHostname(); // Assuming you have a method to get hostname
    createCgroup(sandbox->getCpuLimit(), sandbox->getMemoryLimit());
    setupNamespaces(nsConfig, sandbox->getHostname());
    sethostname("ProcLeParent", strlen("ProcLeParent"));
    sandbox->setupNetwork();
    sandbox->setupFilesystem();
    // Now you can access members
    // sandbox->setupFilesystem();
    // sandbox->setupNetwork();
    // first setup the cgroup, then setup the namespaces, then setup the filesystem, then setup the network, then setup the hostname, then setup the security, then execute the program, then cleanup.


    return 0;
}
void Sandbox::run(std::string cpuLimit, std::string memoryLimit, std::string hostname)
{
    constexpr int STACK_SIZE = 1024 * 1024;
    char *stack = new char[STACK_SIZE];
    int flags = CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWPID | SIGCHLD;
    pid_t pid = clone( Sandbox::child, stack + STACK_SIZE, flags, this);
    if (pid == -1)
    {
        std::cerr << "clone() failed: " << strerror(errno) << std::endl;
        throw std::runtime_error("clone failed");
    }
        // else if (pid == 0)
        // {
        // // Child process
        // try
        // {
            // printMetadata();
        //     createCgroup(cpuLimit, memoryLimit);
        //     t_NamespaceConfig nsConfig = 
        //     {// explaination of every flag : 
        //         true,  // mount: create a new mount namespace
        //         true,  // pid: create a new PID namespace
        //         true,  // net: create a new network namespace
        //         true,  // uts: create a new UTS namespace
        //         false, // ipc: create a new IPC namespace
        //         false, // user: create a new user namespace
        //         false  // cgroup: create a new cgroup namespace
        // };

        //     setupNamespaces(nsConfig, hostname);
        //     setupFilesystem();
        //     // setupNetwork();
        //     // setupHostname();
        //     // setupSecurity();
        //     // executeProgram();
        //     printMetadata();
        //     cleanup();

        // }
        // catch (const std::exception &e)
        // {
        //     std::cerr << "Error: " << e.what() << std::endl;
        //     _exit(0);
        // }
    //     std::cout << "Child process running in sandboxed environment." << std::endl;
    // }

    int status;
    waitpid(pid, &status, 0);
    cleanup();
}



/*
to do:
understand the clone flag chroot the network hndling 
mounting the filesystem

*/