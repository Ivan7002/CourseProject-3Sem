#include <iostream>
#include <time.h>
#include <cstring>
#include <cstdio>

extern "C" {
    int __stdcall str_len(char* buffer, char* str) 
    {
        if (str == nullptr)
            return 0;
        int len = 0;
        while(str[len] != '\0') len++;
        return len;
    }

    int __stdcall tostring(char* buffer, int value) 
    {
        sprintf_s(buffer, 255, "%d", value);
        return (int)buffer;
    }
    
    int __stdcall atoii(char* buffer, char* ptr) 
    {
        if (!ptr) return 0;
        int n = 0;
        bool neg = false;
        if (*ptr == '-') { neg=true; ptr++; }
        while (*ptr >= '0' && *ptr <= '9') {
            n *= 10;
            n += *ptr++;
            n -= '0';
        }
        return neg ? -n : n;
    }

    int __stdcall outrad(char* ptr) 
    {
        if (ptr == nullptr)
        {
            std::cout << std::endl;
            return 0;
        }
        std::cout << ptr;
        return 0;
    }

    int __stdcall outlich(int value) 
    {
        std::cout << value;
        return 0;
    }
}
