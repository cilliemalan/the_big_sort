/*
    See LICENSE

    This utility generates random stuff.
*/

#include "common.hpp"
#include "mappedfile.hpp"
#include "check-sorted.h"

static void print_usage()
{
    std::cerr << R"(
checks if a file is the sorted version of another.

    Usage: check-sorted <sorted file> <unsorted file>

The program will print status to stderr and return 0 if the file is sorted.

)";
}

std::string make_string(const char *pl0, int cl0)
{
    std::string s;
    s.assign(pl0, cl0);
    return s;
}

static inline bool is_uppercase(char c) { return c >= 65 && c <= 90; }

static inline std::uint64_t line_length(const mapped_file &fsorted, std::uint64_t line_start)
{
    for (std::uint64_t len = line_start; len < fsorted.size(); len++)
    {
        if (fsorted[len] == '\n') return len;
    }

    return fsorted.size() - line_start;
}

void print_unsorted_message(const mapped_file &fsorted, std::uint64_t line_p, std::uint64_t line_c, std::uint64_t line_cnt)
{
    std::cerr << "\n\nFound unsorted lines: lines #"
        << line_cnt << " and #" << line_cnt + 1 << ":\n\n";

    std::string l0, l1;
    l0.assign(&fsorted[line_p], line_length(fsorted, line_p));
    l1.assign(&fsorted[line_c], line_length(fsorted, line_c));
    std::cerr << l0 << "\n"
        << l1 << "\n\n"
        << "NOT SORTED\n";
}

static inline void scan_for_newline(uint64_t &index, const uint64_t &file_size, mapped_file &fsorted)
{
    for (;;)
    {
        std::uint64_t _r = *reinterpret_cast<std::uint64_t*>(&fsorted[index]);
        _r ^= 0x0a0a0a0a0a0a0a0a;

        int stride = 0;
        for (; stride < 8; stride++)
        {
            if (!(_r & 0xFF)) break;
            _r >>= 8;
        }
        index += stride;
        if (stride < 8) return;
        //do { index++; } while (index < file_size && fsorted[index] != '\n');
    }
}

static inline void newline_found(uint64_t &line_p, uint64_t &line_c, uint64_t &line_p_len, const uint64_t &index, uint64_t &line_cnt)
{
    line_p = line_c;
    line_p_len = index - line_p;
    line_c = index + 1;
    line_cnt++;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage();
    }

    try
    {
        // memory map the file to check
        mapped_file fsorted(argv[1]);
        std::uint64_t lines = 0;
        std::uint64_t line_n0 = 0;
        std::uint64_t line_n1 = 0;
        std::uint64_t file_size = fsorted.size();
        std::uint64_t index = 0;
        std::uint64_t line_p_len = 0;
        std::uint64_t line_p = 0;
        std::uint64_t line_c = 0;
        std::uint64_t line_cnt = 0;

        // skip blank lines at start of file
        for (; index < file_size; index++)
        {
            char c = fsorted[index];
            if (c != '\n')
            {
                line_p = index;
                break;
            }
            else
            {
                line_cnt++;
            }
        }

        // find the first newline
        for (; index < file_size; index++)
        {
            char c = fsorted[index];
            if (c == '\n')
            {
                line_p_len = index - line_p;
                line_c = ++index;
                line_cnt++;
                break;
            }
        }

        // no newline found
        if (index >= file_size)
        {
            std::cerr << "\n\nONLY ONE LINE\n\n";
            return 0;
        }


        for (; index < file_size; index++)
        {
            char p = fsorted[index - line_p_len - 1];
            char c = fsorted[index];

            if (c == '\n')
            {
                if (p != '\n')
                {
                    // reached \n before previous line
                    print_unsorted_message(fsorted, line_p, line_c, line_cnt);
                    return 1;
                }
                else
                {
                    newline_found(line_p, line_c, line_p_len, index, line_cnt);
                }
            }
            else
            {
                if (p == '\n')
                {
                    // reached end of previous line, scan for \n
                    scan_for_newline(index, file_size, fsorted);
                    newline_found(line_p, line_c, line_p_len, index, line_cnt);
                }
                else
                {
                    if (p > c)
                    {
                        char up = (is_uppercase(p) ? p + 32 : p);
                        char uc = (is_uppercase(c) ? c + 32 : c);

                        if (up > uc)
                        {
                            // reached \n before previous line
                            print_unsorted_message(fsorted, line_p, line_c, line_cnt);
                            return 1;
                        }
                        else if (up < uc)
                        {
                            // line success, scan for newline
                            scan_for_newline(index, file_size, fsorted);
                            newline_found(line_p, line_c, line_p_len, index, line_cnt);
                        }
                    }
                    else if (p < c)
                    {
                        // line success, scan for newline
                        scan_for_newline(index, file_size, fsorted);
                        newline_found(line_p, line_c, line_p_len, index, line_cnt);
                    }
                }
            }
        }

        std::cerr << "\n\nSORTED\n\n";
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}