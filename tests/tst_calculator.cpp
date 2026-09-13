#include <QtTest>
#include <QLabel>
#include <QPushButton>
#include "mainwindow.h"

class CalculatorTest : public QObject
{
    Q_OBJECT

    static QLabel *display(MainWindow &window)
    {
        return window.findChild<QLabel *>("displayPanel");
    }

    static void click(MainWindow &window, const QString &name)
    {
        QPushButton *button = window.findChild<QPushButton *>(name);
        QVERIFY2(button, qPrintable(name));
        button->click();
    }

    static void enter(MainWindow &window, const QString &sequence, bool keyboard = false)
    {
        for (const QChar token : sequence) {
            if (token.isSpace()) continue;
            if (keyboard && (token.isDigit() || QString("+-*/%=.,").contains(token))) {
                QTest::keyClicks(&window, QString(token));
                continue;
            }
            if (token.isDigit()) {
                click(window, "num" + QString(token));
                continue;
            }
            switch (token.toLatin1()) {
            case '+': click(window, "actionPlus"); break;
            case '-': click(window, "actionMinus"); break;
            case '*': click(window, "actionMul"); break;
            case '/': click(window, "actionDiv"); break;
            case '=': click(window, "actionCalc"); break;
            case '.': case ',': click(window, "comma"); break;
            case '%': click(window, "actionPercent"); break;
            case 'S': click(window, "actionSign"); break;
            case 'D': click(window, "actionDel"); break;
            case 'C': click(window, "actionClear"); break;
            default: QFAIL("Unknown test input");
            }
        }
    }

private slots:
    void sequences_data()
    {
        QTest::addColumn<QString>("sequence");
        QTest::addColumn<QString>("expected");
        QTest::newRow("13-fraction-after-operator") << "5 + .2" << "0.2";
        QTest::newRow("13-fraction-calculation") << "5 + .2 =" << "5.2";
        QTest::newRow("13-old-fraction") << "5.1 + .2 =" << "5.3";
        QTest::newRow("14-new-number") << "2 + 3 = 4" << "4";
        QTest::newRow("14-new-fraction") << "2 + 3 = .4" << "0.4";
        QTest::newRow("14-use-result") << "2 + 3 = * 4 =" << "20";
        QTest::newRow("16-empty-plus") << "+ 2 =" << "2";
        QTest::newRow("16-empty-minus") << "- 2 =" << "2";
        QTest::newRow("16-empty-multiply") << "* 2 =" << "2";
        QTest::newRow("16-empty-divide") << "/ 2 =" << "2";
        QTest::newRow("16-deleted-operand") << "5 + 2 D * 3 =" << "8";
        QTest::newRow("17-negative-second-operand") << "5 + S 2 =" << "3";
        QTest::newRow("17-negative-fraction") << "5 + S .2 =" << "4.8";
        QTest::newRow("17-toggle-twice") << "5 + S S 2 =" << "7";
        QTest::newRow("17-preserve-fraction") << "1. S 2" << "-1.2";
        QTest::newRow("18-percent-waits") << "5 + %" << "5";
        QTest::newRow("18-enter-after-percent") << "5 + % 2 =" << "7";
        QTest::newRow("19-delete-waits") << "12 + D" << "12";
        QTest::newRow("19-enter-after-delete") << "12 + D 3 =" << "15";
        QTest::newRow("20-add-percent") << "200 + 10 % =" << "220";
        QTest::newRow("20-subtract-percent") << "200 - 10 % =" << "180";
        QTest::newRow("20-multiply-percent") << "200 * 10 % =" << "20";
        QTest::newRow("20-divide-percent") << "200 / 10 % =" << "2000";
        QTest::newRow("20-standalone-percent") << "10 %" << "0.1";
        QTest::newRow("21-leading-zeros") << "000012" << "12";
        QTest::newRow("21-negative-leading-zeros") << "S 000012" << "-12";
        QTest::newRow("21-fractional-zeros") << "000.0012" << "0.0012";
        QTest::newRow("22-equals") << "2 + 3 =" << "5";
        QTest::newRow("23-comma") << "1,5 + ,5 =" << "2";
        QTest::newRow("25-division-by-zero") << "5 / 0 =" << "Error";
        QTest::newRow("25-zero-divided-by-zero") << "0 / 0 =" << "Error";
        QTest::newRow("25-negative-zero") << "5 / S 0 =" << "Error";
        QTest::newRow("25-chained-division") << "5 / 0 +" << "Error";
        QTest::newRow("25-recovery-digit") << "5 / 0 = 2 + 3 =" << "5";
        QTest::newRow("25-recovery-fraction") << "5 / 0 = .2" << "0.2";
        QTest::newRow("25-recovery-clear") << "5 / 0 = C 3 + 4 =" << "7";
        QTest::newRow("25-error-commands") << "5 / 0 = + S % D =" << "Error";
        QTest::newRow("replace-operator") << "5 + * 2 =" << "10";
        QTest::newRow("chain") << "5 + 7 + 3 =" << "15";
        QTest::newRow("clear-pending") << "5 + C 2 =" << "2";
        QTest::newRow("incomplete-equals") << "5 + = 2 =" << "7";
        QTest::newRow("duplicate-decimal") << "1.2.3" << "1.23";
        QTest::newRow("trailing-decimal") << "5 + 2. =" << "7";
        QTest::newRow("input-limit") << "12345678901234567" << "1234567890123456";
        QTest::newRow("signed-input-limit") << "S 12345678901234567" << "-1234567890123456";
        QTest::newRow("delete-negative") << "S 2 D 3" << "3";
        QTest::newRow("signed-result-new-input") << "2 + 3 = S 4" << "4";
        QTest::newRow("delete-result") << "2 + 3 = D 4" << "4";
    }

