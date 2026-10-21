# HWID Lock — C++ Windows Demo

A small C++17 library demonstrating local machine-bound licensing on Windows. It reads the Windows `MachineGuid`, hashes it with SHA-256 and an application namespace, and stores only the hash in a local license file.

> **Important:** This is a demo, not secure commercial licensing. Since validation and license data are local, users can edit the file or patch the check. Real licensing should validate signed tokens or entitlements on a server.

## Features
- C++17 static library
- Windows CNG (`BCrypt`) SHA-256; no third-party crypto dependency
- Raw MachineGuid is never returned or written to disk
- Product namespace separates fingerprints between integrated applications
- Creates a demo license on first run; validates it on later runs
- CMake build

## Privacy
- `MachineGuid` is a persistent device identifier. Hashing does **not** make it anonymous; the same input and namespace produce the same output.
- Explain the purpose to users, obtain consent where required, publish a privacy policy, and provide a reset/support process.
- Do not upload fingerprints without clear disclosure and a legitimate reason.
- Windows reinstalls, cloning, and system changes can affect device identity. A fingerprint is not infallible proof of identity.
- The app namespace is not a secret or a cryptographic key.

## Build on Windows
Install Visual Studio 2022 or Build Tools with **Desktop development with C++**, including CMake. In PowerShell, from this repository directory:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Run `build\Release\hwid-demo.exe` (single-config generators may put it directly in `build\`).

## Run
```powershell
.\build\Release\hwid-demo.exe
```
First run creates `license.dat` in the current directory. Later runs validate it. An alternate path can be supplied:

```powershell
.\build\Release\hwid-demo.exe .\data\license.dat
```

Delete the chosen `license.dat` to reset this demo. This is a local reset, not an administrator-controlled reactivation system.

## Integrate into a C++ client
Add this repository as a CMake subdirectory or copy `include/hwid_lock.h` and `src/hwid_lock.cpp`. Link `advapi32` and `bcrypt`.

```cpp
#include "hwid_lock.h"
#include <iostream>
#include <string>

int main() {
    std::string message;
    if (!hwidlock::activate_or_validate_local(
            "license.dat", "your-product-id", message)) {
        std::cerr << "License check failed: " << message << '\\n';
        return 1;
    }
    // Continue with your application.
    return 0;
}
```

Use a stable, product-specific namespace. Do not treat it as a secret.

## Publishing checklist
1. Create a public GitHub repository and upload the files, preserving folder structure.
2. Review the included MIT license before publishing.
3. Do not commit build output or generated `license.dat` files.
4. Keep the local-only security limitation prominent.

## Structure
```text
hwid-lock-cpp/
├── CMakeLists.txt
├── include/hwid_lock.h
├── src/hwid_lock.cpp
├── src/main.cpp
├── .gitignore
├── LICENSE
└── README.md
```
