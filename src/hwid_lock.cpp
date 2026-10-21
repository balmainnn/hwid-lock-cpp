#include "hwid_lock.h"
#include <windows.h>
#include <bcrypt.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::wstring read_machine_guid() {
    HKEY key = nullptr;
    LONG rc = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Cryptography", 0,
        KEY_READ | KEY_WOW64_64KEY, &key);
    if (rc != ERROR_SUCCESS)
        throw std::runtime_error("Could not open Windows MachineGuid registry key.");
    wchar_t value[256] = {};
    DWORD type = 0, bytes = sizeof(value);
    rc = RegQueryValueExW(key, L"MachineGuid", nullptr, &type,
        reinterpret_cast<BYTE*>(value), &bytes);
    RegCloseKey(key);
    if (rc != ERROR_SUCCESS || type != REG_SZ || value[0] == L'\0')
        throw std::runtime_error("Could not read Windows MachineGuid.");
    return value;
}
std::string ascii(const std::wstring& value) {
    std::string out;
    for (wchar_t c : value) {
        if (c > 0x7f) throw std::runtime_error("Unexpected non-ASCII MachineGuid.");
        out.push_back(static_cast<char>(c));
    }
    return out;
}
std::string sha256_hex(const std::string& data) {
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD object_len = 0, hash_len = 0, got = 0;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (status < 0) throw std::runtime_error("Could not initialize SHA-256.");
    status = BCryptGetProperty(alg, BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&object_len), sizeof(object_len), &got, 0);
    if (status >= 0) status = BCryptGetProperty(alg, BCRYPT_HASH_LENGTH,
        reinterpret_cast<PUCHAR>(&hash_len), sizeof(hash_len), &got, 0);
    if (status < 0) {
        BCryptCloseAlgorithmProvider(alg, 0);
        throw std::runtime_error("Could not query SHA-256 properties.");
    }
    std::vector<UCHAR> object(object_len), digest(hash_len);
    status = BCryptCreateHash(alg, &hash, object.data(), object_len, nullptr, 0, 0);
    if (status >= 0) status = BCryptHashData(hash,
        reinterpret_cast<PUCHAR>(const_cast<char*>(data.data())),
        static_cast<ULONG>(data.size()), 0);
    if (status >= 0) status = BCryptFinishHash(hash, digest.data(), hash_len, 0);
    if (hash) BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(alg, 0);
    if (status < 0) throw std::runtime_error("SHA-256 hashing failed.");
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (UCHAR b : digest) out << std::setw(2) << static_cast<unsigned int>(b);
    return out.str();
}
std::string trim(std::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
                          s.back() == ' ' || s.back() == '\t')) s.pop_back();
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
    return s;
}
} // namespace

namespace hwidlock {
std::string machine_fingerprint(const std::string& app_namespace) {
    if (app_namespace.empty()) throw std::runtime_error("App namespace must not be empty.");
    // Hash only; the raw MachineGuid is never returned or written to disk.
    return sha256_hex("hwid-lock-demo:v1:" + app_namespace + ":" + ascii(read_machine_guid()));
}
bool activate_or_validate_local(const std::string& path,
                                const std::string& app_namespace,
                                std::string& message) {
    const std::string current = machine_fingerprint(app_namespace);
    std::ifstream in(path, std::ios::binary);
    if (!in.good()) {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) { message = "Could not create license file: " + path; return false; }
        out << "HWIDLOCK_LOCAL_DEMO_V1\n" << current << "\n";
        out.close();
        if (!out) { message = "Failed to write the license file."; return false; }
        message = "Demo license created and bound to this machine.";
        return true;
    }
    std::string header, stored;
    std::getline(in, header);
    std::getline(in, stored);
    stored = trim(stored);
    if (header != "HWIDLOCK_LOCAL_DEMO_V1" || stored.size() != 64) {
        message = "Malformed license file. Delete it to reset the local demo.";
        return false;
    }
    if (stored != current) {
        message = "License belongs to a different machine.";
        return false;
    }
    message = "Local license validated for this machine.";
    return true;
}
} // namespace hwidlock
