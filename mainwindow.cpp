#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>

namespace {
int digitCount(const QString &text)
{
    int count = 0;
    for (int i = 0; i < text.size(); ++i) {
        if (text.at(i).isDigit()) {
            ++count;
        }
    }
    return count;
}
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    inputState(EnteringNumber),
    hasStoredNumber(false),
    storedNumber(0.0)
{
    ui->setupUi(this);
    ui->displayPanel->clear();

    connect(ui->numberGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(numberGroup_clicked(QAbstractButton*)));
    connect(ui->actionGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(actionGroup_clicked(QAbstractButton*)));

    setFixedSize(QSize(306, 319));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startNewEntry()
{
    if (inputState != EnteringNumber) {
        ui->displayPanel->clear();
        ui->displayPanel->setToolTip(QString());
        inputState = EnteringNumber;
    }
}

void MainWindow::numberGroup_clicked(QAbstractButton *button)
{
    startNewEntry();
    QString displayLabel = ui->displayPanel->text();
    // Replace an integer zero, but preserve the sign and fractional zeros.
    if (displayLabel == "0" || displayLabel == "-0") {
        displayLabel.chop(1);
    }
    if (digitCount(displayLabel) >= DIGIT_LIMIT) {
        return;
    }
    displayLabel.append(button->text());
    ui->displayPanel->setText(displayLabel);
}

void MainWindow::actionGroup_clicked(QAbstractButton *button)
{
    if (inputState == Error || ui->displayPanel->text().isEmpty()) {
        return;
    }
    // Repeated operators replace the pending operator without calculating.
    if (inputState == WaitingForOperand) {
        storedOperator = button->text().at(0);
        return;
    }

    if (hasStoredNumber) {
        if (!calculate_result()) {
            return;
        }
    } else {
        bool ok = false;
        storedNumber = ui->displayPanel->text().toDouble(&ok);
        if (!ok || !std::isfinite(storedNumber)) {
            showError(tr("Invalid number"));
            return;
        }
    }
    hasStoredNumber = true;
    storedOperator = button->text().at(0);
    inputState = WaitingForOperand;
}

void MainWindow::on_actionDel_clicked()
{
    // The displayed first operand is already stored; it is not being edited.
    if (inputState == WaitingForOperand || inputState == Error) {
        return;
    }
    // A formatted result (possibly scientific notation) is not an input buffer.
    startNewEntry();
    QString displayLabel = ui->displayPanel->text();
    displayLabel.chop(1);
    if (displayLabel == "-") {
        displayLabel.clear();
    }
    ui->displayPanel->setText(displayLabel);
}

void MainWindow::on_actionCalc_clicked()
{
    if (!hasStoredNumber || ui->displayPanel->text().isEmpty() ||
        inputState == WaitingForOperand || inputState == Error) {
        return;
    }
    if (calculate_result()) {
        hasStoredNumber = false;
        storedOperator = QChar();
        inputState = ResultShown;
    }
}

void MainWindow::on_comma_clicked()
{
    startNewEntry();
    QString displayLabel = ui->displayPanel->text();
    if (digitCount(displayLabel) >= DIGIT_LIMIT || displayLabel.contains('.')) {
        return;
    }
    if (displayLabel.isEmpty()) {
        displayLabel = "0";
    }
    displayLabel.append('.');
    ui->displayPanel->setText(displayLabel);
}

void MainWindow::on_actionClear_clicked()
{
    ui->displayPanel->clear();
    ui->displayPanel->setToolTip(QString());
    inputState = EnteringNumber;
    hasStoredNumber = false;
    storedNumber = 0.0;
    storedOperator = QChar();
}

void MainWindow::on_actionPercent_clicked()
{
    // A percentage needs an entered operand, not the previous displayed value.
    if (inputState == WaitingForOperand || inputState == Error ||
        ui->displayPanel->text().isEmpty()) {
        return;
    }
    bool ok = false;
    double percentage = ui->displayPanel->text().toDouble(&ok);
    if (!ok || !std::isfinite(percentage)) {
        showError(tr("Invalid number"));
        return;
    }
    percentage /= 100.0;
    if (hasStoredNumber && (storedOperator == '+' || storedOperator == '-')) {
        percentage *= storedNumber;
    }
    if (showNumber(percentage)) {
        inputState = ResultShown;
    }
}

void MainWindow::on_actionSign_clicked()
{
    if (inputState == Error) {
        return;
    }
    if (inputState == WaitingForOperand) {
        startNewEntry();
    }
    QString displayLabel = ui->displayPanel->text();
    if (displayLabel.isEmpty()) {
        displayLabel = "0";
    }
    // Toggle the text to keep an unfinished fraction and negative zero editable.
    if (displayLabel.startsWith('-')) {
        displayLabel.remove(0, 1);
    } else {
        displayLabel.prepend('-');
    }
    ui->displayPanel->setText(displayLabel);
}

void MainWindow::showError(const QString &message)
{
    on_actionClear_clicked();
    inputState = Error;
    ui->displayPanel->setText(tr("Error"));
    ui->displayPanel->setToolTip(message);
}

bool MainWindow::showNumber(double number)
{
    if (!std::isfinite(number)) {
        showError(tr("Numeric overflow"));
        return false;
    }
    ui->displayPanel->setText(QString::number(number, 'g', DIGIT_LIMIT));
    return true;
}

bool MainWindow::calculate_result()
{
    bool ok = false;
    const double operand = ui->displayPanel->text().toDouble(&ok);
    if (!ok || !std::isfinite(operand)) {
        showError(tr("Invalid number"));
        return false;
    }

    double result = storedNumber;
    if (storedOperator == '+') {
        result += operand;
    } else if (storedOperator == '-') {
        result -= operand;
    } else if (storedOperator == 'x') {
        result *= operand;
    } else if (storedOperator == '/') {
        if (operand == 0.0) {
            showError(tr("Cannot divide by zero"));
            return false;
        }
        result /= operand;
    } else {
        return false;
    }

    if (!showNumber(result)) {
        return false;
    }
    storedNumber = result;
    return true;
}

//Keyboard buttons should call the corresponding functions
void MainWindow::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
        //Numbers
        case Qt::Key_1:
            numberGroup_clicked(ui->num1);
            break;
        case Qt::Key_2:
            numberGroup_clicked(ui->num2);
            break;
        case Qt::Key_3:
            numberGroup_clicked(ui->num3);
            break;
        case Qt::Key_4:
            numberGroup_clicked(ui->num4);
            break;
        case Qt::Key_5:
            numberGroup_clicked(ui->num5);
            break;
        case Qt::Key_6:
            numberGroup_clicked(ui->num6);
            break;
        case Qt::Key_7:
            numberGroup_clicked(ui->num7);
            break;
        case Qt::Key_8:
            numberGroup_clicked(ui->num8);
            break;
        case Qt::Key_9:
            numberGroup_clicked(ui->num9);
            break;
        case Qt::Key_0:
            numberGroup_clicked(ui->num0);
            break;
        //Operators
        case Qt::Key_Plus:
            actionGroup_clicked(ui->actionPlus);
            break;
        case Qt::Key_Minus:
            actionGroup_clicked(ui->actionMinus);
            break;
        case Qt::Key_Asterisk:
            actionGroup_clicked(ui->actionMul);
            break;
        case Qt::Key_Slash:
            actionGroup_clicked(ui->actionDiv);
            break;
        //Comma
        case Qt::Key_Period:
        case Qt::Key_Comma:
            on_comma_clicked();
            break;
        //Return (enter)
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Equal:
            on_actionCalc_clicked();
            break;
        //Backspace and delete
        case Qt::Key_Backspace:
            on_actionDel_clicked();
            break;
        case Qt::Key_Delete:
            on_actionClear_clicked();
            break;
        //Percentage
        case Qt::Key_Percent:
            on_actionPercent_clicked();
            break;
        default:
            QMainWindow::keyPressEvent(e);
            return;
    }
    e->accept();
}
