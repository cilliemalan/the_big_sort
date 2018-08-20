#include <iostream>
#include <vector>

using namespace std;
constexpr size_t increment = 4*1024*1024;

int main(int, char**)
{
    cout << "purging all shared and cached memory...\n";
    size_t total = 0;
    {
        vector<void*> blocks;
        try
        {
            for(;;)
            {
                auto block = new char[increment];
                memset(block, 0, increment);
                blocks.push_back(block);
                total += increment;
            }
        }
        catch(...) {}

        std::cout << blocks.size() << "\n";
        std::cout << "press any key...\n";
        getc(stdin);
    }

    cout << "purged " << (total/1024/1024/1024) << " gigabytes of memory\n";
}