    void sequences()
    {
        QFETCH(QString, sequence);
        QFETCH(QString, expected);
        for (bool keyboard : {false, true}) {
            MainWindow window;
            enter(window, sequence, keyboard);
            QCOMPARE(display(window)->text(), expected);
        }
    }

    void overflow_data()
    {
        QTest::addColumn<bool>("chained");
        QTest::addColumn<bool>("negative");
        QTest::newRow("15-positive-equals") << false << false;
        QTest::newRow("15-negative-equals") << false << true;
        QTest::newRow("15-positive-chain") << true << false;
        QTest::newRow("15-negative-chain") << true << true;
    }

    void overflow()
    {
        QFETCH(bool, chained);
        QFETCH(bool, negative);
        MainWindow window;
        enter(window, negative ? "S 9999999999999999" : "9999999999999999");
        if (chained) enter(window, "*");
        for (int i = 0; i < 25 && display(window)->text() != "Error"; ++i) {
            enter(window, chained ? "9999999999999999 *" : "* 9999999999999999 =");
        }
        QCOMPARE(display(window)->text(), QString("Error"));
        QCOMPARE(display(window)->toolTip(), QString("Numeric overflow"));
        enter(window, "2 + 3 =");
        QCOMPARE(display(window)->text(), QString("5"));
        QVERIFY(display(window)->toolTip().isEmpty());
    }

    void unhandledKey()
    {
        MainWindow window;
        QKeyEvent event(QEvent::KeyPress, Qt::Key_F8, Qt::NoModifier);
        QApplication::sendEvent(&window, &event);
        QVERIFY(!event.isAccepted()); // QWidget's default handler ignores this key.
        QVERIFY(display(window)->text().isEmpty());
    }

    void keyboardControls()
    {
        MainWindow window;
        enter(window, "2 + 3", true);
        QTest::keyClick(&window, Qt::Key_Return);
        QCOMPARE(display(window)->text(), QString("5"));
        enter(window, "* 2", true);
        QTest::keyClick(&window, Qt::Key_Enter);
        QCOMPARE(display(window)->text(), QString("10"));
        QTest::keyClick(&window, Qt::Key_Delete);
        QVERIFY(display(window)->text().isEmpty());
        enter(window, "123", true);
        QTest::keyClick(&window, Qt::Key_Backspace);
        QCOMPARE(display(window)->text(), QString("12"));
    }
};

QTEST_MAIN(CalculatorTest)
#include "tst_calculator.moc"
