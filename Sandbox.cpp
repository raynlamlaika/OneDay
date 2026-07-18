// #include "Sandbox.hpp"


// Sandbox::Sandbox()
// {
//     tmpfsMounted = false;
// }

// Sandbox::~Sandbox()
// {
// }

// Sandbox::Sandbox(const Sandbox &other)
// {
//     *this = other;
// }

// Sandbox &Sandbox::operator=(const Sandbox &other)
// {
//     (void)other;
//     return *this;

// }




// void Sandbox::createCgroup(std::string cpuLimit, std::string memoryLimit)
// {
//     // start the creation of the cgroup
//     namespace fs = std::filesystem;
//     fs::path cgroupPath = "/sys/fs/cgroup/sandbox";
//     if (!fs::exists(cgroupPath))
//     {
//         if (!fs::create_directory(cgroupPath))
//         {
//             throw std::runtime_error("Failed to create cgroup directory.");
//         }
//     }
//     std::ofstream memory(cgroupPath / "memory.max");
//     memory << (std::stoull(memoryLimit) * 1024ULL * 1024ULL);

//     std::ofstream cpu(cgroupPath / "cpu.max");
//     cpu << (std::stoull(cpuLimit) * 1000ULL) << " 100000";

//     std::ofstream procs(cgroupPath / "cgroup.procs");
//     if (!procs)
//         throw std::runtime_error("Failed to open cgroup.procs.");
//     procs << getpid();

//     std::ofstream pidsMax(cgroupPath / "pids.max");
//     if (pidsMax)
//         pidsMax << "max";
// }

// bool Sandbox::setupNamespaces(t_NamespaceConfig config, std::string hostname)
// {
//     // to setup namespaces, we will use the unshare system call
//     // unshare(CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWUSER);

//     // first: there is a severral namespaces that we can use, but for this project, we will use the following:
//     // CLONE_NEWNS: mount namespace
//     // CLONE_NEWUTS: hostname namespace
//     // CLONE_NEWIPC: IPC namespace
//     // CLONE_NEWPID: PID namespace
//     // CLONE_NEWNET: network namespace
//     // CLONE_NEWUSER: user namespace
//     // clone(childFunction, stack, CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS, nullptr);

//     // unshare is buildin lunix that it makes the current process and its children to be in a new namespace
//     // so we can use it to create a new namespace for our sandbox.


//     // setns() Join an existing namespace.
//     // set u the flag for unshare() to create a new namespace for the current process and its children.
//     int flags = 0;

//     if (config.mount)
//         flags |= CLONE_NEWNS;
//     if (config.uts)
//         flags |= CLONE_NEWUTS;
//     if (config.ipc)
//         flags |= CLONE_NEWIPC;
//     // if (config.pid)
//     //     flags |= CLONE_NEWPID;
//     if (config.user)
//         flags |= CLONE_NEWUSER;
//     if (config.cgroup)
//         flags |= CLONE_NEWCGROUP;
//     if (flags == 0)
//         throw std::runtime_error("No namespaces specified for unshare.");
    

//     if (unshare(flags) == -1)// unshare it will create a new namespace for the current process and its children.
//     {
//         throw std::runtime_error("Failed to unshare namespaces.");
//     }
//     std::filesystem::create_directories(MOUNT_FILE);
//     if (config.mount && mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) == -1)
//         perror("mount private root");
//     if (sethostname(hostname.c_str(), hostname.length()) == -1)
//         perror("sethostname");
//     bool tmpfsMounted = false;
//     if (mount("tmpfs", MOUNT_FILE, "tmpfs", 0, nullptr) == -1)
//         perror("mount");
//     else
//     {
//         tmpfsMounted = true;
//     }

//     // system("findmnt -o TARGET,PROPAGATION");

//     // pause();
//     return tmpfsMounted;
// }

// void Sandbox::setupFilesystem()
// {
//     namespace fs = std::filesystem;
 
//     if (fs::exists(ROOTFS_PATH) && !fs::is_directory(ROOTFS_PATH))
//         throw std::runtime_error("Rootfs path exists but is not a directory: " ROOTFS_PATH);

