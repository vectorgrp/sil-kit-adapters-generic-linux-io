// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <stdexcept>
#include <errno.h>
#include <string>
#include <string.h>

namespace adapters {

enum ExitCode
{
    NO_ERROR = 0,
    INIT_ERROR = -1,
    EVENT_IGNORED = -2,
    READ_ERROR = -3,
    CLI_ERROR = -4,
    CONFIGURATION_ERROR = -5,
    SILKIT_ERROR = -6,
    YAML_ERROR = -7,
    INOTIFY_ERROR = -8,
    OTHER_ERROR = -9
};

// Errno handler
inline auto GetErrno() -> std::string
{
    char buffer[256];
#ifdef QNX_BUILD
    int errorMsg = strerror_r(errno, buffer, 256);
    return std::string(buffer);
#else // Other Unix
    char * errorMsg = strerror_r(errno, buffer, 256);
    return std::string(errorMsg);
#endif
}

// Throw handlers

// Base class of all adapter exceptions
class AdapterError: public std::exception
{
protected:
    std::string _what;
public:

    AdapterError(std::string message)
        :_what{std::move(message)}
    {
    }

    AdapterError(const char* message)
        :_what{message}
    {
    }

    const char* what() const noexcept override
    {
        return _what.c_str();
    }
};

class YamlError : public AdapterError
{
public:
    using AdapterError::AdapterError;

    YamlError() : YamlError("Adapter: YAML configuration has syntactical error or missing mandatory field.") { }
};

class InotifyError : public AdapterError
{
public:
    using AdapterError::AdapterError;

    InotifyError() : InotifyError("Adapter: Inotify has error.") { }
};

struct IncompleteReadError : std::runtime_error
{
    IncompleteReadError()
        : std::runtime_error("incomplete read")
    {
    }
};

class InvalidCli : public std::exception
{
};

template<class InvalidCli>
void throwIf(bool b)
{
    if (b)
        throw std::exception();
}

inline void throwInvalidCliIf(bool b)
{
    return throwIf<InvalidCli>(b);
}

} // namespace adapters
