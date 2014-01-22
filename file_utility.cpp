#include "file_utility.h"
#define NOMINMAX 

#include <algorithm>
#include "platform.h"

std::string WorkingDir()
{
	char strBuf[256];
	std::string cwd;
#if defined( _WIN32)
	if(_getcwd(strBuf, sizeof(strBuf)))
#elif defined(__APPLE__)
    if(getcwd(strBuf, sizeof(strBuf)))
#endif
	{
		cwd = std::string(strBuf);
	}

	return cwd;
}

std::string NormalizePath(const std::string& path)
{
	std::string normalizedPath(path);
	
	for(size_t i = 0; i < path.size(); i++)
	{
		if(path[i] == '/' || ( i + 1 < path.size() && path[i] == ':' && !(path[i + 1] == '\\' || path[i + 1] == '/') ))
			normalizedPath[i] = '\\';
		else
			normalizedPath[i] = path[i];
	}

	return normalizedPath;

}

std::string PathRemoveFileName(const std::string& path)
{
	char* strBuf = new char[path.size() + 1];
	memcpy(strBuf, path.c_str(), path.size());
	strBuf[path.size()] = 0;
    
#ifdef _WIN32
	BOOL success = PathRemoveFileSpecA(strBuf);
#endif
	std::string retPath = std::string(strBuf);

	delete [] strBuf;

	return retPath;
}

std::string PathRemoveFolder(const std::string& path)
{
	char* strBuf = new char[path.size() + 1];
	memcpy(strBuf, path.c_str(), path.size());
	strBuf[path.size()] = 0;
#ifdef _WIN32
	PathStripPathA(strBuf);
#endif
	std::string retPath(strBuf);

	delete [] strBuf;

	return retPath;
}

int FileSize(const std::string& path)
{
	FILE* fp = fopen(path.c_str(), "rb");

	if(fp == NULL)
		return 0;

	fseek(fp, 0, SEEK_END);
	int seekLen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	fclose(fp);

	return seekLen;
}

int ReadFile(const std::string& path, void* buffer, int buffer_len)
{
	FILE* fp = fopen(path.c_str(), "rb");

	if(fp == NULL)
		return 0;

	fseek(fp, 0, SEEK_END);
	int seekLen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	int readLen = fread(buffer, 1, std::min(seekLen, buffer_len), fp);
	fclose(fp);

	return readLen;
}