//     if (!fs::exists(ROOTFS_PATH))
//         fs::create_directories(ROOTFS_PATH);

//     fs::create_directories(fs::path(ROOTFS_PATH) / "tmp/sandbox_mount/proc/self");
//     fs::create_directories(fs::path(ROOTFS_PATH) / "proc");
//     fs::create_directories(fs::path(ROOTFS_PATH) / "root");
//     fs::create_directories(fs::path(ROOTFS_PATH) / "home");
//     fs::create_directories(fs::path(ROOTFS_PATH) / "tmp");
//     fs::create_directories(fs::path(ROOTFS_PATH) / "etc");

//     auto bindDirectory = [](const fs::path &source, const fs::path &target)
//     {
//         if (!fs::exists(source))
//             return;

//         fs::create_directories(target);
//         if (mount(source.c_str(), target.c_str(), nullptr, MS_BIND | MS_REC, nullptr) == -1)
//             throw std::runtime_error("Failed to bind mount " + source.string() + " to " + target.string() + ": " + std::string(strerror(errno)));
//     };

//     auto bindFile = [](const fs::path &source, const fs::path &target)
//     {
//         if (!fs::exists(source))
//             return;

//         fs::create_directories(target.parent_path());
//         std::ofstream(target).close();
//         if (mount(source.c_str(), target.c_str(), nullptr, MS_BIND, nullptr) == -1)
//             throw std::runtime_error("Failed to bind mount " + source.string() + " to " + target.string() + ": " + std::string(strerror(errno)));
//     };

//     bindDirectory("/bin", fs::path(ROOTFS_PATH) / "bin");
//     bindDirectory("/usr/bin", fs::path(ROOTFS_PATH) / "usr/bin");
//     bindDirectory("/usr", fs::path(ROOTFS_PATH) / "usr");
//     bindDirectory("/lib", fs::path(ROOTFS_PATH) / "lib");
//     bindDirectory("/lib64", fs::path(ROOTFS_PATH) / "lib64");
//     bindDirectory("/sbin", fs::path(ROOTFS_PATH) / "sbin");
//     bindDirectory("/usr/sbin", fs::path(ROOTFS_PATH) / "usr/sbin");
//     bindDirectory("/usr/lib", fs::path(ROOTFS_PATH) / "usr/lib");
//     bindDirectory("/usr/lib64", fs::path(ROOTFS_PATH) / "usr/lib64");
//     bindFile("/run/systemd/resolve/resolv.conf", fs::path(ROOTFS_PATH) / "etc/resolv.conf");
//     bindFile("/etc/hosts", fs::path(ROOTFS_PATH) / "etc/hosts");
//     bindFile("/etc/nsswitch.conf", fs::path(ROOTFS_PATH) / "etc/nsswitch.conf");
 
//     // chdir BEFORE chroot: chroot() alone only changes what "/" resolves to,
//     // it does not move the process's cwd. If we chroot'd without first
//     // being inside ROOTFS_PATH, a process could still reach outside the
//     // jail via relative paths (the classic chroot escape).
//     if (chdir(ROOTFS_PATH) == -1)
//         throw std::runtime_error("chdir to rootfs failed: " + std::string(strerror(errno)));
 
//     if (chroot(".") == -1)// chroot for changing the root directory of the current process to the specified path. After a successful call to chroot, the process will see the specified directory as its root directory ("/"). This is often used for creating isolated environments, such as sandboxes or containers, where the process is restricted to a specific portion of the filesystem.
//         throw std::runtime_error("chroot failed: " + std::string(strerror(errno)));
 
//     // Re-anchor cwd to the new root so relative paths behave as expected
//     // for whatever runs next (e.g. execve).
//     if (chdir("/") == -1)
//         throw std::runtime_error("chdir to / after chroot failed: " + std::string(strerror(errno)));

//     if (mount("proc", "/proc", "proc", 0, nullptr) == -1)
//         throw std::runtime_error("mount proc failed: " + std::string(strerror(errno)));
    
//     std::ofstream procs(FILE_PATH);
//     procs << getpid();

// }

// void Sandbox::setupNetwork()
// {
//     // We are already inside a new network namespace.

