#include <parser/string.h>

#include <parser/expression.h>

#include <sstream>

bool alwaysStop(const char *, size_t) {
    return true;
}

StringNode::StringNode(Node *parent) : Node(parent, Kinds::String) {
    auto defaultPop = spaceStoppable;

    // do not skip text
    spaceStoppable = alwaysStop;

    needs("'");
    level = MatchLevel::Strong;

    std::stringstream stream;

    enum class BreakChars {
        Dollar,
        Backslash,
        Quote,
    };

    bool loop = true;
    while (loop) {
        stream << until({ "$", "\\", "'" });

        if (peek("'"))
            spaceStoppable = defaultPop;

        switch (select<BreakChars>({"$", "\\", "'"})) {
            case BreakChars::Dollar:
                spaceStoppable = notSpace;
                needs("{");

                inserts.push_back(stream.str().size());
                push<ExpressionNode>();

                spaceStoppable = alwaysStop;
                needs("}");

                break;

            case BreakChars::Backslash: {
                enum class SpecialChars {
                    NewLine,
                    Tab,
                    DollarSign,
                    Quote,
                };

                switch (select<SpecialChars>({ "n", "t", "$", "'" })) {
                    case SpecialChars::NewLine:
                        stream << "\n";
                        break;
                    case SpecialChars::Tab:
                        stream << "\t";
                        break;
                    case SpecialChars::DollarSign:
                        stream << "$";
                        break;
                    case SpecialChars::Quote:
                        stream << "'";
                        break;
                }

                break;
            }

            case BreakChars::Quote:
                loop = false;
                break;
        }
    }

    text = stream.str();
}
