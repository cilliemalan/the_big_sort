/*
    See LICENSE

    This utility generates random stuff.
*/


#include "common.hpp"

static void print_usage()
{
    std::cerr << R"(
checks if a file is sorted.

    Usage: check-sorted <unsorted file> <sorted file>

The program will print status to stderr and return 0 if the file is stable sorted. The
unsorted file is needed in order to check if the sort was stable.

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