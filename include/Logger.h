#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>

class Logger {
public:
    template<typename... Args>
    static void logEvent(const char* eventKind, const char* format, Args... args) {
        char buffer[2048];
        snprintf(buffer, sizeof(buffer), format, args...);
        printf("{\"eventKind\":\"%s\", \"data\":%s}\n", eventKind, buffer);
        fflush(stdout);
    }

    static void logEvent(const char* eventKind, const char* jsonData) {
        printf("{\"eventKind\":\"%s\", \"data\":%s}\n", eventKind, jsonData);
        fflush(stdout);
    }
};

#endif
