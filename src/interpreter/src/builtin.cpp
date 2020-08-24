#include <interpreter/builtin.h>

#include <interpreter/language.h>
#include <interpreter/interpreter.h>

#include <fmt/printf.h>

#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

#define ARG_COUNT(count) \
    if (arguments.size() != count) \
        throw std::runtime_error(fmt::format("Expected {} argument to {}, got {}", \
            count, __FUNCTION__, arguments.size()))

std::string format(double value) {
    if (static_cast<double>(static_cast<int>(value)) == value)
        return std::to_string(static_cast<int>(value));

    return std::to_string(value);
}

namespace interpreter::builtin {
    std::string readTemplate(const Arguments &arguments) {
        ARG_COUNT(1);

        std::string templateName = arguments[0];

        std::string project = getProject();
        if (project.empty())
            throw std::runtime_error("Could not find project directory for some reason...");

        fs::path templatePath = fs::path(project) / templateName;
        if (!fs::exists(templatePath))
            throw std::runtime_error(fmt::format("Could not find template at {}", templatePath));

        std::ifstream stream(templatePath);
        std::stringstream buffer;
        buffer << stream.rdbuf();

        return buffer.str();
    }

    std::string read(const Arguments &arguments) {
        ARG_COUNT(1);

        std::string path = arguments[0];

        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();

        return buffer.str();
    }

    std::string write(const Arguments &arguments) {
        ARG_COUNT(2);

        std::string data = arguments[0];
        std::string path = arguments[1];

        std::ofstream stream(path, std::ios::trunc);
        stream << data;

        return "";
    }

    std::string parse(const Arguments &arguments) {
        ARG_COUNT(3);

        std::string fileValue = arguments[0];
        std::string templateValue = arguments[1];
        std::string point = arguments[2];

        std::string value = language::parse(templateValue, fileValue, point);
        return value;
    }

    std::string directory(const Arguments &arguments) {
        ARG_COUNT(1);

        std::string path = arguments[0];

        fs::create_directories(path);

        return "";
    }

    std::string print(const Arguments &arguments) {
        ARG_COUNT(1);

        std::string text = arguments[0];

        fmt::print("{}\n", text);

        return "";
    }

    std::string substr(const Arguments &arguments) {
        ARG_COUNT(3);

        std::string text = arguments[0];
        size_t start = std::stoull(arguments[1]);
        size_t count = std::stoull(arguments[2]);

        return text.substr(start, count);
    }

    std::string size(const Arguments &arguments) {
        ARG_COUNT(1);

        std::string text = arguments[0];

        return std::to_string(text.size());
    }

    std::string uppercase(const Arguments &arguments) {
        ARG_COUNT(1);

        std::string text = arguments[0];
        std::transform(text.begin(), text.end(), text.begin(), [](char x) { return std::toupper(x); });

        return text;
    }

    std::string lowercase(const Arguments &arguments) {
        ARG_COUNT(1);

        std::string text = arguments[0];
        std::transform(text.begin(), text.end(), text.begin(), [](char x) { return std::tolower(x); });

        return text;
    }

    std::string expNot(const Arguments &arguments) {
        ARG_COUNT(1);

        std::string text = arguments[0];

        return (text == "true") ? "false" : "true";
    }

    std::string expEquals(const Arguments &arguments) {
        ARG_COUNT(2);

        std::string first = arguments[0];
        std::string second = arguments[1];

        return (first == second) ? "true" : "false";
    }

    std::string expNotEquals(const Arguments &arguments) {
        ARG_COUNT(2);

        std::string first = arguments[0];
        std::string second = arguments[1];

        return (first == second) ? "false" : "true";
    }

    std::string expAdd(const Arguments &arguments) {
        ARG_COUNT(2);

        double first = std::stod(arguments[0]);
        double second = std::stod(arguments[1]);

        return format(first + second);
    }

    std::string expSub(const Arguments &arguments) {
        ARG_COUNT(2);

        double first = std::stod(arguments[0]);
        double second = std::stod(arguments[1]);

        return format(first - second);
    }

    std::string expMul(const Arguments &arguments) {
        ARG_COUNT(2);

        double first = std::stod(arguments[0]);
        double second = std::stod(arguments[1]);

        return format(first * second);
    }

    std::string expDiv(const Arguments &arguments) {
        ARG_COUNT(2);

        double first = std::stod(arguments[0]);
        double second = std::stod(arguments[1]);

        return format(first / second);
    }

    std::string expGreater(const Arguments &arguments) {
        ARG_COUNT(2);

        double first = std::stod(arguments[0]);
        double second = std::stod(arguments[1]);

        return (first > second) ? "true" : "false";
    }

    std::string expLesser(const Arguments &arguments) {
        ARG_COUNT(2);

        double first = std::stod(arguments[0]);
        double second = std::stod(arguments[1]);

        return (first < second) ? "true" : "false";
    }

    std::string expGreaterEquals(const Arguments &arguments) {
        ARG_COUNT(2);

        double first = std::stod(arguments[0]);
        double second = std::stod(arguments[1]);

        return (first >= second) ? "true" : "false";
    }

    std::string expLesserEquals(const Arguments &arguments) {
        ARG_COUNT(2);

        double first = std::stod(arguments[0]);
        double second = std::stod(arguments[1]);

        return (first <= second) ? "true" : "false";
    }

    std::string expOr(const Arguments &arguments) {
        ARG_COUNT(2);

        bool first = arguments[0] == "true";
        bool second = arguments[1] == "true";

        return (first || second) ? "true" : "false";
    }

    std::string expAnd(const Arguments &arguments) {
        ARG_COUNT(2);

        bool first = arguments[0] == "true";
        bool second = arguments[1] == "true";

        return (first && second) ? "true" : "false";
    }

    std::string expTurnary(const Arguments &arguments) {
        ARG_COUNT(3);

        bool state = arguments[0] == "true";
        std::string first = arguments[1];
        std::string second = arguments[2];

        return state ? first : second;
    }

    std::unordered_map<std::string, Function> functions = {
        { "template", readTemplate },
        { "read", read },
        { "write", write },
        { "parse", parse },
        { "directory", directory },
        { "print", print },
        { "substr", substr },
        { "size", size },
        { "uppercase", uppercase },
        { "lowercase", lowercase },
        { "n", expNot },
        { "e", expEquals },
        { "ne", expNotEquals },
        { "a", expAdd },
        { "s", expSub },
        { "m", expMul },
        { "d", expDiv },
        { "g", expGreater },
        { "l", expLesser },
        { "ge", expGreaterEquals },
        { "le", expLesserEquals },
        { "or", expOr },
        { "and", expAnd },
        { "t", expTurnary }
    };
}
