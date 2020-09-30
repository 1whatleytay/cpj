#include <interpreter/interpreter.h>

#include <interpreter/builtin.h>
#include <interpreter/language.h>

#include <parser/variable.h>
#include <parser/reference.h>

#include <hermes/error.h>

#include <fmt/printf.h>

#include <fstream>
#include <filesystem>
#include <unordered_map>

#include <nlohmann/json.hpp>

using nlohmann::json;

namespace fs = std::filesystem;

namespace interpreter {
    void init() {
        fs::create_directory(".cpj");
    }

    std::string getProject() {
        fs::path here = fs::current_path();

        while (!fs::exists(here / ".cpj") && here.has_parent_path() && here.has_relative_path()) {
            here = here.parent_path();
        }

        fs::path result = here / ".cpj";
        if (!fs::exists(result))
            return "";

        return result;
    }

    std::string getTemplate(const std::string &file) {
        std::string project = getProject();

        if (project.empty())
            return "";

        fs::path filePath = fs::path(project) / file;

        if (!fs::exists(filePath))
            std::ofstream stream(filePath);

        return filePath;
    }

    std::string getConfig() {
        std::string project = getProject();

        if (project.empty())
            return "";

        fs::path filePath = fs::path(project) / "config.json";

        if (!fs::exists(filePath))
            std::ofstream stream(filePath);

        return filePath;
    }

    std::string getScript() {
        std::string project = getProject();

        if (project.empty())
            return "";

        std::string configPath = fs::path(project) / "config.json";

        std::string scriptName = "script.cpj";

        if (fs::exists(configPath)) {
            std::ifstream stream(configPath);
            std::stringstream buffer;
            buffer << stream.rdbuf();

            json config = json::parse(buffer.str());

            if (!config.is_object())
                throw std::runtime_error("Root json object in config must be an object.");

            size_t matchLevel = 0;
            for (const auto &option : config.items()) {
                if (!option.value().is_string())
                    throw std::runtime_error("All values in config json object in config must be string paths.");

                fs::path path(option.key());

                fs::path result;

                if (path.is_relative()) {
                    result = fs::path(project).parent_path() / path;
                } else {
                    result = path;
                }

                std::string lexicalRelevance = result.lexically_relative(fs::current_path()).string();

                std::string endingMark = "..";

                if (lexicalRelevance.rfind(endingMark) == lexicalRelevance.size() - endingMark.size()
                    && matchLevel <= lexicalRelevance.size()) {
                    scriptName = option.value();
                    matchLevel = lexicalRelevance.size();
                }
            }
        }

        fs::path filePath = fs::path(project) / scriptName;

        if (!fs::exists(filePath))
            std::ofstream stream(filePath);

        return filePath;
    }

    std::optional<RootNode> parseScript(const std::string &path) {
        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();

        State state(buffer.str());

        try {
            return std::optional<RootNode>(state);
        } catch(const ParseError &error) {
            LineDetails details(state.text, error.index);

            fmt::print("{} [line {}]\n{}\n{}\n",
                       error.issue, details.lineNumber + 1, details.line, details.marker);

            return std::optional<RootNode>();
        }
    }


    std::optional<std::string> execute(RootNode *root, Node *context, Locals &locals) {
        switch (context->is<Kinds>()) {
            case Kinds::Expression:
                evaluate(root, context->as<ExpressionNode>(), locals);
                break;

            case Kinds::ParseExpression:
                evaluate(root, context->as<ParseExpressionNode>(), locals);
                break;

            case Kinds::If:
                if (evaluate(root, context->children[0]->as<LiteralExpressionNode>(), locals) == "true") {
                    for (size_t a = 1; a < context->children.size(); a++) {
                        auto value = execute(root, context->children[0].get(), locals);

                        if (value.has_value())
                            return value.value();
                    }
                }

                break;

            case Kinds::For:
                while (evaluate(root, context->children[0]->as<LiteralExpressionNode>(), locals) == "true") {
                    for (size_t a = 1; a < context->children.size(); a++) {
                        auto value = execute(root, context->children[0].get(), locals);

                        if (value.has_value())
                            return value.value();
                    }
                }

                break;

            case Kinds::Variable: {
                std::string localName = context->as<VariableNode>()->name;
                std::string result = evaluate(root, context->children[0]->as<ExpressionNode>(), locals);

                locals[localName] = result;

                break;
            }

            case Kinds::Is:
                return evaluate(root, context->children[0]->as<ExpressionNode>(), locals);

            default:
                break;
        }

        return std::optional<std::string>();
    }

