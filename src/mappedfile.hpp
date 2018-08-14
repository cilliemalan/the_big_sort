#pragma once

class mapped_file
{
  public:
    mapped_file(std::string filename, bool readonly = true);
    ~mapped_file();

    inline char &operator[](size_t pos) { return *(pointer + pos); }
    inline const char &operator[](size_t pos) const { return *(pointer + pos); }
    inline std::uint64_t size() const { return filesize; }
  private:
    char *pointer;
    std::uint64_t filesize;
#if defined(__unix__)
    FILE fd;
#elif defined(WIN32)
    HANDLE mapping_handle, file_handle;
#else
#error Memory mapping functions not implemented for this platform.
#endif
};