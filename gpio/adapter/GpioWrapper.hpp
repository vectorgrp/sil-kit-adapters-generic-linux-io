// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <linux/gpio.h>
#include <string>
#include <string_view>

#include <asio/posix/stream_descriptor.hpp>
#include <asio/read.hpp>

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
	constexpr const char *GetName() const noexcept { return _info.name; }
	constexpr const char *GetLabel() const noexcept { return _info.label; }
	constexpr offset_t GetLines() const noexcept { return _info.lines; }

private:
	gpiochip_info _info;
	constexpr ChipInfo(gpiochip_info info) noexcept : _info(info) {}
};

class LineInfo 
{
    friend class Chip;

public:
	constexpr bool GetOffset() const noexcept { return _info.line_offset; }
	constexpr Dir GetDirection() const noexcept { return _info.flags & GPIOLINE_FLAG_IS_OUT ? Out : In; }

private:
    gpioline_info _info;
	constexpr LineInfo(gpioline_info info) noexcept : _info(info) {}
};

class Chip 
{
	friend class LineHandle;
	friend class EventHandle;

public:
	Chip(Ioc &ioc, std::string_view devName);
	ChipInfo GetInfo();
	LineInfo GetLineInfo(offset_t);

    inline void Close() { if (fd.is_open()) fd.close(); }

private:
    Fd fd;
};

class LineHandle 
{
public:
	static constexpr offset_t MAX = GPIOHANDLES_MAX;

	LineHandle(Ioc &ioc, Chip &chip, gpiohandle_request req);
    
	template <typename Offsets = std::initializer_list<offset_t>>
	LineHandle(Ioc &ioc, Chip &chip, const Offsets &offsets, Dir dir, uint8_t value = -1
		,	std::string_view consumer = "ska-generic-linux-io")
		:	LineHandle(ioc, chip, [&]() constexpr {
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

	uint64_t GetValue();
	void SetValue(uint64_t values);

    inline void Close() { fd.close(); }
    inline bool IsFdOpen() { return fd.is_open(); }

private:
    Fd fd;
    offset_t numLines;
};

class EventHandle 
{
public:
	EventHandle(Ioc &ioc, Chip &chip, offset_t offset, Edge events = BothEdges,
		std::string_view consumer = "ska-generic-linux-io");

	inline void Cancel() { fd.cancel(); }
    inline void Close() { fd.close(); }
    inline bool IsFdOpen() { return fd.is_open(); }

    inline Fd* GetFd() noexcept { return &fd; }

private:
    Fd fd;
};

} // namespace GpioWrapper