//     // 1. Bring up loopback.
//     system("/usr/sbin/ip link set lo up");

//     // 2. Print interfaces.
//     system("/usr/sbin/ip link");
// }

// void Sandbox::setupHostname()
// {
// }

// void Sandbox::setupSecurity()
// {
// }

// void Sandbox::executeProgram()
// {
//     struct rlimit limit = {RLIM_INFINITY, RLIM_INFINITY};
//     if (setrlimit(RLIMIT_NPROC, &limit) == -1)
//         perror("setrlimit RLIMIT_NPROC");
//     if (setrlimit(RLIMIT_AS, &limit) == -1)
//         perror("setrlimit RLIMIT_AS");
//     if (setrlimit(RLIMIT_STACK, &limit) == -1)
//         perror("setrlimit RLIMIT_STACK");
//     setenv("HOME", "/root", 1);
//     setenv("TERM", "xterm-256color", 1);
//     setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 1);
//     execl("/usr/bin/bash", "bash", static_cast<char *>(nullptr));
//     throw std::runtime_error("exec bash failed: " + std::string(strerror(errno)));
// }

// void Sandbox::cleanup()
// {
//     // the clean up here is to remove the cgroup directory
//     namespace fs = std::filesystem;
//     fs::path cgroupPath = "/sys/fs/cgroup/sandbox";

//     if (geteuid() != 0)
//     {
//         std::cerr << "Warning: skipping cgroup cleanup because it requires root privileges." << std::endl;
//         return;
//     }

//     if (fs::exists(cgroupPath))
//     {
//         std::error_code errorCode;
//         fs::remove(cgroupPath, errorCode);
//         if (errorCode)
//         {
//             std::cerr << "Warning: could not remove cgroup directory: "
//                       << errorCode.message() << std::endl;
//         }
//         else
//             std::cout << "Cgroup directory removed successfully." << std::endl;
//     }
//     else
//         std::cerr << "Cgroup directory does not exist." << std::endl;
//     if (umount2(MOUNT_FILE, MNT_DETACH) == -1 && errno != EINVAL)
//         perror("umount2");
//     if (rmdir(MOUNT_FILE) == -1)
//         perror("rmdir");
// }


// static std::string getHostname()
// {
//     char hostname[256] = {0};

//     if (gethostname(hostname, sizeof(hostname)) == 0)
//         return hostname;
//     return "unknown";
// }

// // static void printMetadata()
// // {
// //     std::cout << "\n========== SANDBOX METADATA ==========" << "\n";
// //     std::cout << "Process: pid=" << getpid()
// //               << " ppid=" << getppid() << '\n';
// //     std::cout << "Hostname: " << getHostname() << '\n';
// //     std::cout << "Namespaces: mount pid net uts ipc user cgroup" << '\n';
// // }

// int Sandbox::child(void *arg)
// {
//     Sandbox *sandbox = static_cast<Sandbox *>(arg);
//     t_NamespaceConfig nsConfig = 
//     {// explaination of every flag : 
//         true,  // mount: create a new mount namespace
//         true,  // pid: create a new PID namespace
//         false, // net: use the VM's existing network namespace so DNS and outbound traffic work
//         true,  // uts: create a new UTS namespace
//         false, // ipc: create a new IPC namespace
//         false, // user: create a new user namespace
//         false  // cgroup: create a new cgroup namespace
//     };

//     try
//     {
//         std::cout << "Child process started." << std::endl;
//         createCgroup(sandbox->getCpuLimit(), sandbox->getMemoryLimit());
//         sandbox->tmpfsMounted = setupNamespaces(nsConfig, sandbox->getHostname());
//         sethostname("ProcLeParent", strlen("ProcLeParent"));
//         sandbox->setupFilesystem();
//         sandbox->setupNetwork();
//         sandbox->executeProgram();
//         return 0;
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Sandbox child error: " << e.what() << std::endl;
//         _exit(1);
//     }
// }
// void Sandbox::run(std::string cpuLimit, std::string memoryLimit, std::string hostname)
// {
//     constexpr int STACK_SIZE = 10024 * 1024;
//     char *stack = new char[STACK_SIZE];
//     int flags = CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | SIGCHLD;
//     pid_t pid = clone( Sandbox::child, stack + STACK_SIZE, flags, this);
//     if (pid == -1)
//     {
//         std::cerr << "clone() failed: " << strerror(errno) << std::endl;
//         throw std::runtime_error("clone failed");
//     }
//     int status;
//     waitpid(pid, &status, 0);
//     cleanup();
//     delete[] stack;
// }



