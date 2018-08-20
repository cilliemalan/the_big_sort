#include <iostream>
#include <vector>

using namespace std;
typedef intptr_t elem_t;
constexpr size_t increment = 1024 * 1024;
constexpr size_t gigabyte = 1024 * 1024 * 1024;

int main(int, char **)
{
    cout << "purging all shared and cached memory...\n";
    size_t total = 0;
    {
        vector<elem_t *> blocks;
        try
        {
            for (;;)
            {
                auto block = new elem_t[increment];
                for (size_t i = 0; i < increment; i++)
                    block[i] = 0;
                blocks.push_back(block);
                total += increment * sizeof(elem_t);
                if ((total % gigabyte) == 0)
                {
                    cout << "\rpurged " << (total / gigabyte) << " gigabytes of memory";
                }
            }
        }
        catch (...)
        {
        }
    }

    cout << "\rpurged " << (total / gigabyte) << " gigabytes of memory\n";
}