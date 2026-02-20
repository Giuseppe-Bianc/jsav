#pragma once
// NOLINTBEGIN(*-macro-usage)

// Internal helper: stringifies a pragma for _Pragma()
#define JSAV_DO_PRAGMA(x) _Pragma(#x)

#ifdef _MSC_VER
    #define DISABLE_WARNINGS_PUSH(...) __pragma(warning(push)) __pragma(warning(disable : __VA_ARGS__))
    #define DISABLE_WARNINGS_POP()     __pragma(warning(pop))
    #define DISABLE_CLANG_WARNINGS_PUSH(warning)
    #define DISABLE_CLANG_WARNINGS_POP()
    #define DISABLE_GCC_WARNINGS_PUSH(warning)
    #define DISABLE_GCC_WARNINGS_POP()
#elif defined(__clang__)
    #define DISABLE_WARNINGS_PUSH(...)
    #define DISABLE_WARNINGS_POP()
    #define DISABLE_CLANG_WARNINGS_PUSH(warning) \
        _Pragma("clang diagnostic push")         \
        JSAV_DO_PRAGMA(clang diagnostic ignored warning)
    #define DISABLE_CLANG_WARNINGS_POP() _Pragma("clang diagnostic pop")
    #define DISABLE_GCC_WARNINGS_PUSH(warning)
    #define DISABLE_GCC_WARNINGS_POP()
#elif defined(__GNUC__)
    #define DISABLE_WARNINGS_PUSH(...)
    #define DISABLE_WARNINGS_POP()
    #define DISABLE_CLANG_WARNINGS_PUSH(warning)
    #define DISABLE_CLANG_WARNINGS_POP()
    #define DISABLE_GCC_WARNINGS_PUSH(warning) \
        _Pragma("GCC diagnostic push")         \
        JSAV_DO_PRAGMA(GCC diagnostic ignored warning)
    #define DISABLE_GCC_WARNINGS_POP() _Pragma("GCC diagnostic pop")
#else
    #define DISABLE_WARNINGS_PUSH(...)
    #define DISABLE_WARNINGS_POP()
    #define DISABLE_CLANG_WARNINGS_PUSH(warning)
    #define DISABLE_CLANG_WARNINGS_POP()
    #define DISABLE_GCC_WARNINGS_PUSH(warning)
    #define DISABLE_GCC_WARNINGS_POP()
#endif

// NOLINTEND(*-macro-usage)