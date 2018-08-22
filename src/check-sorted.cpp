/*
    See LICENSE

    This utility generates random stuff.
*/

#include "common.hpp"
#include "mappedfile.hpp"

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

uint64_t last_progress = 1000;
static inline void progress(uint64_t x, uint64_t total)
{
    if (x > total) x = total;
    auto nx = x * 100;
    auto prog = nx / total;
    if (last_progress != prog)
    {
        last_progress = prog;
        std::cerr << "\r" << prog << "%" << std::flush;

        if (prog >= 100)
        {
            std::cerr << "\n";
        }
        else
        {
            std::cerr << std::flush;
        }
    }
}

static std::vector<int> generate_randoms()
{
    std::mt19937 generator;
    std::vector<int> randoms(1000000);
    for (size_t i = 0; i < randoms.size(); i++)
    {
        randoms[i] = generator();
        if (randoms[i] < 0)
            randoms[i] = -randoms[i];
    }

    return randoms;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        print_usage();
        return 1;
    }

    try
    {
        // memory map the file to check
        mapped_file fsorted(argv[1]);
        mapped_file funsorted(argv[2]);

        // the files have to be  the same length
        if (fsorted.size() != funsorted.size())
        {
            std::cerr << "The sorted and unsorted files are not equal in size\n" << "\n\n"
                << "NOT SORTED\n";
            return 1;
        }

        std::uint64_t lines = 0;
        std::uint64_t line_n0 = 0;
        std::uint64_t line_n1 = 0;
        std::uint64_t file_size = fsorted.size();
        std::uint64_t index = 0;
        std::uint64_t line_p_len = 0;
        std::uint64_t line_p = 0;
        std::uint64_t line_c = 0;
        std::uint64_t line_cnt = 0;
        std::vector<int> randoms(generate_randoms());
        std::uint64_t rix = 0;

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

        std::cerr << "checking if the file is sorted\n";

        for (; index < file_size; index++)
        {
            char p = fsorted[index - line_p_len - 1];
            char c = fsorted[index];

            bool newline_was_found = false;
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
                    newline_was_found = true;
                }
            }
            else
            {
                if (p == '\n')
                {
                    // reached end of previous line, scan for \n
                    scan_for_newline(index, file_size, fsorted);
                    newline_was_found = true;
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
                            newline_was_found = true;
                        }
                    }
                    else if (p < c)
                    {
                        // line success, scan for newline
                        scan_for_newline(index, file_size, fsorted);
                        newline_was_found = true;
                    }
                }
            }

            if (newline_was_found)
            {
                line_p = line_c;
                line_p_len = index - line_p;
                line_c = index + 1;
                line_cnt++;

                progress(index, file_size);
            }
        }

        progress(1, 1);

        std::cerr << "doing spot checks\n";

        std::uint64_t u_line_p_len = 0;
        std::uint64_t u_line_p = 0;
        std::uint64_t u_line_cnt = 0;
        std::uint64_t u_line_c = 0;
        for (index = 0; index < file_size; index++)
        {
            char c = funsorted[index];

            if (c == '\n')
            {
                u_line_p = u_line_c;
                u_line_p_len = index - u_line_p;
                u_line_c = index + 1;
                u_line_cnt++;

                progress(index, file_size);
            }
        }

        progress(1, 1);

        if (u_line_cnt != line_cnt)
        {
            std::cerr << "\n\nSPOT CHECK FAIL!\nthe sorted file contained a different number of lines!\n\n"
                << "number of lines in unsorted file: " << u_line_cnt << "\n"
                << "number of lines in   sorted file: " << line_cnt << "\n\n";
            return 1;
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