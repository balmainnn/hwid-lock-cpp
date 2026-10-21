#pragma once
#include <string>
namespace hwidlock {
std::string machine_fingerprint(const std::string& app_namespace);
bool activate_or_validate_local(const std::string& license_path,
                                const std::string& app_namespace,
                                std::string& message);
} // namespace hwidlock