    std::string execute(RootNode *root, const std::string &name, const std::vector<std::string> &arguments) {
        if (builtin::functions.find(name) != builtin::functions.end())
            return builtin::functions[name](arguments);

        ActionNode *action = nullptr;
        for (const auto &context : root->children) {
            std::string title = context->as<ActionNode>()->name;

            if (name == title) {
                action = context->as<ActionNode>();
                break;
            }
        }

        if (!action)
            throw std::runtime_error(fmt::format("Cannot resolve reference to {}.", name));

        Locals locals;

        for (size_t a = 0; a < action->params; a++) {
            bool isVariableLength = a == action->params - 1 && action->lastIsVariable;

            if (a >= arguments.size() && !isVariableLength) {
                throw std::runtime_error(fmt::format("Too few arguments passed to {}.", action->name));
            }

            std::string value;

            if (isVariableLength) {
                value = fmt::format("{}",
                    fmt::join(std::vector<std::string>(arguments.begin() + a, arguments.end()), " "));
            } else {
                value = arguments[a];
            }

            locals[action->children[a]->as<VariableNode>()->name] = value;
        }

        for (size_t a = action->params; a < action->children.size(); a++) {
            const auto &context = action->children[a];

            auto value = execute(root, context.get(), locals);

            // an is expression was hit
            if (value.has_value())
                return value.value();
        }

        return "";
    }

    std::string evaluate(RootNode *root, StringNode *string, const Locals &locals) {
        std::stringstream stream;

        size_t lastInsert = 0;

        for (size_t a = 0; a < string->inserts.size(); a++) {
            stream << string->text.substr(lastInsert, string->inserts[a] - lastInsert);
            lastInsert = string->inserts[a];

            stream << evaluate(root, string->children[a]->as<ExpressionNode>(), locals);
        }

        stream << string->text.substr(lastInsert, string->text.size() - lastInsert);

        return stream.str();
    }

    std::string evaluate(RootNode *root, ExpressionNode *expression, const Locals &locals) {
        auto &first = expression->children.front();

        switch (first->is<Kinds>()) {
            case Kinds::String:
                return evaluate(root, first->as<StringNode>(), locals);

            case Kinds::TemplateExpression:
                return evaluate(root, first->as<TemplateExpressionNode>(), locals);

            case Kinds::ReplaceExpression:
                return evaluate(root, first->as<ReplaceExpressionNode>(), locals);

            case Kinds::Reference: {
                std::string name = first->as<ReferenceNode>()->name;

                if (expression->children.size() == 1 && locals.find(name) != locals.end()) {
                    return locals.at(name);
                } else {
                    std::vector<std::string> arguments;
                    arguments.reserve(expression->children.size() - 1);

                    for (size_t a = 1; a < expression->children.size(); a++) {
                        auto &child = expression->children[a];

                        assert(child->is(Kinds::LiteralExpression));

                        arguments.push_back(evaluate(root, child->as<LiteralExpressionNode>(), locals));
                    }

                    return execute(root, name, arguments);
                }
            }

            default:
                break;
        }

        throw std::runtime_error("Unknown expression first kind.");
    }

    std::string evaluate(RootNode *root, LiteralExpressionNode *literal, const Locals &locals) {
        auto &sub = literal->children[0];

        switch (sub->is<Kinds>()) {
            case Kinds::Expression:
                return evaluate(root, sub->as<ExpressionNode>(), locals);

            case Kinds::Reference: {
                std::string referenceName = sub->as<ReferenceNode>()->name;

                if (locals.find(referenceName) == locals.end())
                    throw std::runtime_error(fmt::format("Could not find {} in locals.", referenceName));

                return locals.at(referenceName);
            }

            case Kinds::String:
                return evaluate(root, sub->as<StringNode>(), locals);

            default:
                break;
        }

        throw std::runtime_error("Unknown literal expression context kind.");
    }


    void evaluate(RootNode *root, ParseExpressionNode *context, Locals &locals) {
        std::string content = evaluate(root, context->children[0]->as<LiteralExpressionNode>(), locals);
        std::string templateExp = evaluate(root, context->children[1]->as<LiteralExpressionNode>(), locals);

        std::unordered_map<std::string, std::string> map = language::parse(templateExp, content);

        for (size_t a = 2; a < context->children.size(); a += 2) {
            auto *string = context->children[a]->as<StringNode>();
            auto *right = context->children[a + 1]->as<ReferenceNode>();

            if (!string->inserts.empty())
                throw std::runtime_error("Template expressions must have constant strings on left side.");

            if (map.find(string->text) == map.end())
                throw std::runtime_error(fmt::format("Cannot find insert {} in template.", string->text));

            locals[right->name] = map[string->text];
        }
    }

