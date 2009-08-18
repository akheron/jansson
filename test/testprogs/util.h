#ifndef TESTPROGS_UTIL_H
#define TESTPROGS_UTIL_H

#define fail(msg)                                                \
    do {                                                         \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1;                                                \
    } while(0)

#endif
