#include "expression.h"

#include <cmath>
#include <stdexcept>

namespace {
const double pi = std::acos(-1.0);

struct Value
{
    double number;
    bool percentage;
    Value(double n, bool p = false) : number(n), percentage(p) {}
};

class Parser
{
public:
    Parser(QString text, bool degrees) : text(text), degrees(degrees) {}

    double parse()
    {
        if (text.size() > 512) fail("Expression is too long (maximum 512 characters)");
        text.replace(QChar(0x00d7), '*').replace(QChar(0x00f7), '/');
        text.replace(QChar(0x2212), '-').replace(',', '.');
        if (text.trimmed().isEmpty()) fail("Enter an expression");
        const double result = sum().number;
        spaces();
        if (position != text.size()) fail("Unexpected character or missing operator");
        return checked(result);
    }

private:
    QString text;
    bool degrees;
    int position = 0;
    int depth = 0;

    void fail(const char *message) const
    {
        throw std::runtime_error(message);
    }

    double checked(double value) const
    {
        if (!std::isfinite(value)) fail("Numeric overflow or undefined result");
        return value;
    }

    void spaces()
    {
        while (position < text.size() && text.at(position).isSpace()) ++position;
    }

    bool take(QChar character)
    {
        spaces();
        if (position < text.size() && text.at(position) == character) {
            ++position;
            return true;
        }
        return false;
    }

    Value sum()
    {
        Value left = product();
        for (;;) {
            const bool plus = take('+');
            if (!plus && !take('-')) return left;
            Value right = product();
            // A standalone percentage on the right of + or - is relative to the left.
            const double operand = right.percentage
                ? checked(left.number * right.number) : right.number;
            left = Value(checked(plus ? left.number + operand : left.number - operand));
        }
    }

    Value product()
    {
        Value left = unary();
        for (;;) {
            if (take('*')) {
                left = Value(checked(left.number * unary().number));
            } else if (take('/')) {
                const double divisor = unary().number;
                if (divisor == 0.0) fail("Cannot divide by zero");
                left = Value(checked(left.number / divisor));
            } else {
                return left;
            }
        }
    }

    Value unary()
    {
        if (++depth > 64) fail("Expression nesting is too deep");
        Value result(0.0);
        if (take('+')) result = unary();
        else if (take('-')) {
            result = unary();
            result.number = -result.number;
        } else result = power();
        --depth;
        return result;
    }

    Value power()
    {
        Value base = primary();
        while (take('%')) base = Value(base.number / 100.0, true);
        if (take('^')) {
            const double exponent = unary().number;
            if (base.number == 0.0 && exponent < 0.0) fail("Cannot divide by zero");
            if (base.number < 0.0 && std::trunc(exponent) != exponent)
                fail("A negative base requires an integer exponent");
            base = Value(checked(std::pow(base.number, exponent)));
        }
        return base;
    }

    Value primary()
    {
        if (take('(')) {
            Value result = sum();
            if (!take(')')) fail("Missing closing parenthesis");
            return result;
        }
        spaces();
        if (position >= text.size()) fail("Missing operand");
        if (text.at(position).isLetter()) {
            const int start = position;
            while (position < text.size() && text.at(position).isLetter()) ++position;
            const QString name = text.mid(start, position - start).toLower();
            if (name == "pi" || name == QString(QChar(0x03c0))) return Value(pi);
            if (name == "e") return Value(std::exp(1.0));
            if (!take('(')) fail("A function requires parentheses");
            const double argument = sum().number;
            if (!take(')')) fail("Missing closing parenthesis");
            return Value(function(name, argument));
        }

        const int start = position;
        bool digit = false;
        while (position < text.size() && text.at(position).isDigit()) {
            digit = true;
            ++position;
        }
        if (position < text.size() && text.at(position) == '.') {
            ++position;
            while (position < text.size() && text.at(position).isDigit()) {
                digit = true;
                ++position;
            }
        }
        if (!digit) fail("Expected a number or function");
        if (position < text.size() && text.at(position).toLower() == 'e') {
            ++position;
            if (position < text.size() && (text.at(position) == '+' || text.at(position) == '-'))
                ++position;
            const int exponentStart = position;
            while (position < text.size() && text.at(position).isDigit()) ++position;
            if (exponentStart == position) fail("Invalid scientific notation");
        }
        bool ok = false;
        const double result = text.mid(start, position - start).toDouble(&ok);
        if (!ok) fail("Invalid number or numeric overflow");
        return Value(checked(result));
    }

    double function(const QString &name, double argument)
    {
        if (name == "sqrt") {
            if (argument < 0.0) fail("Square root requires a non-negative argument");
            return std::sqrt(argument);
        }
        if (name == "ln" || name == "log") {
            if (argument <= 0.0) fail("Logarithm requires a positive argument");
            return name == "ln" ? std::log(argument) : std::log10(argument);
        }
        if (name == "abs") return std::abs(argument);
        const double radians = degrees ? (argument / 180.0) * pi : argument;
        if (name == "sin") return checked(std::sin(radians));
        if (name == "cos") return checked(std::cos(radians));
        if (name == "tan") {
            if (std::abs(std::cos(radians)) < 1e-14)
                fail("Tangent is undefined for this angle");
            return checked(std::tan(radians));
        }
        fail("Unknown function");
        return 0.0;
    }
};
}

Calculation evaluateExpression(const QString &expression, bool degrees)
{
    Calculation result;
    try {
        result.value = Parser(expression, degrees).parse();
        result.ok = true;
    } catch (const std::exception &error) {
        result.error = QString::fromUtf8(error.what());
    }
    return result;
}
