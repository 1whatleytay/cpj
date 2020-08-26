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
#include <parser/if.h>
#include <parser/for.h>

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

    std::string getScript() {
        std::string project = getProject();

        if (project.empty())
            return "";

        fs::path filePath = fs::path(project) / "script.cpj";

        if (!fs::exists(filePath))
            std::ofstream stream(filePath);

        return filePath;
    }

    std::unique_ptr<RootContext> parseScript(const std::string &path) {
        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();

        State state(buffer.str());

        try {
            return std::make_unique<RootContext>(state);
        } catch(const ParseError &error) {
            LineDetails details(state.text, error.index);

            fmt::print("{} [line {}]\n{}\n{}\n", error.issue, details.lineNumber + 1, details.line, details.marker);

            return nullptr;
        }
    }


    std::optional<std::string> execute(RootContext *root, Context *context, Locals &locals) {
        if (context->kind == KindExpression) {
            evaluate(root, context->as<ExpressionContext>(), locals);
        } else if (context->kind == KindParseExpression) {
            evaluate(root, context->as<ParseExpressionContext>(), locals);
        } else if (context->kind == KindIf) {
            if (evaluate(root, context->children[0]->as<LiteralExpressionContext>(), locals) == "true") {
                for (size_t a = 1; a < context->children.size(); a++) {
                    auto value = execute(root, context->children[0].get(), locals);

                    if (value.has_value())
                        return value.value();
                }
            }
        } else if (context->kind == KindFor) {
            while (evaluate(root, context->children[0]->as<LiteralExpressionContext>(), locals) == "true") {
                for (size_t a = 1; a < context->children.size(); a++) {
                    auto value = execute(root, context->children[0].get(), locals);

                    if (value.has_value())
                        return value.value();
                }
            }
        } else if (context->kind == KindVariable) {
            std::string localName = context->as<VariableContext>()->name;
            std::string result = evaluate(root, context->children[0]->as<ExpressionContext>(), locals);

            locals[localName] = result;
        } else if (context->kind == KindIs) {
            return evaluate(root, context->children[0]->as<ExpressionContext>(), locals);
        }

        return std::optional<std::string>();
    }

    std::string execute(RootContext *root, const std::string &name, const std::vector<std::string> &arguments) {
        if (builtin::functions.find(name) != builtin::functions.end())
            return builtin::functions[name](arguments);

        ActionContext *action = nullptr;
        for (const std::unique_ptr<Context> &context : root->children) {
            std::string title = context->as<ActionContext>()->name;

            if (name == title) {
                action = context->as<ActionContext>();
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

            locals[action->children[a]->as<VariableContext>()->name] = value;
        }

        for (size_t a = action->params; a < action->children.size(); a++) {
            const auto &context = action->children[a];

            auto value = execute(root, context.get(), locals);

            // an is experession was hit
            if (value.has_value())
                return value.value();
        }

        return "";
    }

    std::string evaluate(RootContext *root, StringContext *string, const Locals &locals) {
        std::stringstream stream;

        size_t lastInsert = 0;

        for (size_t a = 0; a < string->inserts.size(); a++) {
            stream << string->text.substr(lastInsert, string->inserts[a] - lastInsert);
            lastInsert = string->inserts[a];

            stream << evaluate(root, string->children[a]->as<ExpressionContext>(), locals);
        }

        stream << string->text.substr(lastInsert, string->text.size() - lastInsert);

        return stream.str();
    }

    std::string evaluate(RootContext *root, ExpressionContext *expression, const Locals &locals) {
        auto &first = expression->children[0];

        if (first->kind == KindString) {
            return evaluate(root, first->as<StringContext>(), locals);
        } else if (first->kind == KindTemplateExpression) {
            return evaluate(root, first->as<TemplateExpressionContext>(), locals);
        } else if (first->kind == KindReference) {
            std::string name = first->as<ReferenceContext>()->name;

            if (expression->children.size() == 1 && locals.find(name) != locals.end()) {
                return locals.at(name);
            } else {
                std::vector<std::string> arguments;
                arguments.reserve(expression->children.size() - 1);

                for (size_t a = 1; a < expression->children.size(); a++) {
                    auto &child = expression->children[a];

                    assert(child->kind == KindLiteralExpression);

                    arguments.push_back(evaluate(root, child->as<LiteralExpressionContext>(), locals));
                }

                return execute(root, name, arguments);
            }
        }

        throw std::runtime_error("Unknown expression first kind.");
    }

    std::string evaluate(RootContext *root, LiteralExpressionContext *literal, const Locals &locals) {
        auto &sub = literal->children[0];

        if (sub->kind == KindExpression) {
            return evaluate(root, sub->as<ExpressionContext>(), locals);
        } else if (sub->kind == KindReference) {
            std::string referenceName = sub->as<ReferenceContext>()->name;

            if (locals.find(referenceName) == locals.end())
                throw std::runtime_error(fmt::format("Could not find {} in locals.", referenceName));

            return locals.at(referenceName);
        } else if (sub->kind == KindString) {
            return evaluate(root, sub->as<StringContext>(), locals);
        }

        throw std::runtime_error("Unknown literal expression context kind.");
    }


    void evaluate(RootContext *root, ParseExpressionContext *context, Locals &locals) {
        std::string content = evaluate(root, context->children[0]->as<LiteralExpressionContext>(), locals);
        std::string templateExp = evaluate(root, context->children[1]->as<LiteralExpressionContext>(), locals);

        std::unordered_map<std::string, std::string> map = language::parse(templateExp, content);

        for (size_t a = 2; a < context->children.size(); a += 2) {
            auto *string = context->children[a]->as<StringContext>();
            auto *right = context->children[a + 1]->as<ReferenceContext>();

            if (!string->inserts.empty())
                throw std::runtime_error("Template expressions must have constant strings on left side.");

            if (map.find(string->text) == map.end())
                throw std::runtime_error(fmt::format("Cannot find insert {} in template.", string->text));

            locals[right->name] = map[string->text];
        }
    }

    std::string evaluate(RootContext *root, TemplateExpressionContext *templateExp, const Locals &locals) {
        std::string value = evaluate(root, templateExp->children[0]->as<LiteralExpressionContext>(), locals);

        std::unordered_map<std::string, std::string> replacements;

        for (size_t a = 1; a < templateExp->children.size(); a += 2) {
            auto *string = templateExp->children[a]->as<StringContext>();
            auto *right = templateExp->children[a + 1]->as<LiteralExpressionContext>();

            if (!string->inserts.empty())
                throw std::runtime_error("Template expressions must have constant strings on left side.");

            replacements[string->text] = evaluate(root, right, locals);
        }

        std::string x = language::replace(value, replacements);
        return x;
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
        } else {
            std::string path = getScript();
            if (path.empty()) {
                fmt::print("Initialize the project with `cpj init`\n");
                return;
            }

            std::unique_ptr<RootContext> script = parseScript(path);
            if (!script)
                return;

            std::vector<std::string> arguments;
            arguments.reserve(count - 2);

            for (size_t a = 2; a < count; a++) {
                arguments.emplace_back(args[a]);
            }

//            try {
                execute(script.get(), args[1], arguments);
//            } catch (const std::runtime_error &error) {
//                fmt::print("{}\n", error.what());
//            }
        }
    }
}
