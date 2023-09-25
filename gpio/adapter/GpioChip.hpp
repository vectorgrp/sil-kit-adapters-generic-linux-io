// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <filesystem>

#include "ChipDatas.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "gpiod.hpp"

// Used to communicate with the gpio chip through gpiod library
class GpioChip
{
public: 
	GpioChip() = delete;
	GpioChip(const ::std::filesystem::path& fd);
	~GpioChip();

	// Set input direction or output with associated values
	void SetGpioValues(const ChipDatas& chipDatas);
	// Get all lines direction 
	auto GetGpioOffsetsDirection() const -> std::vector<::gpiod::line::direction>;
	// Get the value of each line without modifying their direction
	auto GetGpioOffsetsValues(bool initialization = false) -> ::gpiod::line::values;
	// Wait an event from chip's lines and write it in a buffer
	auto ReadGpioEvents(ChipDatas& chipDatas, ::gpiod::edge_event_buffer& buffer) -> bool;

private:
	// Convert a bit to an ACTIVE or INACTIVE value
	auto ConvertBitToGpioValue(const std::uint8_t bit) const -> ::gpiod::line::value;
	// Convert a vector of bits to a vector of values
	auto ConvertBitsToGpiodValues(const std::vector<std::uint8_t>& bits) const -> ::gpiod::line::values;

	// Convert lines (vector of std::size_t) to offsets
	auto ConvertLinesToOffsets(const std::vector<std::size_t>& lines) const -> ::gpiod::line::offsets;

	std::shared_ptr<::gpiod::chip> _chip;
	std::shared_ptr<::gpiod::line_request> _lineReq;
	std::size_t _numLines;
};