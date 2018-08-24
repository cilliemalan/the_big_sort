/*
    See LICENSE

    This utility sorts.
*/

#include "common.hpp"
#include "mappedfile.hpp"

using namespace std;

static void print_usage()
{
    cerr << R"(
Sorts a file

    Usage: sorter <unsorted file> <sorted file>

)";
}

static inline constexpr bool is_uppercase(char c) { return c >= 65 && c <= 90; }
static inline uint64_t key_for(const mapped_file &fsorted, uint64_t line, uint64_t len)
{
    uint64_t key = 0;
    for (int i = 0; i < 8; i++)
    {
        char c = static_cast<uint32_t>(i) >= len ? 0 : fsorted[line + i];
        key <<= 8;
        key |= is_uppercase(c) ? c + 32 : c;
    }

    return key;
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
        mapped_file funsorted(argv[2], false, fsorted.size());
        vector<pair<uint64_t,uint64_t>> all_lines;

        cerr << "counting lines...";
        uint64_t last_line_start = 0;
        auto file_size = funsorted.size();
        for (uint64_t i = 0; i < file_size; i++)
        {
            auto c = fsorted[i];

            if(c == '\n')
            {
                auto key = key_for(funsorted, last_line_start, i - last_line_start);
                all_lines.emplace_back(make_pair(key, last_line_start));
            }
        }

        cerr << "found " << all_lines.size() << " lines";
    }
    catch (exception &e)
    {
        cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}