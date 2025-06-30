#pragma once

#define CSL_PRAGMA_STRINGIFY_HELPER(x) #x
#define CSL_PRAGMA_STRINGIFY(x) CSL_PRAGMA_STRINGIFY_HELPER(x)

#if defined(__clang__)
    #define CSL_DIAG_PUSH _Pragma("clang diagnostic push")
    #define CSL_DIAG_POP  _Pragma("clang diagnostic pop")
    #define CSL_DIAG_DISABLE(warningName) \
        _Pragma(CSL_PRAGMA_STRINGIFY(clang diagnostic ignored warningName))
#elif defined(__GNUC__) || defined(__GNUG__)
    #define CSL_DIAG_PUSH _Pragma("GCC diagnostic push")
    #define CSL_DIAG_POP  _Pragma("GCC diagnostic pop")
    #define CSL_DIAG_DISABLE(warningName) \
        _Pragma(CSL_PRAGMA_STRINGIFY(GCC diagnostic ignored warningName))
#else
    #warning csl disable warning: compiler not supported
    #define CSL_DIAG_PUSH
    #define CSL_DIAG_POP
    #define CSL_DIAG_DISABLE(warningName)
#endif
