#pragma once

#include <parser/root.h>
#include <parser/action.h>
#include <parser/string.h>
#include <parser/expression.h>

#include <string>
#include <optional>

namespace interpreter {
    using Locals = std::unordered_map<std::string, std::string>;
    using Arguments = std::vector<std::string>;

    void init();

    std::string getProject();

    std::string getTemplate(const std::string &file);
    std::string getScript();

    std::unique_ptr<RootContext> parseScript(const std::string &path);

    std::optional<std::string> execute(RootContext *root, Context *context, Locals &locals);
    std::string execute(RootContext *root, const std::string &name, const Arguments &arguments);
    std::string evaluate(RootContext *root, StringContext *string, const Locals &locals);
    std::string evaluate(RootContext *root, ExpressionContext *context, const Locals &locals);
    std::string evaluate(RootContext *root, LiteralExpressionContext *context, const Locals &locals);
    std::string evaluate(RootContext *root, TemplateExpressionContext *context, const Locals &locals);

    void run(int count, const char **args);
}
