/*
    See LICENSE

    This utility generates random stuff.
*/


#include "common.hpp"
#include "blockingconcurrentqueue.h"

// the amount of memory for the buffer used to generate 
// random sequences. Will have num processors * 2 - 2
// of these hanging around
static const std::uint64_t memory_buffer_size = 32ull * 1024 * 1024;

// the number of random numbers to generate
static const int random_number_count = 8 * 1024 * 1024;

// the maximum length of a line (in words)
static const int max_line_length = 30;

// the probability of capitalizing a word (p = 1/capitalization_coefficient)
static const int capitalization_coefficient = 10;

// this string contains a bajillion different words
extern std::string all_words_string;

// a helper class for a word which points to a a place in all_words_string.
struct Word
{
    Word(const char *pointer, int length) : pointer(pointer), length(length) {}
    inline void copy(char *buffer)
    {
        std::memcpy(buffer, pointer, length);
    }

    const char *pointer;
    int length;
};

// turn all_words_string into pointers to the discrete words.
static std::vector<Word> read_all_words()
{
    std::vector<Word> result;
    const char *last_ptr = &all_words_string[0];
    int last_size = 0;
    for (size_t i = 0; i < all_words_string.size(); i++)
    {
        if (all_words_string[i] == '\n')
        {
            result.emplace_back(last_ptr, last_size);
            last_size = 0;
            last_ptr = &all_words_string[i + 1];
        }
        else
        {
            last_size++;
        }
    }

    return result;
}

// generates a sequence of random numbers. A small amount of random
// numbers (like 8 million) are generated and iterated through.
static std::vector<int> generate_randoms()
{
    std::mt19937 generator;
    std::vector<int> randoms(random_number_count);
    for (size_t i = 0; i < randoms.size(); i++)
    {
        randoms[i] = generator();
        if (randoms[i] < 0)
            randoms[i] = -randoms[i];
    }

    return randoms;
}

// the buffer containing all words.
static const std::vector<Word> all_words;

static void capitalize(char *c)
{
    if ((*c > 96) && (*c < 123))
        *c -= 32;
}


// random number generating stuff.
static std::mt19937 rnd_seeder;
// the buffer of random numbers.
static const std::vector<int> randoms(generate_randoms());
static thread_local int rnd_ri = 0;
static thread_local int rnd_cnt = 0;
static thread_local int rnd_mod = 0;
static int rnd(int max)
{
    if(--rnd_cnt <= 0)
    {
        // reseed
        rnd_ri = rnd_seeder();
        rnd_mod = rnd_seeder();
        rnd_cnt = random_number_count;
    }

    int r = ((randoms[++rnd_ri % randoms.size()] ^ rnd_mod) % (max));
    if (r < 0) r = -r;
    return r;
}

static std::vector<char> generate_buffer(int size)
{
    std::vector<char> buffer(size);
    int buffer_size = 0;
    int next_endl = rnd(max_line_length);
    for (;;)
    {
        int word_ix = rnd(all_words.size());
        auto word = all_words[word_ix];
        auto space_left = buffer.size() - buffer_size;
        if (space_left < static_cast<size_t>(word.length + 2))
        {
            buffer[buffer_size++] = '\n';
            break;
        }

        word.copy(&buffer[buffer_size]);
        if (rnd(capitalization_coefficient) == 0)
        {
            // capitalize randomly
            capitalize(&buffer[buffer_size]);
        }
        buffer_size += word.length;

        if (--next_endl <= 0)
        {
            buffer[buffer_size++] = '\n';
            next_endl = rnd(max_line_length);
        }
    }

    buffer.resize(buffer_size);
    return buffer;
}

static void print_usage()
{
    std::cerr << R"(
generates a file with random stuff.

    Usage: generate <amount in gigabytes>

The program will print to stdout so run it e.g. like this:

    # will generate ~3Gb of random data and store in /tmp/wut.dat
    generate 3 > /tmp/wut.dat

)";
}

void write_progress(int n, int total)
{
    int progress = (100 * n) / total;
    std::cerr << "\r" << progress << "%";
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage();
        return 1;
    }

    int num_gigabytes = std::atoi(argv[1]);
    if (num_gigabytes < 1)
    {
        print_usage();
        return 2;
    }

    if (num_gigabytes > 500)
    {
        std::cerr << "The program has a hardcoded limit of 500 gigabytes. If you want more than that you're going to have to modify the source code.";
        return 3;
    }

    // on first call the randmo number generator will reseed.
    rnd(1);

    *const_cast<std::vector<Word> *>(&all_words) = read_all_words();
    int buffers_to_write = static_cast<int>((1024ull * 1024 * 1024 * num_gigabytes) / memory_buffer_size);
    int buffers_written = 0;
    std::atomic<int> buffers_generating;
    std::atomic<int> total_buffers_generated;
    buffers_generating = 0;
    total_buffers_generated = 0;
    std::uint64_t total_bytes_written = 0;

    // using almost all the threads to generate random sequences.
    int numThreads = std::thread::hardware_concurrency() - 2;
    if (numThreads <= 0) numThreads = 1;
    
    // borrowing moodycamel's cross platform semaphore.
    // we can generate max numThreads * 2 buffers before we have to wait
    // for them to be written to disk
    moodycamel::details::mpmc_sema::Semaphore semaphore(numThreads * 2);

    // the queue of generated buffers to be written to disk
    moodycamel::BlockingConcurrentQueue<std::vector<char>> queue;

    // kick off the generating threads
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back([&]() {
            while (++total_buffers_generated <= buffers_to_write)
            {
                semaphore.wait();
                auto buffer = generate_buffer(memory_buffer_size);
                queue.enqueue(std::move(buffer));
                semaphore.signal();
            }
        });
    }

    // don't know how to write binary to cout so using c-style write instead
    // set up output buffer
    std::vector<char> line_buffer(256 * 1024 * 1024);
    setvbuf(stdout, &line_buffer[0], _IOFBF, line_buffer.size());

#ifdef WIN32
    // windows is really pesky when it comes to newline conversion, so we need to
    // manually switch to binary mode.
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    while (buffers_written < buffers_to_write)
    {
        // get the generated buffer
        std::vector<char> buffer;
        queue.wait_dequeue(buffer);

        // interim progress
        write_progress((buffers_written*2) + 1, buffers_to_write * 2);
        
        // write
        fwrite(&buffer[0], 1, buffer.size(), stdout);
        fflush (stdout);

        buffers_written++;
        write_progress(buffers_written, buffers_to_write);
        total_bytes_written += buffer.size();
        continue;
    }

    for (auto &t : threads)
        t.join();

    std::cerr << "\nwrote " << total_bytes_written << " bytes.\n";

    fclose (stdout);
    return 0;
}
