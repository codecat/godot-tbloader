#ifndef LIBMAP_PLATFORM_H
#define LIBMAP_PLATFORM_H

#if defined(_MSC_VER)
#define STRDUP(s) _strdup(s)
#else
#define STRDUP(s) strdup(s)
#endif

#endif