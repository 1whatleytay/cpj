#include <interpreter/language.h>

#include <hermes/state.h>

#include <fmt/format.h>

#include <sstream>

using namespace hermes;

namespace interpreter::language {
    bool openingBrackets(const char *text, size_t size) {
        return size >= 2 && std::memcmp(text, "{{", 2) == 0
            && !(size >= 3 && std::memcmp(text, "{{{", 3) == 0);
    }

    bool closingBrackets(const char *text, size_t size) {
        return std::isspace(*text)
            || (size >= 2 && std::memcmp(text, "}}", 2) == 0);
    }

    bool notSpace(const char *text, size_t) {
        return !std::isspace(*text);
    }

    bool always(const char *, size_t) {
        return true;
    }

    std::string replace(const std::string &templateValue,
        const std::unordered_map<std::string, std::string> &replacements) {
        State state(templateValue);

        std::stringstream stream;

        while (state.index != state.text.size()) {
            size_t size = state.until(openingBrackets);
            stream << state.pull(size);
            state.pop(size, notSpace);

            if (state.pull(2) != "{{")
                break;
            state.pop(2, notSpace);

            size_t expSize = state.until(closingBrackets);
            std::string expName = state.pull(expSize);
            state.pop(expSize, notSpace);

            if (replacements.find(expName) != replacements.end())
                stream << replacements.at(expName);

            if (state.pull(2) != "}}")
                throw std::runtime_error("Expected }} right after opening braces.");
            state.pop(2, always);
        }

        return stream.str();
    }

    std::string parse(const std::string &templateValue,
        const std::string &value, const std::string &point) {

        State templateState(templateValue);
        State documentState(value);

        std::string before = templateState.pull(templateState.until(openingBrackets));

        while (templateState.index != templateState.text.size()) {
            if (documentState.pull(before.size()) != before)
                throw std::runtime_error(
                    fmt::format("Document does not match template.\n  Expected ->\n{}\n  Got ->\n{}...",
                        before, documentState.pull(before.size())));

            documentState.pop(before.size(), always);
            templateState.pop(before.size(), notSpace);

            if (templateState.pull(2) != "{{")
                break;
            templateState.pop(2, notSpace);

            std::string expName = templateState.pull(templateState.until(closingBrackets));
            templateState.pop(expName.size(), notSpace);

            if (templateState.pull(2) != "}}")
                throw std::runtime_error("Expected }} right after opening braces.");
            templateState.pop(2, always);

            std::string after = templateState.pull(templateState.until(openingBrackets));

            bool isEndOfFile = templateState.text.size() == templateState.index;
            if (!isEndOfFile && after.empty())
                throw std::runtime_error("Two insertion points right next to each other.");

            std::string content = documentState.pull(documentState.until(
                [after](const char *text, size_t size) -> bool {
                    return size >= after.size() && std::memcmp(text, after.c_str(), after.size()) == 0;
                })
            );

            if (expName == point)
                return content;

            documentState.pop(content.size(), always);

            before = after;
        }

        throw std::runtime_error(fmt::format("Could not find point with name {} in template.", point));
    }
}
