// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <filesystem>

#include "ChipDatas.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "gpiod.hpp"

class GpioChip
{
public: 
	GpioChip() = delete;
	GpioChip(const ::std::filesystem::path& fd);
	~GpioChip();

	void SetGpioValues(const ChipDatas& chipDatas);
	auto GetGpioOffsetsDirection() const -> std::vector<::gpiod::line::direction>;
	auto GetGpioOffsetsValues(bool initialization = false) -> ::gpiod::line::values;
	auto ReadGpioEvents(ChipDatas& chipDatas, ::gpiod::edge_event_buffer& buffer) -> bool;

private:
	auto ConvertBitToGpioValue(const std::uint8_t bit) const -> ::gpiod::line::value;
	auto ConvertBitsToGpiodValues(const std::vector<std::uint8_t>& bits) const -> ::gpiod::line::values;

	auto ConversBitsToOffsets(const std::vector<std::uint8_t>& bits) const -> ::gpiod::line::offsets;

	std::shared_ptr<::gpiod::chip> _chip;
	std::shared_ptr<::gpiod::line_request> _lineReq;
	std::size_t _numLines;
};