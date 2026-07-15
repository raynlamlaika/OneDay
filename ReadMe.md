# OneDay

> A lightweight Linux sandbox runtime built from scratch in C++ to understand how modern containers work.

## Overview

OneDay is an educational container runtime that explores the Linux kernel primitives used by technologies such as Docker, Podman, and LXC. Instead of relying on existing container engines, the project implements process isolation and resource management directly using Linux system calls.

The objective is not to build another Docker clone, but to gain a deep understanding of how the Linux kernel provides isolation, security, and resource control.

---

# Features

* Process isolation using Linux namespaces
* CPU and memory limitation through cgroups v2
* Filesystem isolation
* Custom root filesystem
* Capability management
* Process execution with `execve()`
* Secure sandbox environment
* Modular C++ architecture

---

# Learning Goals

This project focuses on understanding the Linux kernel rather than simply using existing tools.

Topics covered include:

* Linux Processes
* Process Creation (`fork`, `clone`)
* Namespace Isolation
* Control Groups (cgroups v2)
* Linux Capabilities
* Virtual Filesystems
* Mount Namespaces
* PID Namespaces
* Network Namespaces
* User Namespaces
* IPC Namespaces
* System Calls
* Process Scheduling
* Memory Management

---

# Architecture

```text
                    User Command
                         │
                         ▼
                  Command Parser
                         │
                         ▼
                  Runtime Manager
        ┌────────────────┼────────────────┐
        │                │                │
        ▼                ▼                ▼
 Namespace Setup     CGroup Setup    Filesystem Setup
        │                │                │
        └────────────────┼────────────────┘
                         │
                         ▼
                 Capability Manager
                         │
                         ▼
                     execve()
                         │
                         ▼
                  Sandboxed Process
```

---

# Linux Technologies Used

## Namespaces

* UTS
* PID
* Mount
* Network
* IPC
* User
* Cgroup

---

## Cgroups v2

Resource control through:

* `cpu.max`
* `memory.max`
* `memory.current`
* `memory.events`
* `pids.max`

---

## System Calls

The project makes direct use of Linux system calls including:

* `fork`
* `clone`
* `unshare`
* `setns`
* `execve`
* `waitpid`
* `mount`
* `pivot_root`
* `chroot`
* `sethostname`
* `setuid`
* `setgid`
* `capset`
* `prctl`

---

# Project Structure

```text
OneDay/
│
├── src/
│   ├── Runtime.cpp
│   ├── Sandbox.cpp
│   ├── Namespace.cpp
│   ├── CGroup.cpp
│   ├── Filesystem.cpp
│   ├── Capability.cpp
│   └── Process.cpp
│
├── include/
│
├── rootfs/
│
├── docs/
│
├── Makefile
│
└── README.md
```

---

# Execution Flow

```text
main()

│

├── Parse CLI

├── Create child process

├── Configure namespaces

├── Configure cgroups

├── Prepare root filesystem

├── Mount virtual filesystems

├── Drop capabilities

├── Apply resource limits

└── Execute target program
```

---

# Example

```bash
./OneDay exec /bin/bash
```

or

```bash
./OneDay exec --memory 256M --cpu 50 /usr/bin/python3 script.py
```

---

# Future Features

* Virtual Ethernet (veth)
* Bridge networking
* Overlay filesystem
* OCI bundle support
* Seccomp filters
* AppArmor integration
* Checkpoint & Restore (CRIU)
* Image management
* Layered filesystem
* Runtime daemon

---

# References

* Linux Kernel Documentation
* `man 2 clone`
* `man 2 unshare`
* `man 2 setns`
* `man 7 namespaces`
* `man 7 cgroups`
* *Linux Programming Interface* — Michael Kerrisk
* *Understanding the Linux Kernel*
* *Linux Kernel Development* — Robert Love

---

# Why This Project?

Modern container technologies often hide the complexity of the Linux kernel behind high-level tools. OneDay removes that abstraction by implementing the underlying mechanisms directly. Building these features from scratch provides practical insight into how containers achieve isolation, resource control, and secure execution, making it a valuable systems programming exercise.

---

# License

This project is intended for educational and research purposes.