// /*
// to do:
// understand the clone flag chroot the network hndling 
// mounting the filesystem

// */


#include "Sandbox.hpp"


Sandbox::Sandbox()
{
    tmpfsMounted = false;
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
    memory << (std::stoull(memoryLimit) * 1024ULL * 1024ULL);

    std::ofstream cpu(cgroupPath / "cpu.max");
    cpu << (std::stoull(cpuLimit) * 1000ULL) << " 100000";

    std::ofstream procs(cgroupPath / "cgroup.procs");
    if (!procs)
        throw std::runtime_error("Failed to open cgroup.procs.");
    procs << getpid();

    std::ofstream pidsMax(cgroupPath / "pids.max");
    if (pidsMax)
        pidsMax << "max";
}

bool Sandbox::setupNamespaces(t_NamespaceConfig config, std::string hostname)
{
    int flags = 0;

    if (config.mount)
        flags |= CLONE_NEWNS;
    if (config.uts)
        flags |= CLONE_NEWUTS;
    if (config.network)
        flags |= CLONE_NEWNET;
    if (config.ipc)
        flags |= CLONE_NEWIPC;
    // NOTE: PID namespace is intentionally NOT unshared here. It's already
    // created correctly via CLONE_NEWPID in the clone() call in run().
    // unshare(CLONE_NEWPID) does not move the calling process into the new
    // namespace (per unshare(2)) -- it only diverts future forked children
    // into a half-seeded namespace with no init, which is what caused the
    // earlier "fork: Cannot allocate memory" bug.
    if (config.user)
        flags |= CLONE_NEWUSER;
    if (config.cgroup)
        flags |= CLONE_NEWCGROUP;
    if (flags == 0)
        throw std::runtime_error("No namespaces specified for unshare.");


    if (unshare(flags) == -1)
    {
        throw std::runtime_error("Failed to unshare namespaces.");
    }
    std::filesystem::create_directories(MOUNT_FILE);
    if (config.mount && mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) == -1)
        perror("mount private root");
    if (sethostname(hostname.c_str(), hostname.length()) == -1)
        perror("sethostname");
    bool tmpfsMounted = false;
    if (mount("tmpfs", MOUNT_FILE, "tmpfs", 0, nullptr) == -1)
        perror("mount");
    else
    {
        tmpfsMounted = true;
    }

    return tmpfsMounted;
}

