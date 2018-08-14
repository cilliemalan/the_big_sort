#pragma once

class mapped_file
{
  public:
    mapped_file(std::string filename, bool readonly = true);
    ~mapped_file();

    char &operator[](size_t pos) { return *(pointer + pos); }
    const char &operator[](size_t pos) const { return *(pointer + pos); }
  private:
    char *pointer;
#if defined(__unix__)
    FILE fd;
#elif defined(WIN32)
    HANDLE mapping_handle, file_handle;
#else
#error Memory mapping functions not implemented for this platform.
#endif
};