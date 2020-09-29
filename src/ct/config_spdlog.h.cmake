#cmakedefine SPDLOG_FMT_EXTERNAL
#ifdef SPDLOG_FMT_EXTERNAL
#include <fmt/printf.h>
#else
#include <spdlog/fmt/bundled/printf.h>
#endif
