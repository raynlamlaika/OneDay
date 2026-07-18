#ifndef SANDBOX_HPP 
#define SANDBOX_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/mount.h>
#include <sched.h>     // CLONE_* flags
#include <sys/wait.h>  // waitpid, pid_t
#include <unistd.h>
#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
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

#define MOUNT_FILE "/tmp/sandbox_mount"
#define ROOTFS_PATH "/tmp/sandbox_rootfs"

class Sandbox
{
public:
    void run(std::string cpuLimit, std::string memoryLimit, std::string hostname);
    Sandbox();
    ~Sandbox();
    Sandbox (const Sandbox &other);
    Sandbox &operator=(const Sandbox &other);

   static int child(void *arg);

private:


    void createCgroup(std::string cpuLimit, std::string memoryLimit);
    void setupNamespaces(t_NamespaceConfig config, std::string hostname);

    void setupFilesystem();

    void setupNetwork();

    void setupHostname();

    void setupSecurity();

    void executeProgram();

    void cleanup();
};


#endif