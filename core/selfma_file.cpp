#include "selfma_file.h"
#include <filesystem>
#include <string>

const static std::filesystem::path storage{STORAGE_PATH};

void header_print(const header_t* h) {
    fprintf(stderr, "CRC: 0x%08x\n", h->crc);   
    fprintf(stderr, "VER: %d\n", h->version);   
    fprintf(stderr, "MAGIC: %s\n", h->magic);   
    fprintf(stderr, "CHUNKS: %d\n", h->num_of_chunks);   
    fprintf(stderr, "USR_DATA_LENG: %zu\n", h->user_data_length);   
    fprintf(stderr, "FILE: %s\n", h->file_name);   
    fprintf(stderr, "POINTER: 0x%08x\n", h->each_chunk_size);   
}

header_t* get_header_buffer(size_t num_of_chunks) {
    return (header_t*) qwistys_malloc(header_size(num_of_chunks), nullptr);
}

size_t header_size(size_t num_of_chunks) {
    return sizeof(header_t) + sizeof(uint32_t) * num_of_chunks;
}

bool is_storage() {
    return is_exist(STORAGE_PATH);
}

bool is_exist(std::string_view path) {
    return std::filesystem::exists(path);
}

std::string hash_to_file(const std::string& uuid) {
#if 0 
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
#endif
    return uuid;
}

bool handle_security(const char* uuid, std::fstream& fd, bool (*callback)(const char*, std::fstream&)) {
    if (callback)
        return callback(uuid, fd);
    return false;
}
