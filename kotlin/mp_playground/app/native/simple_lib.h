#ifndef SIMPLE_LIB_H
#define SIMPLE_LIB_H

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT int add(int a, int b);
EXPORT const char* get_greeting(void);

#ifdef __cplusplus
}
#endif

#endif // SIMPLE_LIB_H