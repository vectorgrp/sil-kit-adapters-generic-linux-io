// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


class GpioChip {
public: 
	GpioChip() = delete;
	GpioChip(const std::string& fd);

	SetGpioValues(::gpiod::line_request& line_req, GpioChip newValues)

private:
	std::string _fd;
};