#ifndef SELFMA_FILE_H
#define SELFMA_FILE_H

#include <string>
#include <string_view>

#ifndef STORAGE_PATH
#define STORAGE_PATH "none"
#endif

bool is_storage();
std::string hash_to_file(const std::string& uuid);
bool is_exist(std::string_view path);
bool handle_security(const char* uuid, std::fstream& fd, bool (*callback)(const char*, std::fstream&));

#endif // SELFMA_FILE_H
