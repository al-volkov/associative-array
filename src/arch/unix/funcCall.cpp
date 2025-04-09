#include <dlfcn.h>
#include "../../../include/WSEML.hpp"

namespace wseml {
    typedef WSEML (*func)(const WSEML&);
    WSEML callFunc(const char* dllName, const char* funcName, const WSEML& Args) {
        void* lib = dlopen(dllName, RTLD_LAZY);
        if (!lib) {
            fprintf(stderr, "dlopen error: %s\n", dlerror());
            return WSEML();
        }

        func ProcAddr = (func)dlsym(lib, funcName);
        const char* error = dlerror();
        if (error != nullptr) {
            fprintf(stderr, "dlsym error: %s\n", error);
            dlclose(lib);
            return WSEML();
        }

        WSEML res = ProcAddr(Args);
        dlclose(lib);
        return res;
    }
} // namespace wseml