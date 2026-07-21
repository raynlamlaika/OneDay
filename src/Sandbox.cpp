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

    {
        std::ofstream sandboxResolv(fs::path(ROOTFS_PATH) / "etc/resolv.conf");

        auto copyResolverFile = [&](const fs::path &source)
        {
            std::ifstream input(source);
            if (!input)
                return false;

            std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
            if (contents.find("127.0.0.53") != std::string::npos)
                return false;

            sandboxResolv << contents;
            return true;
        };

        if (!copyResolverFile("/run/systemd/resolve/resolv.conf") &&
            !copyResolverFile("/etc/resolv.conf"))
        {
            sandboxResolv << "nameserver 8.8.8.8\n";
            sandboxResolv << "nameserver 1.1.1.1\n";
        }
    }

    auto bindDirectory = [](const fs::path &source, const fs::path &target)
    {
        if (!fs::exists(source))
            return;

        fs::create_directories(target);
        if (mount(source.c_str(), target.c_str(), nullptr, MS_BIND | MS_REC, nullptr) == -1)
            throw std::runtime_error("Failed to bind mount " + source.string() + " to " + target.string() + ": " + std::string(strerror(errno)));
    };

    auto bindFile = [](const fs::path &source, const fs::path &target)
    {
        if (!fs::exists(source))
            return;

        fs::create_directories(target.parent_path());
        std::ofstream(target).close();
        if (mount(source.c_str(), target.c_str(), nullptr, MS_BIND, nullptr) == -1)
            throw std::runtime_error("Failed to bind mount " + source.string() + " to " + target.string() + ": " + std::string(strerror(errno)));
    };

    bindDirectory("/bin", fs::path(ROOTFS_PATH) / "bin");
    bindDirectory("/usr/bin", fs::path(ROOTFS_PATH) / "usr/bin");
    bindDirectory("/usr", fs::path(ROOTFS_PATH) / "usr");
    bindDirectory("/lib", fs::path(ROOTFS_PATH) / "lib");
    bindDirectory("/lib64", fs::path(ROOTFS_PATH) / "lib64");
    bindDirectory("/lib/x86_64-linux-gnu", fs::path(ROOTFS_PATH) / "lib/x86_64-linux-gnu");
    bindDirectory("/sbin", fs::path(ROOTFS_PATH) / "sbin");
    bindDirectory("/usr/sbin", fs::path(ROOTFS_PATH) / "usr/sbin");
    bindDirectory("/usr/lib", fs::path(ROOTFS_PATH) / "usr/lib");
    bindDirectory("/usr/lib64", fs::path(ROOTFS_PATH) / "usr/lib64");
    bindDirectory("/usr/lib/x86_64-linux-gnu", fs::path(ROOTFS_PATH) / "usr/lib/x86_64-linux-gnu");
    bindFile("/etc/hosts", fs::path(ROOTFS_PATH) / "etc/hosts");
    bindFile("/etc/nsswitch.conf", fs::path(ROOTFS_PATH) / "etc/nsswitch.conf");

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
    system("/usr/sbin/ip link set lo up");
    system("/usr/sbin/ip addr add 10.200.1.2/24 dev veth-sandbox");
    system("/usr/sbin/ip link set veth-sandbox up");
    system("/usr/sbin/ip route add default via 10.200.1.1");

    system("/usr/sbin/ip link");
}

void Sandbox::setupHostNetworking(pid_t childPid)
{
    std::string pidStr = std::to_string(childPid);

    auto run = [](const std::string &cmd)
    {
        if (system(cmd.c_str()) != 0)
            throw std::runtime_error("host networking command failed: " + cmd);
    };

    system("ip link del veth-host 2>/dev/null");
    run("ip link add veth-host type veth peer name veth-sandbox");
    run("ip link set veth-sandbox netns " + pidStr);
    run("ip addr add 10.200.1.1/24 dev veth-host");
    run("ip link set veth-host up");

    run("sysctl -w net.ipv4.ip_forward=1 >/dev/null");
    // idempotent MASQUERADE rule: skip adding it again if it's already there
    system("iptables -t nat -C POSTROUTING -s 10.200.1.0/24 -j MASQUERADE 2>/dev/null || "
           "iptables -t nat -A POSTROUTING -s 10.200.1.0/24 -j MASQUERADE");
}

void Sandbox::setupHostname(std::string &hostname)
{
    if (sethostname(hostname.c_str(), hostname.length()) == -1)
        throw std::runtime_error("sethostname failed: " + std::string(strerror(errno)));
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
        false, // net: clone() already created the network namespace
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
    int status;
    try
    {
        setupHostNetworking(pid);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Host networking setup failed: " << e.what() << std::endl;
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        delete[] stack;
        throw;
    }
    if (write(syncPipe[1], "1", 1) != 1)
        perror("sync pipe write");
    close(syncPipe[1]);

    waitpid(pid, &status, 0);
    cleanup();
    delete[] stack;
}


/*
check those commad 
sudo nft add rule ip filter DOCKER-USER ip saddr 10.200.1.0/24 accept
sudo nft add rule ip filter DOCKER-USER ip daddr 10.200.1.0/24 ct state established,related accept
*/