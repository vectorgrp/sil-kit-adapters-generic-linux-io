// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <linux/gpio.h>
#include <string>

#include "asio/posix/stream_descriptor.hpp"

namespace GpioWrapper 
{
using offset_t = uint8_t;
using Fd = asio::posix::stream_descriptor;
using Ioc = asio::io_context;

enum Edge 
{
	BothEdges = GPIOEVENT_REQUEST_BOTH_EDGES
};

enum Dir 
{
	In  = GPIOHANDLE_REQUEST_INPUT,
	Out = GPIOHANDLE_REQUEST_OUTPUT
};

class ChipInfo 
{
    friend class Chip;

public:
	constexpr auto GetName() const noexcept -> const char * { return _info.name; }
	constexpr auto GetLabel() const noexcept -> const char * { return _info.label; }
	constexpr auto GetLines() const noexcept -> offset_t { return _info.lines; }

private:
	gpiochip_info _info;
	constexpr ChipInfo(gpiochip_info info) noexcept : _info(info) {}
};

class LineInfo 
{
    friend class Chip;

public:
	constexpr auto GetOffset() const noexcept -> bool { return _info.line_offset; }
	constexpr auto GetDirection() const noexcept -> Dir { return _info.flags & GPIOLINE_FLAG_IS_OUT ? Out : In; }

private:
    gpioline_info _info;
	constexpr LineInfo(gpioline_info info) noexcept : _info(info) {}
};

class Chip 
{
	friend class LineHandle;
	friend class EventHandle;

public:
	Chip(Ioc &ioc, const std::string& devName);
    ~Chip() {
        if(IsFdOpen()) 
            Close();
     }
	ChipInfo GetInfo();
	LineInfo GetLineInfo(offset_t);

    inline void Close() { _fd.close(); }
    inline auto IsFdOpen() -> bool { return _fd.is_open(); }

private:
    Fd _fd;
};

class LineHandle 
{
public:
	LineHandle(Ioc &ioc, Chip &chip, gpiohandle_request req);
    
	template <typename Offsets = std::initializer_list<offset_t>>
	LineHandle(Ioc &ioc, Chip &chip, const Offsets &offsets, Dir dir, uint8_t value = -1
		,	const std::string& consumer = "ska-generic-linux-io")
		:	LineHandle(ioc, chip, [&]() {
			gpiohandle_request req;
			offset_t i = 0; 
            auto it = offsets.begin();
            while (i < offsets.size()) {
				req.lineoffsets[i] = *it;
				req.default_values[i] = value & 1;
				++i; ++it; 
			}
			req.flags = dir;
			std::memcpy(req.consumer_label, consumer.data(), consumer.length());
			req.consumer_label[consumer.length()] = '\0';
			req.lines = offsets.size();
			req.fd = -1;
			return req;
		}()) {}

	auto GetValue() -> uint64_t;
	void SetValue(uint64_t values);

    inline void Close() { _fd.close(); }
    inline auto IsFdOpen() -> bool { return _fd.is_open(); }

private:
    Fd _fd;
    offset_t _numLines;
};

class EventHandle 
{
public:
	EventHandle(Ioc &ioc, Chip &chip, offset_t offset, Edge events = BothEdges,
		const std::string& consumer = "ska-generic-linux-io");

	inline void Cancel() { _fd.cancel(); }
    inline void Close() { _fd.close(); }
    inline auto IsFdOpen() -> bool { return _fd.is_open(); }
    inline auto GetFd() noexcept -> Fd* { return &_fd; }

private:
    Fd _fd;
};

} // namespace GpioWrapper