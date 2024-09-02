#ifndef SELFMA_FILE_H
#define SELFMA_FILE_H

#include <cstddef>
#include <fstream>
#include <string>
#include <string_view>
#include "qwistys_alloc.h"
#include "qwistys_macros.h"
#include "task.h"

#ifndef STORAGE_PATH
#    define STORAGE_PATH "none"
#endif

// Endianness conversion functions
template<typename T>
inline T to_little_endian(T value) {
    if constexpr (sizeof(T) == 1) return value;
    union { T val; uint8_t bytes[sizeof(T)]; } src, dst;
    src.val = value;
    for (size_t i = 0; i < sizeof(T); ++i)
        dst.bytes[i] = src.bytes[sizeof(T) - 1 - i];
    return dst.val;
}

template<typename T>
inline T from_little_endian(T value) {
    return to_little_endian(value); // The operation is symmetric
}

// Specialized for float and double to handle IEEE 754 format
template<>
inline float to_little_endian(float value) {
    union { float f; uint32_t i; } u;
    u.f = value;
    u.i = to_little_endian(u.i);
    return u.f;
}

template<>
inline double to_little_endian(double value) {
    union { double d; uint64_t i; } u;
    u.d = value;
    u.i = to_little_endian(u.i);
    return u.d;
}

template<>
inline float from_little_endian(float value) {
    return to_little_endian(value);
}

template<>
inline double from_little_endian(double value) {
    return to_little_endian(value);
}

typedef PACKED_STRUCT struct {
    uint32_t crc;                              // Do i need to check the integrety of data from storaje ?
    uint32_t version;                          // File format version
    char magic[4];                             // Magic number for file identification
    uint8_t num_of_chunks;                     // Number of main tree nodes
    size_t user_data_length;                   // Length of user data (max 1024 bytes)
    char user_buffer[MAX_DESCRIPTION_LENGTH];  // user buffer
    char file_name[MAX_NAME_LENGTH];           // user buffer
    uint32_t each_chunk_size[];                // User identification (SHA256)
} header_t;
#ifdef USE_MSVC_PRAGMA_PACK
    #pragma pack(pop)
#endif

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
        QWISTYS_DEBUG_MSG("File closed ");
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