void Sandbox::setupFilesystem()
{
    namespace fs = std::filesystem;

    if (fs::exists(ROOTFS_PATH) && !fs::is_directory(ROOTFS_PATH))
        throw std::runtime_error("Rootfs path exists but is not a directory: " ROOTFS_PATH);

    if (!fs::exists(ROOTFS_PATH))
        fs::create_directories(ROOTFS_PATH);

    fs::create_directories(fs::path(ROOTFS_PATH) / "tmp/sandbox_mount/proc/self");
    fs::create_directories(fs::path(ROOTFS_PATH) / "proc");
    fs::create_directories(fs::path(ROOTFS_PATH) / "root");
    fs::create_directories(fs::path(ROOTFS_PATH) / "home");
    fs::create_directories(fs::path(ROOTFS_PATH) / "tmp");
    fs::create_directories(fs::path(ROOTFS_PATH) / "etc");

    // FIX: /etc/resolv.conf never existed inside the chroot, so DNS
    // resolution had nothing to read at all. Has to be written before
    // chroot, since it's addressed via the host-visible ROOTFS_PATH here.
    {
        std::ofstream resolv(fs::path(ROOTFS_PATH) / "etc/resolv.conf");
        resolv << "nameserver 8.8.8.8\n";
        resolv << "nameserver 1.1.1.1\n";
    }

    auto bindDirectory = [](const fs::path &source, const fs::path &target)
    {
        if (!fs::exists(source))
            return;

        fs::create_directories(target);
        if (mount(source.c_str(), target.c_str(), nullptr, MS_BIND | MS_REC, nullptr) == -1)
            throw std::runtime_error("Failed to bind mount " + source.string() + " to " + target.string() + ": " + std::string(strerror(errno)));
    };

    bindDirectory("/bin", fs::path(ROOTFS_PATH) / "bin");
    bindDirectory("/usr/bin", fs::path(ROOTFS_PATH) / "usr/bin");
    bindDirectory("/usr", fs::path(ROOTFS_PATH) / "usr");
    bindDirectory("/lib", fs::path(ROOTFS_PATH) / "lib");
    bindDirectory("/lib64", fs::path(ROOTFS_PATH) / "lib64");
    bindDirectory("/sbin", fs::path(ROOTFS_PATH) / "sbin");
    bindDirectory("/usr/sbin", fs::path(ROOTFS_PATH) / "usr/sbin");
    bindDirectory("/usr/lib", fs::path(ROOTFS_PATH) / "usr/lib");
    bindDirectory("/usr/lib64", fs::path(ROOTFS_PATH) / "usr/lib64");

    // chdir BEFORE chroot: chroot() alone only changes what "/" resolves to,
    // it does not move the process's cwd. If we chroot'd without first
    // being inside ROOTFS_PATH, a process could still reach outside the
    // jail via relative paths (the classic chroot escape).
    if (chdir(ROOTFS_PATH) == -1)
        throw std::runtime_error("chdir to rootfs failed: " + std::string(strerror(errno)));

    if (chroot(".") == -1)
        throw std::runtime_error("chroot failed: " + std::string(strerror(errno)));

    // Re-anchor cwd to the new root so relative paths behave as expected
    // for whatever runs next (e.g. execve).
    if (chdir("/") == -1)
        throw std::runtime_error("chdir to / after chroot failed: " + std::string(strerror(errno)));

    if (mount("proc", "/proc", "proc", 0, nullptr) == -1)
        throw std::runtime_error("mount proc failed: " + std::string(strerror(errno)));

    std::ofstream procs(FILE_PATH);
    procs << getpid();

}

void Sandbox::setupNetwork()
{
    // We're inside the new network namespace, and veth-sandbox has just
    // been moved in by the parent's setupHostNetworking() -- the sync pipe
    // read in child() guarantees that already happened by the time we get
    // here.
    system("/usr/sbin/ip link set lo up");
    system("/usr/sbin/ip addr add 10.200.1.2/24 dev veth-sandbox");
    system("/usr/sbin/ip link set veth-sandbox up");
    system("/usr/sbin/ip route add default via 10.200.1.1");

    system("/usr/sbin/ip link");
}

// FIX: new method, runs in the PARENT (host netns), after clone() so the
// child's netns already exists. Creates the veth pair, hands one end to
// the child, keeps the other on the host, and enables NAT so the sandbox
// subnet can actually reach the internet.
void Sandbox::setupHostNetworking(pid_t childPid)
{
    std::string pidStr = std::to_string(childPid);

    auto run = [](const std::string &cmd)
    {
        if (system(cmd.c_str()) != 0)
            throw std::runtime_error("host networking command failed: " + cmd);
    };

    run("ip link add veth-host type veth peer name veth-sandbox");
    run("ip link set veth-sandbox netns " + pidStr);
    run("ip addr add 10.200.1.1/24 dev veth-host");
    run("ip link set veth-host up");

    run("sysctl -w net.ipv4.ip_forward=1 >/dev/null");
    // idempotent MASQUERADE rule: skip adding it again if it's already there
    system("iptables -t nat -C POSTROUTING -s 10.200.1.0/24 -j MASQUERADE 2>/dev/null || "
           "iptables -t nat -A POSTROUTING -s 10.200.1.0/24 -j MASQUERADE");
}

void Sandbox::setupHostname()
{
}

void Sandbox::setupSecurity()
{
}

