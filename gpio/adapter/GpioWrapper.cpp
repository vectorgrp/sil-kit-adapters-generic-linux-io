// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "GpioWrapper.hpp"

namespace GpioWrapper 
{

template <uint Name, typename T>
struct IoControlCommand 
{
	T data_;
	constexpr auto name() const noexcept -> int { return Name; }
	constexpr void *data() noexcept { return &data_; }
};

Chip::Chip(Ioc &ioc, const std::string& devName)
	: _fd(ioc, open(devName.c_str(), 0)) {}

ChipInfo Chip::GetInfo() 
{
	IoControlCommand<GPIO_GET_CHIPINFO_IOCTL, gpiochip_info> cmd;
	_fd.io_control(cmd);
	return ChipInfo(std::move(cmd.data_));
}

LineInfo Chip::GetLineInfo(offset_t offset) 
{
	IoControlCommand<GPIO_GET_LINEINFO_IOCTL, gpioline_info> cmd;
	cmd.data_.line_offset = offset;
	_fd.io_control(cmd);
	return LineInfo(std::move(cmd.data_));
}

LineHandle::LineHandle(Ioc &ioc, Chip &chip, gpiohandle_request req)
	: _fd(ioc, [&]() {
		IoControlCommand<GPIO_GET_LINEHANDLE_IOCTL, gpiohandle_request &> cmd({req});
		chip._fd.io_control(cmd);
		return cmd.data_.fd;
	}()), _numLines(req.lines) {}

auto LineHandle::GetValue() -> uint64_t
{
	IoControlCommand<GPIOHANDLE_GET_LINE_VALUES_IOCTL, gpiohandle_data> cmd;
	_fd.io_control(cmd);
	uint64_t res = 0;
	for (offset_t i = 0; i < _numLines; ++i) {
		res |= cmd.data_.values[i] << i;
	}
	return res;
}

void LineHandle::SetValue(uint64_t values) 
{
	IoControlCommand<GPIOHANDLE_SET_LINE_VALUES_IOCTL, gpiohandle_data> cmd;
	for (offset_t i = 0; i < _numLines; ++i, values >>= 1) {
		cmd.data_.values[i] = values & 1;
	}
	_fd.io_control(cmd);
}

EventHandle::EventHandle(Ioc &ioc, Chip &chip, offset_t offset
	, Edge events, const std::string& consumer)
  : _fd(ioc, [&] {
		assert(consumer.length() < 32);
		IoControlCommand<GPIO_GET_LINEEVENT_IOCTL, gpioevent_request> cmd;
		cmd.data_.lineoffset = offset;
		cmd.data_.handleflags = In;
		cmd.data_.eventflags = events;
		std::memcpy(cmd.data_.consumer_label, consumer.data(), consumer.length());
		cmd.data_.consumer_label[consumer.length()] = '\0';
		chip._fd.io_control(cmd);
		return cmd.data_.fd;
	}()) {}

} // namespace GpioWrapper
