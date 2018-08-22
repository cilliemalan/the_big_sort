/*
    See LICENSE

    This utility generates random stuff.
*/

#include "common.hpp"
#include "mappedfile.hpp"

using namespace std;
constexpr uint64_t index_page_size = 4096;


static void print_usage()
{
    cerr << R"(
checks if a file is the sorted version of another.

    Usage: check-sorted <sorted file> <unsorted file>

The program will print status to stderr and return 0 if the file is sorted.

)";
}

static inline bool is_uppercase(char c) { return c >= 65 && c <= 90; }

static inline uint64_t line_length(const mapped_file &fsorted, uint64_t line_start)
{
    for (uint64_t len = line_start; len < fsorted.size(); len++)
    {
        if (fsorted[len] == '\n') return len;
    }

    return fsorted.size() - line_start;
}

void print_unsorted_message(const mapped_file &fsorted, uint64_t line_p, uint64_t line_c, uint64_t line_cnt)
{
    cerr << "\n\nFound unsorted lines: lines #"
        << line_cnt << " and #" << line_cnt + 1 << ":\n\n";

    string l0, l1;
    l0.assign(&fsorted[line_p], line_length(fsorted, line_p));
    l1.assign(&fsorted[line_c], line_length(fsorted, line_c));
    cerr << l0 << "\n"
        << l1 << "\n\n"
        << "NOT SORTED\n";
}

static inline void scan_for_newline(uint64_t &index, const uint64_t &file_size, mapped_file &fsorted)
{
    for (;;)
    {
        uint64_t _r = *reinterpret_cast<uint64_t*>(&fsorted[index]);
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
        cerr << "\r" << prog << "%" << flush;

        if (prog >= 100)
        {
            cerr << "\n";
        }
        else
        {
            cerr << flush;
        }
    }
}

static vector<int> generate_randoms()
{
    mt19937 generator;
    vector<int> randoms(1000000);
    for (size_t i = 0; i < randoms.size(); i++)
    {
        randoms[i] = generator();
        if (randoms[i] < 0)
            randoms[i] = -randoms[i];
    }

    return randoms;
}

static uint64_t key_for(const mapped_file &fsorted, uint64_t line, uint64_t len)
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

static inline const void* strstr_s(const void *l, uint64_t l_len, const void *s, uint64_t s_len)
{
    char *cur, *last;
    const char *cl = (const char *)l;
    const char *cs = (const char *)s;

    if (l_len == 0 || s_len == 0) return nullptr;

    if (l_len < s_len) return nullptr;
    if (s_len == 1) return memchr(l, (int)*cs, l_len);

    last = (char *)cl + l_len - s_len;

    for (cur = (char *)cl; cur <= last; cur++)
    {
        if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
        {
            return cur;
        }
    }

    return nullptr;
}

static inline bool find_line(const char* start, const char* end, char* lookingfor, uint64_t lookingfor_len)
{
    return strstr_s(start, static_cast<uint64_t>(end - start), lookingfor, lookingfor_len);
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
            cerr << "The sorted and unsorted files are not equal in size\n"
                << string(argv[1]) << " is " << fsorted.size() << " bytes\n"
                << string(argv[2]) << " is " << funsorted.size() << " bytes\n"
                << "\n\n"
                << "NOT SORTED\n";
            return 1;
        }

        uint64_t file_size = fsorted.size();
        uint64_t index = 0;
        uint64_t line_p_len = 0;
        uint64_t line_p = 0;
        uint64_t line_c = 0;
        uint64_t line_cnt = 0;
        vector<int> randoms(generate_randoms());
        uint64_t rix = 0;

        // we'll index the sorted file as we go along
        uint64_t prev_key = 0;
        uint64_t prev_key_ix = 0;
        map<uint64_t, char*> sorted_index;
        sorted_index[0] = &fsorted[0];

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
            cerr << "\n\nONLY ONE LINE\n\n";
            return 0;
        }

        cerr << "checking if the file is sorted\n";

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

                if (line_cnt % 10 == 0) progress(index, file_size);

                auto key = key_for(fsorted, line_p, line_p_len);
                if (key != prev_key && prev_key - key > 0xffffffff)
                {
                    if (index - prev_key_ix > index_page_size)
                    {
                        sorted_index[key] = &fsorted[line_p];
                        prev_key_ix = index;
                    }
                }
                prev_key = key;
            }
        }

        progress(1, 1);

        cerr << "doing spot checks\n";

        uint64_t u_line_p_len = 0;
        uint64_t u_line_p = 0;
        uint64_t u_line_cnt = 0;
        uint64_t u_line_c = 0;
        uint64_t num_checks = 0;
        for (index = 0; index < file_size; index++)
        {
            char c = funsorted[index];

            if (c == '\n')
            {
                u_line_p = u_line_c;
                u_line_p_len = index - u_line_p;
                u_line_c = index + 1;
                u_line_cnt++;

                if (u_line_cnt % 10 == 0) progress(index, file_size);

                if (u_line_p_len && randoms[rix++%randoms.size()] < 2000000)
                {
                    auto key = key_for(funsorted, u_line_p, u_line_p_len);
                    auto search_end = sorted_index.lower_bound(key);
                    auto search_start = search_end;
                    search_start--;
                    if (search_end != sorted_index.end() && key == search_end->first) search_end++;
                    auto ptr_start = search_start->second;
                    auto ptr_end = search_end != sorted_index.end()
                        ? search_end->second
                        : &fsorted[file_size];
                    auto found = find_line(ptr_start, ptr_end, &funsorted[u_line_p], u_line_p_len);

                    if (!found)
                    {
                        string line;
                        line.assign(&funsorted[u_line_p], u_line_p_len);
                        cerr << "\n\nSPOT CHECK FAIL!\ncould not find a line in the sorted file:\n\n"
                            << line << "\n";
                        return 1;
                    }
                    num_checks++;
                }
            }
        }

        progress(1, 1);
        cerr << "did " << num_checks << " successful spot checks\n";

        if (u_line_cnt != line_cnt)
        {
            cerr << "\n\nLINE COUNT FAIL!\nthe sorted file contained a different number of lines!\n\n"
                << "number of lines in unsorted file: " << u_line_cnt << "\n"
                << "number of lines in   sorted file: " << line_cnt << "\n\n";
            return 1;
        }

        cerr << "\n\nSORTED\n\n";
    }
    catch (exception &e)
    {
        cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}