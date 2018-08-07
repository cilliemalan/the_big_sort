#include "sort.hpp"

// the size of the memory buffer. Better would be to check avail memory
const std::uint64_t buffer_size = 1024*1024;
const std::string tmp { std::getenv("TEMP") };

static std::uint64_t get_free_temp_space()
{
#ifdef __unix__
    return 1024*1024*1024;
#else
    return 1024*1024*1024;
#endif
}

static void perform_sort(std::ifstream &input, std::ostream &output)
{
    // Step 1: read buffer_size chunks from input and sort to OUTPUT

    // allocate untyped buffer
    std::vector<char> buffer(buffer_size);
}

int main(int argc, char *argv[])
{
    // deal with arguments
    std::string input_file;
    std::string output_file;

    if(argc == 3)
    {
        input_file.assign(argv[1]);
        output_file.assign(argv[2]);
    }
    else
    {
        std::cerr << "Usage:  ./sort <input> <output>" << std::endl;
        return -1;
    }


    std::ifstream input_stream(input_file, std::ios::binary);
    std::ofstream output_stream(output_file, std::ios::trunc | std::ios::binary);

    if(!input_stream.good())
    {
        std::cerr << "Could not open input file." << std::endl;
        return -2;
    }
    
    if(!output_stream.good())
    {
        std::cerr << "Could not open input file." << std::endl;
        return -3;
    }

    return 0;
}
