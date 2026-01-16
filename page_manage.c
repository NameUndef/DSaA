#include "page_manage.h"

#if defined(_WIN32) || defined(_WIN64)

    #include <windows.h>

    size_t get_page_size()
    {
        static size_t page_size = 0;
        if (page_size) {
            return page_size;
        }

        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        page_size = (size_t) sysinfo.dwPageSize;
        return page_size;
    }

    void* allocate_page()
    {
        return (void*) VirtualAlloc(NULL, get_page_size(), MEM_COMMIT, PAGE_READWRITE);
    }

    void* allocate_page_sz(size_t page_size)
    {
        return (void*) VirtualAlloc(NULL, page_size, MEM_COMMIT, PAGE_READWRITE);
    }

    void free_page(void* page)
    {
        VirtualFree(page, 0, MEM_RELEASE);
    }

    void free_page_sz(void* page, size_t)
    {
        VirtualFree(page, 0, MEM_RELEASE);
    }

#elif defined(__linux__)

    #include <unistd.h>
    #include <sys/mman.h>

    size_t get_page_size()
    {
        static size_t page_size = 0;
        if (page_size) {
            return page_size;
        }


        page_size = (size_t) sysconf(_SC_PAGESIZE);
        return page_size;
    }

    void* allocate_page()
    {
        return (void*) mmap(NULL, get_page_size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }

    void* allocate_page_sz(size_t page_size)
    {
        return (void*) mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }

    void free_page(void* page)
    {
        munmap(page, get_page_size());
    }

    void free_page_sz(void* page, size_t page_size)
    {
        munmap(page, page_size);
    }

#endif