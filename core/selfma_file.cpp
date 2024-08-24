#include "selfma_file.h"
#include <openssl/evp.h>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include "qwistys_macros.h"

namespace fs = std::filesystem;

const static std::filesystem::path storage{STORAGE_PATH};

bool is_storage() {
    return is_exist(STORAGE_PATH);
}

bool is_exist(std::string_view path) {
    return std::filesystem::exists(path);
}

std::string hash_to_file(const std::string& uuid) {
    // 1. Hash the UUID using EVP API
    std::vector<unsigned char> hash(EVP_MAX_MD_SIZE);
    unsigned int hash_len;

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == nullptr) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to initialize SHA-256 context");
    }

    if (EVP_DigestUpdate(mdctx, uuid.c_str(), uuid.length()) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to update digest");
    }

    if (EVP_DigestFinal_ex(mdctx, hash.data(), &hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        throw std::runtime_error("Failed to finalize digest");
    }

    EVP_MD_CTX_free(mdctx);

    // 2. Convert hash to hex string
    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    std::string hash_str = ss.str();

    // 3. Create directory structure (using first 4 characters of hash)
    fs::path dir_path = storage; // hash_str.substr(0, 2) / hash_str.substr(2, 2);

    // 4. Create full file path
    fs::path file_path = dir_path / (uuid + ".dat");
    QWISTYS_DEBUG_MSG("FILE TO HASH %s uuid %s", file_path.c_str(), uuid.c_str());
    return file_path.string();
}

bool handle_security(const char* uuid, std::fstream& fd, bool (*callback)(const char*, std::fstream&)) {
    if (callback)
        return callback(uuid, fd);
    return false;
}
