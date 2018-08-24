#include "common.hpp"
#include "mappedfile.hpp"

mapped_file::mapped_file(std::string filename, bool readonly, size_t size)
    :filesize(size)
{
#if defined(__unix__)

    // open file
    file_handle = open(filename.c_str(),
        readonly ? O_RDONLY : O_RDWR);

    if (file_handle == -1)
    {
        throw std::runtime_error("could not open file");
    }

    if (filesize == 0)
    {
        // get file size
        struct stat filestats;
        auto statresult = fstat(file_handle, &filestats);
        if (statresult != 0)
        {
            close(file_handle);
            throw std::runtime_error("could not stat file");
        }
        filesize = filestats.st_size;
    }
    else
    {
        if (!readonly)
        {
            if (ftruncate(file_handle, filesize))
            {
                close(file_handle);
                throw std::runtime_error("could not truncate file");
            }
        }
    }

    // memory map
    pointer = static_cast<char*>(mmap(
        0,
        filesize,
        readonly ? PROT_READ : PROT_WRITE,
        MAP_SHARED,
        file_handle,
        0));

    if (!pointer || pointer == MAP_FAILED)
    {
        close(file_handle);
        throw std::runtime_error("could not open file");
    }
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

    DWORD hi_size, lo_size;
    if (filesize == 0)
    {
        // get its size
        lo_size = GetFileSize(file_handle, &hi_size);
        filesize = static_cast<size_t>(lo_size) | (static_cast<size_t>(hi_size) << 32);
    }
    else
    {
        hi_size = static_cast<DWORD>(filesize >> 32);
        lo_size = static_cast<DWORD>(filesize & 0xFFFFFFFF);

        if (!readonly)
        {
            LONG lhi_size = *reinterpret_cast<PLONG>(&hi_size);
            LONG llo_size = *reinterpret_cast<PLONG>(&lo_size);
            SetFilePointer(file_handle,
                llo_size,
                &lhi_size,
                FILE_BEGIN);
            SetEndOfFile(file_handle);
            SetFilePointer(file_handle, 0, nullptr, FILE_BEGIN);
        }
    }

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
        filesize));

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
    if (filesize && pointer) munmap(pointer, filesize);
    if (file_handle) close(file_handle);
#elif defined(WIN32)
    if (mapping_handle && mapping_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(mapping_handle);
    }
    if (file_handle && file_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file_handle);
    }
#endif

    pointer = nullptr;
}
