#pragma once

#include <string>
#include <unordered_map>

namespace interpreter::language {
    std::string replace(const std::string &templateValue,
        const std::unordered_map<std::string, std::string> &replacements);
    std::unordered_map<std::string, std::string> parse(const std::string &templateValue, const std::string &value);
}
