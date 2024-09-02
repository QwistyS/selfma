#ifndef SELFMA_FILE_H
#define SELFMA_FILE_H

#include <cstddef>
#include <fstream>
#include <string>
#include <string_view>
#include "qwistys_alloc.h"
#include "task.h"

#ifndef STORAGE_PATH
#    define STORAGE_PATH "none"
#endif

typedef struct {
    uint32_t crc;                              // Do i need to check the integrety of data from storaje ?
    uint32_t version;                          // File format version
    char magic[4];                             // Magic number for file identification
    uint8_t num_of_chunks;                     // Number of main tree nodes
    size_t user_data_length;                   // Length of user data (max 1024 bytes)
    char user_buffer[MAX_DESCRIPTION_LENGTH];  // user buffer
    char file_name[MAX_NAME_LENGTH];           // user buffer
    uint32_t each_chunk_size[];                // User identification (SHA256)
} header_t;

void header_print(const header_t* h);

size_t header_size(size_t num_of_chunks);
header_t* get_header_buffer(size_t num_of_chunks);

// RAII wrapper for header
class HeaderGuard {
    header_t* header;

public:
    explicit HeaderGuard(size_t size) : header(get_header_buffer(size)) {}
    HeaderGuard() : header(nullptr) {}
    ~HeaderGuard() {
        if (header)
            qwistys_free(header);
    }
    header_t* get() { return header; }
    header_t* operator->() { return header; }
    void set_seze(size_t size) { header = get_header_buffer(size); }
};

// RAII wrapper for file
class FileGuard {
    std::fstream file;
public:
    FileGuard(const std::string& filename, std::ios_base::openmode mode) : file(filename, mode) {}
    ~FileGuard() {
        if (file.is_open())
            file.close();
    }
    bool is_open() const { return file.is_open(); }
    std::fstream& get() { return file; }
};

bool is_storage();
std::string hash_to_file(const std::string& uuid);
bool is_exist(std::string_view path);
bool handle_security(const char* uuid, std::fstream& fd, bool (*callback)(const char*, std::fstream&));

#endif  // SELFMA_FILE_H
