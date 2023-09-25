// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <filesystem>

#include "GpioChip.hpp"
#include "ChipDatas.hpp"

GpioChip::GpioChip(const ::std::filesystem::path& fd)
{
    _chip = std::make_shared<::gpiod::chip>(::gpiod::chip(fd));
    _numLines = _chip->get_info().num_lines();

    // Initialize _lineReq without modifying gpio chip lines
    // Returned values are not useful
    static_cast<void>(GetGpioOffsetsValues(true));
}

GpioChip::~GpioChip() 
{
    _lineReq->release();
    _chip->close();
}

void GpioChip::SetGpioValues(const ChipDatas& chipDatas) 
{
    const auto offsetsOUT = chipDatas.GetLinesDirection().second;
    const auto valuesOUT = chipDatas.GetOutputLinesValues();

    auto line_conf = ::gpiod::line_config();

    for (std::size_t i = 0; i < chipDatas.GetDatasSize(); ++i) 
    {
        if (chipDatas.GetPinValue(i) == 0) 
        {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                .set_direction(::gpiod::line::direction::INPUT)
            );
        }
        else 
        {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                .set_direction(::gpiod::line::direction::OUTPUT)
            );
        }
    }

    // Setting ouput values if any
    if (offsetsOUT.size() > 0)
    {
        _lineReq->reconfigure_lines(line_conf)
            .set_values(ConvertLinesToOffsets(offsetsOUT), ConvertBitsToGpiodValues(valuesOUT));
    }
    else 
    {
        _lineReq->reconfigure_lines(line_conf);
    }
}

auto GpioChip::GetGpioOffsetsDirection() const -> std::vector<::gpiod::line::direction>
{
    std::vector<::gpiod::line::direction> directions;

    for (std::size_t i = 0; i < _numLines; ++i) 
    {
        const auto line_info = _chip->get_line_info(::gpiod::line::offset(i));
        directions.push_back(line_info.direction());
    }

    return directions;
}

auto GpioChip::GetGpioOffsetsValues(bool initialization) -> ::gpiod::line::values
{
    auto line_conf = ::gpiod::line_config();
    auto directions = GetGpioOffsetsDirection();

    for (std::size_t i = 0; i < _numLines; ++i) 
    {
        if (directions[i] == ::gpiod::line::direction::INPUT) 
        {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                    .set_direction(::gpiod::line::direction::INPUT)
            );
        } 
        else 
        {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                    .set_direction(::gpiod::line::direction::OUTPUT)
                    // This is a default value, should set the real value
                    .set_output_value(::gpiod::line::value::ACTIVE)
            );
        }
    }

    if (initialization)
    {
        // Initialize _lineReq with gpio state if it is the first attempt to GetGpioOffsetsValues
        _lineReq = std::make_shared<::gpiod::line_request>(_chip->prepare_request().set_line_config(line_conf).do_request());
        return  _lineReq->get_values();
    }
    else 
        return _lineReq->reconfigure_lines(line_conf).get_values();
}

auto GpioChip::ReadGpioEvents(ChipDatas& chipDatas, ::gpiod::edge_event_buffer& buffer) -> bool
{
    auto line_conf = ::gpiod::line_config();

    for (std::size_t i = 0; i < chipDatas.GetDatasSize(); ++i) 
    {
        if (chipDatas.GetPinDirection(i) == 0)
        {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                .set_direction(::gpiod::line::direction::INPUT)
                .set_edge_detection(::gpiod::line::edge::BOTH)
            );
        }
        else 
        {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                .set_direction(::gpiod::line::direction::OUTPUT)
                .set_output_value(ConvertBitToGpioValue(chipDatas.GetPinValue(i)))
            );
        }
    }

    // Refreshing every 100000000 nanoseconds to check if a newMsg is received
    // Can be modified by adding a new pipe to wait_edge_event
    const bool newEvent = _lineReq->reconfigure_lines(line_conf)
        .wait_edge_events(::std::chrono::nanoseconds(1000000000));

    // If new event, fill the buffer
    if (newEvent) _lineReq->read_edge_events(buffer);

    return newEvent;
}

auto GpioChip::ConvertBitToGpioValue(const std::uint8_t bit) const -> ::gpiod::line::value
{
    return (bit == 1 ? gpiod::line::value::ACTIVE : gpiod::line::value::INACTIVE);
};

auto GpioChip::ConvertBitsToGpiodValues(const std::vector<std::uint8_t>& bits) const -> ::gpiod::line::values
{
    ::gpiod::line::values values;
    for (const auto bit : bits)
        values.push_back(ConvertBitToGpioValue(bit));

    return values;
};

auto GpioChip::ConvertLinesToOffsets(const std::vector<std::size_t>& lines) const -> ::gpiod::line::offsets
{
    ::gpiod::line::offsets offsets;
    for (const auto line : lines)
        offsets.push_back(::gpiod::line::offset(line));
    
    return offsets;
}