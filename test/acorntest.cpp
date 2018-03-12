#include <spdlog/spdlog.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

static auto logger = spdlog::stdout_color_mt("acorn");
