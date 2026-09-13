#include <QtTest>
#include "../expression.h"

class TestExpression : public QObject
{
    Q_OBJECT

private slots:
    void testBasicArithmetic();
    void testDivisionByZero();
    void testSquareRoot();
    void testSquareRootNegative();
    void testParentheses();
    void testPower();
    void testPercent();
    void testScientific();
};

void TestExpression::testBasicArithmetic()
{
    auto r = evaluateExpression("2 + 3 * 4", true);
    QVERIFY(r.ok);
    QCOMPARE(r.value, 14.0);

    r = evaluateExpression("10 - 4", true);
    QVERIFY(r.ok);
    QCOMPARE(r.value, 6.0);
}

void TestExpression::testDivisionByZero()
{
    auto r = evaluateExpression("5 / 0", true);
    QVERIFY(!r.ok);
    QVERIFY(!r.error.isEmpty());
}

void TestExpression::testSquareRoot()
{
    auto r = evaluateExpression("sqrt(16)", true);
    QVERIFY(r.ok);
    QCOMPARE(r.value, 4.0);
}

void TestExpression::testSquareRootNegative()
{
    auto r = evaluateExpression("sqrt(-9)", true);
    QVERIFY(!r.ok);
}

void TestExpression::testParentheses()
{
    auto r = evaluateExpression("2 + 3 * (4 - 1)", true);
    QVERIFY(r.ok);
    QCOMPARE(r.value, 11.0);
}

void TestExpression::testPower()
{
    auto r = evaluateExpression("2^3", true);
    QVERIFY(r.ok);
    QCOMPARE(r.value, 8.0);
}

void TestExpression::testPercent()
{
    auto r = evaluateExpression("200 + 10%", true);
    QVERIFY(r.ok);
    QCOMPARE(r.value, 220.0);
}

void TestExpression::testScientific()
{
    auto r = evaluateExpression("sin(30)", true); // degrees
    QVERIFY(r.ok);
    QVERIFY(qAbs(r.value - 0.5) < 1e-9);
}

QTEST_APPLESS_MAIN(TestExpression)
#include "test_expression.moc"
