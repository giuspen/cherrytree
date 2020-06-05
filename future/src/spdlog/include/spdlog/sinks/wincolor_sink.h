// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <spdlog/details/console_globals.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/sink.h>

#include <memory>
#include <mutex>
#include <string>
#include <array>

#include <spdlog/details/windows_include.h>
#include <wincon.h>

namespace spdlog {
namespace sinks {
/*
 * Windows color console sink. Uses WriteConsoleA to write to the console with
 * colors
 */
template<typename ConsoleMutex>
class wincolor_sink : public sink
{
public:
    const WORD BOLD = FOREGROUND_INTENSITY;
    const WORD RED = FOREGROUND_RED;
    const WORD GREEN = FOREGROUND_GREEN;
    const WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;

    wincolor_sink(HANDLE out_handle, color_mode mode);
    ~wincolor_sink() override;

    wincolor_sink(const wincolor_sink &other) = delete;
    wincolor_sink &operator=(const wincolor_sink &other) = delete;

    // change the color for the given level
    void set_color(level::level_enum level, WORD color);
    void log(const details::log_msg &msg) final override;
    void flush() final override;
    void set_pattern(const std::string &pattern) override final;
    void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override final;
    void set_color_mode(color_mode mode);

protected:
    using mutex_t = typename ConsoleMutex::mutex_t;
    HANDLE out_handle_;
    mutex_t &mutex_;
    bool in_console_;
    bool should_do_colors_;
    std::unique_ptr<spdlog::formatter> formatter_;
    std::array<WORD, level::n_levels> colors_;

    // set foreground color and return the orig console attributes (for resetting later)
    WORD set_foreground_color_(WORD attribs);

    // print a range of formatted message to console
    void print_range_(const memory_buf_t &formatted, size_t start, size_t end);

    // in case we are redirected to file (not in console mode)
    void write_to_file_(const memory_buf_t &formatted);
};

template<typename ConsoleMutex>
class wincolor_stdout_sink : public wincolor_sink<ConsoleMutex>
{
public:
    explicit wincolor_stdout_sink(color_mode mode = color_mode::automatic);
};

template<typename ConsoleMutex>
class wincolor_stderr_sink : public wincolor_sink<ConsoleMutex>
{
public:
    explicit wincolor_stderr_sink(color_mode mode = color_mode::automatic);
};

using wincolor_stdout_sink_mt = wincolor_stdout_sink<details::console_mutex>;
using wincolor_stdout_sink_st = wincolor_stdout_sink<details::console_nullmutex>;

using wincolor_stderr_sink_mt = wincolor_stderr_sink<details::console_mutex>;
using wincolor_stderr_sink_st = wincolor_stderr_sink<details::console_nullmutex>;

} // namespace sinks
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "wincolor_sink-inl.h"
#endif
