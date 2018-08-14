#include "common.hpp"
#include "mappedfile.hpp"

mapped_file::mapped_file(std::string filename, bool readonly)
{
#if defined(__unix__)
#elif defined(WIN32)

    // open the file
    file_handle = CreateFileA(filename.c_str(),
                              readonly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              0,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              0);
    if (!file_handle || file_handle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("could not open file");
    }

    // get its size
    DWORD hi_size, lo_size;
    lo_size = GetFileSize(file_handle, &hi_size);

    // map it
    mapping_handle = CreateFileMapping(
        file_handle,
        0,
        readonly ? PAGE_READONLY : PAGE_READWRITE,
        hi_size,
        lo_size,
        0);

    if (!mapping_handle || mapping_handle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(file_handle);
        throw std::runtime_error("could not map file");
    }

    // get the pointer
    pointer = static_cast<char *>(::MapViewOfFile(
        mapping_handle,
        readonly ? FILE_MAP_READ : FILE_MAP_WRITE,
        0, 0,
        static_cast<size_t>(lo_size) | (static_cast<size_t>(hi_size) << 32)));

    if (!pointer)
    {
        CloseHandle(mapping_handle);
        CloseHandle(file_handle);
        throw std::runtime_error("could not map file");
    }
#endif
}

mapped_file::~mapped_file()
{
#if defined(__unix__)
#elif defined(WIN32)
    CloseHandle(mapping_handle);
    CloseHandle(file_handle);
    pointer = nullptr;
#endif
}
