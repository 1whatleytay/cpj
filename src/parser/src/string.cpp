#include <parser/string.h>

#include <parser/expression.h>

#include <sstream>

bool alwaysStop(const char *value, size_t size) {
    return true;
}

StringContext::StringContext(Context *parent) : Context(parent, KindString) {
    auto defaultPop = popStoppable;

    // do not skip text
    popStoppable = alwaysStop;

    needs("'");
    mark(MatchLevel::Strong);

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
            popStoppable = defaultPop;

        switch (select({"$", "\\", "'"})) {
            case Dollar:
                stoppable = anyHard;
                needs("{");

                inserts.push_back(stream.str().size());
                push<ExpressionContext>();

                stoppable = alwaysStop;
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
