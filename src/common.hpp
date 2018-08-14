#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include <iterator>
#include <numeric>
#include <cstring>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <mutex>


#if defined(unix) || defined(__unix__) || defined(__unix)
#ifndef __unix__
#define __unix__
#endif

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif // unix

#if defined(_WIN64) || defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#ifndef WIN32
#define WIN32
#endif

#include <stdio.h>  
#include <fcntl.h>  
#include <io.h>
#include <windows.h>
#endif // windows