    std::string evaluate(RootNode *root, TemplateExpressionNode *templateExp, const Locals &locals) {
        std::string value = evaluate(root, templateExp->children[0]->as<LiteralExpressionNode>(), locals);

        std::unordered_map<std::string, std::string> replacements;

        for (size_t a = 1; a < templateExp->children.size(); a += 2) {
            auto *string = templateExp->children[a]->as<StringNode>();
            auto *right = templateExp->children[a + 1]->as<LiteralExpressionNode>();

            if (!string->inserts.empty())
                throw std::runtime_error("Template expressions must have constant strings on left side.");

            replacements[string->text] = evaluate(root, right, locals);
        }

        return language::replace(value, replacements);
    }

    std::string evaluate(RootNode *root, ReplaceExpressionNode *context, const Locals &locals) {
        std::string content = evaluate(root, context->children[0]->as<LiteralExpressionNode>(), locals);
        std::string templateExp = evaluate(root, context->children[1]->as<LiteralExpressionNode>(), locals);

        Locals subLocals = locals;
        std::unordered_map<std::string, std::string> map = language::parse(templateExp, content);

        for (size_t a = 2; a < context->children.size(); a += 3) {
            auto *string = context->children[a]->as<StringNode>();
            auto *name = context->children[a + 1]->as<ReferenceNode>();
            auto *right = context->children[a + 2]->as<LiteralExpressionNode>();

            if (!string->inserts.empty())
                throw std::runtime_error("Template expressions must have constant strings on left side.");

            if (map.find(string->text) == map.end())
                throw std::runtime_error(fmt::format("Cannot find insert {} in template.", string->text));

            subLocals[name->name] = map[string->text];
            map[string->text] = evaluate(root, right, subLocals);
        }

        return language::replace(templateExp, map);
    }

    void run(int count, const char **args) {
        if (count <= 1) { // no arguments
            fmt::print("To create a cpj script/project in the current directory use `cpj init`\n");
        } else if (std::strcmp(args[1], "init") == 0) {
            if (count != 2) {
                fmt::print("Usage: cpj init\n");
                return;
            }

            init();
            fmt::print("Add your script here: {}\n", getScript());
        } else if (std::strcmp(args[1], "script") == 0) {
            if (count != 2) {
                fmt::print("Usage: cpj source\n");
                return;
            }

            std::string result = getScript();

            if (result.empty())
                fmt::print("Initialize the project with `cpj init`\n");
            else
                fmt::print("{}\n", result);
        } else if (std::strcmp(args[1], "config") == 0) {
            if (count != 2) {
                fmt::print("Usage: cpj config\n");
                return;
            }

            std::string result = getConfig();

            if (result.empty())
                fmt::print("Initialize the project with `cpj init`\n");
            else
                fmt::print("{}\n", result);
        } else if (std::strcmp(args[1], "template") == 0) {
            if (count != 3) {
                fmt::print("Usage: cpj template <name>\n");
                return;
            }

            std::string name = args[2];

            std::string result = getTemplate(name);

            if (result.empty())
                fmt::print("Initialize the project with `cpj init`\n");
            else
                fmt::print("{}\n", result);
        } else if (std::strcmp(args[1], "eval") == 0) {
            if (count != 3) {
                fmt::print("Usage: cpj template <name>\n");
                return;
            }

            std::string expression = args[2];

            std::string path = getScript();
            if (path.empty()) {
                fmt::print("Initialize the project with `cpj init`\n");
                return;
            }

            std::optional<RootNode> script = parseScript(path);
            if (!script)
                return;

            State state(expression);
            Node parent(state);

            try {
                ExpressionNode node(&parent);

                try {
                    std::string value = evaluate(&script.value(), &node, { });

                    fmt::print("{}\n", value.empty() ? "Empty output." : value);
                } catch (const std::runtime_error &error) {
                    fmt::print("{}\n", error.what());
                }
            } catch (const ParseError &error) {
                LineDetails details(state.text, error.index);

                fmt::print("{} [line {}]\n{}\n{}\n",
                           error.issue, details.lineNumber + 1, details.line, details.marker);
            }
        } else {
            std::string path = getScript();
            if (path.empty()) {
                fmt::print("Initialize the project with `cpj init`\n");
                return;
            }

            std::optional<RootNode> script = parseScript(path);
            if (!script)
                return;

            std::vector<std::string> arguments;
            arguments.reserve(count - 2);

            for (size_t a = 2; a < count; a++) {
                arguments.emplace_back(args[a]);
            }

            try {
                execute(&script.value(), args[1], arguments);
            } catch (const std::runtime_error &error) {
                fmt::print("{}\n", error.what());
            }
        }
    }
}
