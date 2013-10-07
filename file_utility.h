#ifndef FILE_UTILITY_H
#define FILE_UTILITY_H
#include <string>

std::string WorkingDir();
std::string NormalizePath(const std::string& path);
std::string PathRemoveFileName(const std::string& path);
std::string PathRemoveFolder(const std::string& path);
int FileSize(const std::string& path);
int ReadFile(const std::string& path, void* buffer, int buffer_len);

#endif