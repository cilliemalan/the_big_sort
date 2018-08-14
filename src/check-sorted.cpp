/*
    See LICENSE

    This utility generates random stuff.
*/

#include "common.hpp"
#include "mappedfile.hpp"

static void print_usage()
{
    std::cerr << R"(
checks if a file is sorted.

    Usage: check-sorted <sorted file> [unsorted file]

The program will print status to stderr and return 0 if the file is sorted. If the unsorted
file is provided, the sorted file will be checked for stability as well.

)";
}

std::string make_string(const char *pl0, int cl0)
{
    std::string s;
    s.assign(pl0, cl0);
    return s;
}

inline is_uppercase(char c) { return c >= 65 && c <= 90; }

inline bool compare(const char *pl0, int cl0, const char *pl1, int cl1)
{
    int l = cl0 < cl1 ? cl0 : cl1;
    for (int i = 0; i < l; i++)
    {
        const char &c0 = pl0[i];
        const char &c1 = pl1[i];
        const char uc0 = (is_uppercase(c0) ? c0 + 32 : c0);
        const char uc1 = (is_uppercase(c1) ? c1 + 32 : c1);

        if (uc0 == uc1)
            continue;
        if (uc0 < uc1)
            return true;
        if (uc0 > uc1)
            return false;
    }

    return cl0 <= cl1;
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

        for (std::uint64_t index = 0; index < file_size; index++)
        {
            const char &c = fsorted[index];

            if (c == '\n')
            {
                std::uint64_t line_nn = index + 1;

                if (line_n0 != line_n1)
                {
                    const char *pl0 = &fsorted[line_n0];
                    const char *pl1 = &fsorted[line_n1];

                    const int cl0 = static_cast<int>(line_n1 - line_n0 - 1);
                    const int cl1 = static_cast<int>(line_nn - line_n1 - 1);

                    bool result = compare(pl0, cl0, pl1, cl1);

                    if (!result)
                    {
                        std::cerr << "\n\nFound unsorted lines: lines #"
                                  << lines << " and #" << lines + 1 << ":\n\n";

                        std::string l0, l1;
                        l0.assign(pl0, cl0);
                        l1.assign(pl1, cl1);
                        std::cerr << l0 << "\n"
                                  << l1 << "\n\n"
                                  << "exiting.\n";
                        return 1;
                    }
                }

                lines++;
                line_n0 = line_n1;
                line_n1 = line_nn;
            }
        }

        std::cerr << "\n\nThe file is sorted!\n\n";
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}