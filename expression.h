#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QString>

struct Calculation
{
    bool ok = false;
    double value = 0.0;
    QString error;
};

// Parses calculator syntax only; no scripts or external commands are executed.
Calculation evaluateExpression(const QString &expression, bool degrees);

#endif
