/*
    See LICENSE

    This utility sorts.
*/

#include "common.hpp"
#include "mappedfile.hpp"

using namespace std;

typedef vector<pair<uint64_t, uint64_t>> keys_t;

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
        if (c == '\n') return key;
        key <<= 8;
        key |= is_uppercase(c) ? c + 32 : c;
    }

    return key;
}

static uint64_t partition_k(keys_t &A, int64_t lo, int64_t hi)
{
    auto pivot = A[hi].first;
    auto i = lo;
    for (auto j = lo; j < hi; j++)
    {
        if (A[j].first < pivot) {
            if (i != j) swap(A[i], A[j]);
            i++;
        }
    }
    swap(A[i], A[hi]);
    return i;
}

static void quicksort_k(keys_t &A, int64_t lo, int64_t hi)
{
    if (lo < hi)
    {
        auto p = partition_k(A, lo, hi);
        quicksort_k(A, lo, p - 1);
        quicksort_k(A, p + 1, hi);
    }
}

static inline int compare_v(mapped_file &f, uint64_t a, uint64_t b)
{
    char* pa = &f[a];
    char* pb = &f[b];
    size_t i = 0;
    for (;;)
    {
        auto ea = i + a >= f.size() || pa[i] == '\n';
        auto eb = i + b >= f.size() || pb[i] == '\n';

        if (ea && !eb) return -1;
        if (eb && !ea) return 1;
        if (ea && eb) return 0;

        auto ca = is_uppercase(pa[i]) ? pa[i] + 32 : pa[i];
        auto cb = is_uppercase(pb[i]) ? pb[i] + 32 : pb[i];

        if (ca < cb) return -1;
        if (ca > cb) return 1;
        i++;
    }
}

static uint64_t partition_v(mapped_file &f, keys_t &A, int64_t lo, int64_t hi)
{
    auto pivot = A[hi];
    auto i = lo;
    for (auto j = lo; j < hi; j++)
    {
        if (hi == j) continue;
        auto less = A[j].first < pivot.first
            ? true
            : (A[j].first != pivot.first
                ? false
                : compare_v(f, A[j].second, pivot.second) < 0);
        if (less) {
            if (i != j) swap(A[i], A[j]);
            i++;
        }
    }
    swap(A[i], A[hi]);
    return i;
}

static void quicksort_v(mapped_file &f, keys_t &A, int64_t lo, int64_t hi)
{
    if (lo < hi)
    {
        auto p = partition_v(f, A, lo, hi);
        quicksort_v(f, A, lo, p - 1);
        quicksort_v(f, A, p + 1, hi);
    }
}

static inline void sort_k(keys_t &A)
{
    quicksort_k(A, 0, static_cast<int64_t>(A.size()) - 1);
}

static inline void sort_v(mapped_file &f, keys_t &A)
{
    quicksort_v(f, A, 0, static_cast<int64_t>(A.size()) - 1);
}

static inline size_t cpy(const char* src, char* dst, size_t max)
{
    for (size_t i = 0; i < max; i++)
    {
        auto c = src[i];
        dst[i] = c;
        if (c == '\n') return i + 1;
    }

    return max;
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
        mapped_file funsorted(argv[1]);
        auto file_size = funsorted.size();
        mapped_file fsorted(argv[2], false, file_size);
        keys_t all_lines;
        all_lines.reserve(file_size / 150);

        cerr << "counting lines...\n";
        uint64_t last_line_start = 0;
        for (uint64_t i = 0; i < file_size; i++)
        {
            auto c = funsorted[i];

            if (c == '\n')
            {
                auto key = key_for(funsorted, last_line_start, i - last_line_start);
                all_lines.emplace_back(make_pair(key, last_line_start));
                last_line_start = i + 1;
            }
        }

        auto num_lines = all_lines.size();

        cerr << "indexed " << num_lines << " lines\n";
        cerr << "sorting keys...\n";
        sort_v(funsorted, all_lines);
        cerr << "done!\n";

        uint64_t ix = 0;
        uint64_t prev_key = 0;
        uint64_t prev_pos = 0;
        uint64_t dst_pos = 0;

        while (ix < num_lines)
        {
            auto pos = all_lines[ix].second;
            dst_pos += cpy(&funsorted[pos], &fsorted[dst_pos], file_size - pos);
            ix++;
        }
    }
    catch (exception &e)
    {
        cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}