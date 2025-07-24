<div align="center">
  <h1>
    <img src="./img/shrk.ico">
    RK25 Rootkit
    <img src="./img/shrk.ico">
  </h1>
  <p>Windows kernel-level rootkit adapting to multiple OS versions</p>
  <a href="https://github.com/redrockstyle/rk25"><img src="https://img.shields.io/badge/platform-Windows-blue.svg" alt="Supported Platforms"></a>
  <a href="https://github.com/redrockstyle/rk25/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="License"></a>
  <a href="https://github.com/redrockstyle/rk25"><img src="https://img.shields.io/badge/WDK-2004%2B-orange.svg" alt="WDK Version"></a>
</div>

---

## Project Overview
RK25 is a kernel-mode rootkit driver designed to dynamically adapt to various Windows versions through syscall table hooking and memory management techniques. Built as a WDM driver, it provides stealth capabilities while maintaining compatibility across major Windows updates.

---

## Architecture
```mermaid
graph TD;
    A[RK25 Core] o--o B[Hooking]
    A o--o E[Command Dispatcher]
    A o--o F[Kernel Searcher]
    A o--o G[OS Version Switcher]
    E-->H[IOCTL]
    B-->C[Syscall Hooking]
    B-->D[Network Hooking]
    
    K[EntryPoint]-->|init|F
```

---

## Key Features
  - Automatic OS Version Detection
  - Process hiding
  - Process privilege escalation
  - Network connection hiding
  - ... and something in development :D

---

## Dev Requirements
  - WDK/SDK
  - Visual Studio (driver)
  - Golang (manager)
  - WinDBG
  - OS Windows VM (VMWare etc)

---

## 📄 License
MIT License - see [LICENSE](LICENSE) for details