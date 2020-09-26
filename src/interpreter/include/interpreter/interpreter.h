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
    std::string getConfig();
    std::string getScript();

    std::unique_ptr<RootNode> parseScript(const std::string &path);

    std::optional<std::string> execute(RootNode *root, Node *context, Locals &locals);
    std::string execute(RootNode *root, const std::string &name, const Arguments &arguments);
    std::string evaluate(RootNode *root, StringNode *string, const Locals &locals);
    std::string evaluate(RootNode *root, ExpressionNode *context, const Locals &locals);
    std::string evaluate(RootNode *root, LiteralExpressionNode *context, const Locals &locals);
    void evaluate(RootNode *root, ParseExpressionNode *context, Locals &locals);
    std::string evaluate(RootNode *root, TemplateExpressionNode *context, const Locals &locals);
    std::string evaluate(RootNode *root, ReplaceExpressionNode *context, const Locals &locals);

    void run(int count, const char **args);
}
