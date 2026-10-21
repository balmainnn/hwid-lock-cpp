#include "hwid_lock.h"
#include <exception>
#include <iostream>
#include <string>
int main(int argc, char** argv) {
    const std::string path = argc > 1 ? argv[1] : "license.dat";
    const std::string app_namespace = "example-cheat-client";
    try {
        std::string message;
        bool valid = hwidlock::activate_or_validate_local(path, app_namespace, message);
        std::cout << (valid ? "[OK] " : "[DENIED] ") << message << '\n';
        std::cout << "License file: " << path << '\n';
        std::cout << "Local demo only; this is not tamper-resistant licensing.\\n";
        return valid ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << '\n';
        return 2;
    }
}