void Sandbox::executeProgram()
{
    struct rlimit limit = {RLIM_INFINITY, RLIM_INFINITY};
    if (setrlimit(RLIMIT_NPROC, &limit) == -1)
        perror("setrlimit RLIMIT_NPROC");
    if (setrlimit(RLIMIT_AS, &limit) == -1)
        perror("setrlimit RLIMIT_AS");
    if (setrlimit(RLIMIT_STACK, &limit) == -1)
        perror("setrlimit RLIMIT_STACK");
    setenv("HOME", "/root", 1);
    setenv("TERM", "xterm-256color", 1);
    setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 1);
    execl("/usr/bin/bash", "bash", static_cast<char *>(nullptr));
    throw std::runtime_error("exec bash failed: " + std::string(strerror(errno)));
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
    if (umount2(MOUNT_FILE, MNT_DETACH) == -1 && errno != EINVAL)
        perror("umount2");
    if (rmdir(MOUNT_FILE) == -1)
        perror("rmdir");

    // FIX: tear down the host side of the veth pair + NAT rule. veth-sandbox
    // disappears on its own when the child's netns is destroyed, but
    // veth-host and the iptables rule live in the host namespace and won't
    // clean themselves up.
    system("ip link del veth-host 2>/dev/null");
    system("iptables -t nat -D POSTROUTING -s 10.200.1.0/24 -j MASQUERADE 2>/dev/null");
}


static std::string getHostname()
{
    char hostname[256] = {0};

    if (gethostname(hostname, sizeof(hostname)) == 0)
        return hostname;
    return "unknown";
}

int Sandbox::child(void *arg)
{
    Sandbox *sandbox = static_cast<Sandbox *>(arg);
    t_NamespaceConfig nsConfig = 
    {// explaination of every flag : 
        true,  // mount: create a new mount namespace
        true,  // pid: informational only, see setupNamespaces() note
        true,  // net: create a new network namespace
        true,  // uts: create a new UTS namespace
        false, // ipc: create a new IPC namespace
        false, // user: create a new user namespace
        false  // cgroup: create a new cgroup namespace
    };

    try
    {
        std::cout << "Child process started." << std::endl;
        createCgroup(sandbox->getCpuLimit(), sandbox->getMemoryLimit());
        sandbox->tmpfsMounted = setupNamespaces(nsConfig, sandbox->getHostname());
        sandbox->setupFilesystem();

        // FIX: block here until the parent has moved veth-sandbox into
        // this netns (setupHostNetworking runs in the parent after
        // clone() returns). Without this, setupNetwork() below could run
        // before the interface exists at all.
        char syncByte;
        if (read(sandbox->syncPipeReadFd, &syncByte, 1) != 1)
            std::cerr << "Warning: sync pipe read failed, network may not be ready" << std::endl;
        close(sandbox->syncPipeReadFd);

        sandbox->setupNetwork();
        sandbox->executeProgram();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Sandbox child error: " << e.what() << std::endl;
        _exit(1);
    }
}
void Sandbox::run(std::string cpuLimit, std::string memoryLimit, std::string hostname)
{
    int syncPipe[2];
    if (pipe(syncPipe) == -1)
        throw std::runtime_error("pipe failed: " + std::string(strerror(errno)));
    syncPipeReadFd = syncPipe[0];

    constexpr int STACK_SIZE = 1024 * 1024;
    char *stack = new char[STACK_SIZE];
    int flags = CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWPID | SIGCHLD;
    pid_t pid = clone( Sandbox::child, stack + STACK_SIZE, flags, this);
    if (pid == -1)
    {
        std::cerr << "clone() failed: " << strerror(errno) << std::endl;
        close(syncPipe[0]);
        close(syncPipe[1]);
        delete[] stack;
        throw std::runtime_error("clone failed");
    }

    // Parent doesn't read from the pipe -- close our copy of the read end.
    close(syncPipe[0]);

    // FIX: now that the child (and its fresh netns) exists, wire up the
    // veth pair + NAT from the host side, then let the child proceed.
    try
    {
        setupHostNetworking(pid);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Host networking setup failed: " << e.what() << std::endl;
    }
    if (write(syncPipe[1], "1", 1) != 1)
        perror("sync pipe write");
    close(syncPipe[1]);

    int status;
    waitpid(pid, &status, 0);
    cleanup();
    delete[] stack;
}
