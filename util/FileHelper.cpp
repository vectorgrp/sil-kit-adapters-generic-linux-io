// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FileHelper.hpp"

#include <fstream>
#include <sstream>
#include <fcntl.h>

#include "Exceptions.hpp"

namespace adapters {
namespace Util {

auto ReadFileStr(const std::string& path, SilKit::Services::Logging::ILogger* logger) -> std::string
{
    std::ifstream fs(path);

    if (!fs.is_open())
    {
        logger->Error("File " + path + " cannot be opened: " + GetErrno());
        return "";
    }

    logger->Debug("Reading file " + path);

    std::stringstream buffer;
    buffer << fs.rdbuf();

    fs.close();

    return buffer.str();
}

auto ReadFile(const std::string& path, SilKit::Services::Logging::ILogger* logger, std::array<uint8_t, BUF_LEN>& buffer) -> std::size_t
{
    auto fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        logger->Error("File " + path + " cannot be opened (O_RDONLY mode): " + GetErrno());
        return {};
    }

    const auto n = read(fd, &buffer[0], buffer.size());

    if (n == -1)
    {
        if (errno == EAGAIN)
        {
            // File is empty or resource is temporarily unavailable
            return {};
        }
        else
        {
            logger->Error("Reading " + path + " ended with error: " + GetErrno());
        }
    }

    close(fd);
    
    return n;
}

void WriteFile(const std::string& path, const std::vector<uint8_t>& dataToWrite, SilKit::Services::Logging::ILogger* logger)
{
    auto fd = open(path.c_str(), O_WRONLY | O_TRUNC);
    if (fd < 0)
    {
        logger->Error("File " + path + " cannot be opened (WRONLY mode): " + GetErrno());
        close(fd);
        return;
    }
    const int res = write(fd, reinterpret_cast<const char*>(dataToWrite.data()), dataToWrite.size());

    if (res == -1)
    {
        logger->Error("Writing into " + path + " ended with error: " + GetErrno());
    }

    close(fd);
}

} // namespace Util
} // namespace adapters