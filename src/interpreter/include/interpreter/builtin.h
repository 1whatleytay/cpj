#pragma once

#include <string>
#include <functional>
#include <unordered_map>

namespace interpreter {
    using Arguments = std::vector<std::string>;

    namespace builtin {
        using Function = std::function<std::string(const Arguments &arguments)>;

        std::string readTemplate(const Arguments &arguments);
        std::string read(const Arguments &arguments);
        std::string write(const Arguments &arguments);
        std::string parse(const Arguments &arguments);
        std::string directory(const Arguments &arguments);

        extern std::unordered_map<std::string, Function> functions;
    }
}
