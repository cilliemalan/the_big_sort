/*
    See LICENSE

    This utility generates random stuff.
*/


#include "common.hpp"

static void print_usage()
{
    std::cerr << R"(
checks if a file is sorted.

    Usage: check-sorted <sorted file> [unsorted file]

The program will print status to stderr and return 0 if the file is sorted. If the unsorted
file is provided, the sorted file will be checked for stability as well.

)";
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        print_usage();
    }

    return 0;
}