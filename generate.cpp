#include "sort.hpp"

const std::uint64_t memory_buffer_size = 512*1024*1024;
const std::uint64_t to_write = 1024*1024*1024;

extern const std::string all_words_string;

struct word
{
    word(const char* pointer, int length):pointer(pointer),length(length) {}
    inline void copy(char* buffer)
    {
        std::memcpy(buffer, pointer, length);
    }

    const char* pointer;
    int length;
};

std::vector<word> read_all_words()
{
    std::vector<word> result;
    const char* last_ptr = &all_words_string[0];
    int last_size = 0;
    for(size_t i = 0; i<all_words_string.size();i++)
    {
        if(all_words_string[i] == '\n')
        {
            result.emplace_back(last_ptr,last_size);
            last_size = 0;
            last_ptr = &all_words_string[i+1];
        }
        else{
        last_size++;
        }
    }

    return result;
}

int main(int argc, char *argv[])
{
    std::vector<word> all_words(read_all_words());

    std::mt19937 generator;
    std::uint64_t total_written = 0;
    std::vector<int> randoms(128*1024*1024);
    for(size_t i=0;i<randoms.size();i++)
    {
        randoms[i] = generator();
    }

    int ri = 0;
    std::vector<char> buffer(memory_buffer_size);
    while (total_written < to_write)
    {
        int buffer_size = 0;

        int next_endl = randoms[++ri%randoms.size()] % 20;
        for (;;)
        {
            int word_ix = randoms[++ri%randoms.size()] % all_words.size();
            auto word = all_words[word_ix];
            auto space_left = buffer.size() - buffer_size;

            if (space_left < static_cast<size_t>(word.length + 2))
            {
                buffer[buffer_size++] = '\n';
                break;
            }

            word.copy(&buffer[buffer_size]);
            buffer_size += word.length;

            if (--next_endl == 0)
            {
                buffer[buffer_size++] = '\n';
                next_endl = randoms[++ri%randoms.size()] % 20;
            }
        }

        std::cout.write(&buffer[0], buffer_size);
        total_written += buffer_size;

        std::cerr << "\r" << ((total_written * 100) / to_write) << "%    ";
    }

    std::cout << std::endl;
    std::cerr << "\rdone         \n";

    return 0;
}
