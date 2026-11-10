// libnx's newlib has no sysconf(); PCSX ReARMed asks it for the page size (dynarec) and
// the core count (its CD, GPU and SPU threads).
#ifdef __SWITCH__
#include <cerrno>
#include <unistd.h>

extern "C" long sysconf(int name) {
    switch (name) {
        case _SC_PAGESIZE: return 0x1000;
        // An application gets cores 0 to 2; core 3 belongs to the system.
        case _SC_NPROCESSORS_ONLN:
        case _SC_NPROCESSORS_CONF: return 3;
        default: errno = EINVAL; return -1;
    }
}
#endif
