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

    enum BreakChars {
        Dollar,
        Backslash,
        Quote,
    };

    bool loop = true;
    while (loop) {
        stream << until({ "$", "\\", "'" });

        if (peek("'"))
            spaceStoppable = defaultPop;

        switch (select({"$", "\\", "'"})) {
            case Dollar:
                spaceStoppable = notSpace;
                needs("{");

                inserts.push_back(stream.str().size());
                push<ExpressionNode>();

                spaceStoppable = alwaysStop;
                needs("}");

                break;

            case Backslash: {
                enum SpecialChars {
                    NewLine,
                    Tab,
                    DollarSign,
                };

                switch (select({ "n", "t", "$" })) {
                    case NewLine:
                        stream << "\n";
                        break;
                    case Tab:
                        stream << "\t";
                        break;
                    case DollarSign:
                        stream << "$";
                        break;
                }

                break;
            }

            case Quote:
                loop = false;
                break;
        }
    }

    text = stream.str();
}
