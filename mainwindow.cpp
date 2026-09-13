#include "mainwindow.h"
#include "expression.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDockWidget>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QVBoxLayout>
#include <cmath>

namespace {
QString number(double value) { return QString::number(value == 0.0 ? 0.0 : value, 'g', 17); }
bool endsOperand(const QString &text)
{
    if (text.isEmpty()) return false;
    const QChar last = text.back();
    return last.isDigit() || last.isLetter() || last == ')' || last == '%' || last == '.';
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow), settings(new QSettings("QtSimpleCalculatorTeam", "Calculator", this))
{
    ui->setupUi(this);
    setMinimumSize(420, 520);
    resize(760, 600);

    auto *options = new QHBoxLayout;
    auto *scientific = new QCheckBox(tr("Scientific"), this);
    scientific->setObjectName("scientificMode");
    darkTheme = new QCheckBox(tr("Dark theme"), this);
    darkTheme->setObjectName("darkTheme");
    options->addWidget(scientific);
    options->addStretch();
    options->addWidget(darkTheme);
    ui->verticalLayout->insertLayout(0, options);

    message = new QLabel(this);
    message->setObjectName("message");
    message->setWordWrap(true);
    message->setMinimumHeight(34);
    ui->verticalLayout->addWidget(message);

    auto *memoryRow = new QHBoxLayout;
    memoryIndicator = new QLabel(this);
    memoryIndicator->setObjectName("memoryIndicator");
    memoryRow->addWidget(memoryIndicator);
    for (const QString &action : {QString("MC"), QString("MR"), QString("M+"), QString("M-")}) {
        auto *button = new QPushButton(action, this);
        button->setObjectName("memory" + action);
        button->setFocusPolicy(Qt::NoFocus);
        button->setToolTip(action == "MC" ? tr("Clear memory") : action == "MR" ? tr("Recall memory")
                           : action == "M+" ? tr("Add current value to memory") : tr("Subtract current value from memory"));
        memoryRow->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, action] { memoryAction(action); });
    }
    ui->verticalLayout->addLayout(memoryRow);

    sciencePanel = new QWidget(this);
    auto *scienceGrid = new QGridLayout(sciencePanel);
    scienceGrid->setContentsMargins(0, 0, 0, 0);
    angleMode = new QComboBox(sciencePanel);
    angleMode->setObjectName("angleMode");
    angleMode->addItems({tr("Degrees"), tr("Radians")});
    angleMode->setCurrentIndex(settings->value("radians", false).toBool() ? 1 : 0);
    scienceGrid->addWidget(angleMode, 0, 0, 1, 2);
    auto *angleHint = new QLabel(tr("sin / cos / tan"), sciencePanel);
    scienceGrid->addWidget(angleHint, 0, 2, 1, 2);
    const QStringList functions = {"sin", "cos", "tan", "sqrt", "ln", "log", "abs", "x^2"};
    for (int i = 0; i < functions.size(); ++i) {
        const QString function = functions.at(i);
        auto *button = new QPushButton(function, sciencePanel);
        button->setFocusPolicy(Qt::NoFocus);
        button->setObjectName("function" + function);
        scienceGrid->addWidget(button, i / 4 + 1, i % 4);
        connect(button, &QPushButton::clicked, this, [this, function] { applyFunction(function); });
    }
    const QStringList constants = {"pi", "e", "^", "1/x"};
    for (int i = 0; i < constants.size(); ++i) {
        const QString token = constants.at(i);
        auto *button = new QPushButton(token, sciencePanel);
        button->setFocusPolicy(Qt::NoFocus);
        scienceGrid->addWidget(button, 3, i);
        connect(button, &QPushButton::clicked, this, [this, token] {
            if (token == "1/x") applyFunction(token);
            else insertText(token, token == "^");
        });
    }
    ui->verticalLayout->addWidget(sciencePanel);

    auto *keypad = new QGridLayout;
    const QStringList labels = {"C", "Del", "%", "/", "7", "8", "9", "*",
                                "4", "5", "6", "-", "1", "2", "3", "+",
                                "+/-", "0", ".", "=", "(", ")"};
    const QStringList names = {"actionClear", "actionDel", "actionPercent", "actionDiv",
        "num7", "num8", "num9", "actionMul", "num4", "num5", "num6", "actionMinus",
        "num1", "num2", "num3", "actionPlus", "actionSign", "num0", "comma", "actionCalc",
        "openParenthesis", "closeParenthesis"};
    for (int i = 0; i < labels.size(); ++i) {
        const QString label = labels.at(i);
        auto *button = new QPushButton(label, this);
        button->setObjectName(names.at(i));
        button->setMinimumHeight(40);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setFocusPolicy(Qt::NoFocus);
        keypad->addWidget(button, i / 4, i >= 20 ? (i - 20) * 2 : i % 4,
                         1, i >= 20 ? 2 : 1);
        connect(button, &QPushButton::clicked, this, [this, label, button] {
            if (label == "C") on_actionClear_clicked();
            else if (label == "Del") on_actionDel_clicked();
            else if (label == "%") on_actionPercent_clicked();
            else if (label == "+/-") on_actionSign_clicked();
            else if (label == "=") on_actionCalc_clicked();
            else if (label == ".") on_comma_clicked();
            else if (QString("+-*/").contains(label)) actionGroup_clicked(button);
            else if (label.at(0).isDigit()) numberGroup_clicked(button);
            else insertText(label, label == ")");
        });
    }
    ui->verticalLayout->addLayout(keypad, 1);

    auto *historyDock = new QDockWidget(tr("History"), this);
    historyDock->setObjectName("historyDock");
    historyDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    auto *historyBody = new QWidget(historyDock);
    auto *historyLayout = new QVBoxLayout(historyBody);
    auto *historyHint = new QLabel(tr("Click a result to use it again"), historyBody);
    historyHint->setWordWrap(true);
    historyLayout->addWidget(historyHint);
    history = new QListWidget(historyBody);
    history->setObjectName("historyList");
    history->setWordWrap(true);
    history->setMinimumWidth(220);
    historyLayout->addWidget(history, 1);
    auto *clearHistory = new QPushButton(tr("Clear history"), historyBody);
    historyLayout->addWidget(clearHistory);
    historyDock->setWidget(historyBody);
    addDockWidget(Qt::RightDockWidgetArea, historyDock);
    auto *showHistory = new QCheckBox(tr("History"), this);
    showHistory->setChecked(settings->value("historyVisible", true).toBool());
    options->insertWidget(1, showHistory);
    historyDock->setVisible(showHistory->isChecked());
    connect(showHistory, &QCheckBox::toggled, this, [this, historyDock](bool visible) {
        historyDock->setVisible(visible);
        settings->setValue("historyVisible", visible);
    });
    connect(clearHistory, &QPushButton::clicked, this, [this] { history->clear(); saveHistory(); });
    connect(history, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        insertValue(item->data(Qt::UserRole).toDouble());
    });

    connect(scientific, &QCheckBox::toggled, this, [this](bool enabled) {
        sciencePanel->setVisible(enabled);
        settings->setValue("scientific", enabled);
    });
    scientific->setChecked(settings->value("scientific", false).toBool());
    sciencePanel->setVisible(scientific->isChecked());
    darkTheme->setChecked(settings->value("darkTheme", false).toBool());
    connect(darkTheme, &QCheckBox::toggled, this, [this](bool dark) {
        settings->setValue("darkTheme", dark);
        applyTheme();
    });
    connect(angleMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int mode) {
        settings->setValue("radians", mode == 1);
    });
    connect(ui->expressionInput, &QLineEdit::textEdited, this, [this] {
        resultShown = false;
        message->clear();
    });
    ui->expressionInput->installEventFilter(this);
    loadHistory();
    updateMemory();
    applyTheme();
    ui->expressionInput->setFocus();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::insertText(QString text, bool continueResult)
{
    auto *input = ui->expressionInput;
    if (resultShown && !continueResult) input->clear();
    resultShown = false;
    const QString before = input->text().left(input->cursorPosition());
    if (!input->hasSelectedText() && (text == "(" || text == "pi" || text == "e") && endsOperand(before))
        text.prepend('*');
    input->insert(text);
    message->clear();
    input->setFocus();
}

void MainWindow::numberGroup_clicked(QAbstractButton *button)
{
    auto *input = ui->expressionInput;
    if (!resultShown && !input->hasSelectedText() && input->cursorPosition() == input->text().size()) {
        const QRegularExpression zero("(^|[+*/^(\\-])0$");
        if (zero.match(input->text()).hasMatch()) input->backspace();
    }
    insertText(button->text());
}

void MainWindow::actionGroup_clicked(QAbstractButton *button)
{
    const QString op = button->text();
    auto *input = ui->expressionInput;
    if (input->text().isEmpty()) {
        if (op == "-") insertText(op);
        return;
    }
    if (!input->hasSelectedText() && input->cursorPosition() == input->text().size()) {
        const QChar last = input->text().back();
        if (QString("+*/^-").contains(last) && op != "-") input->backspace();
    }
    insertText(op, true);
}

void MainWindow::on_actionDel_clicked()
{
    if (resultShown) ui->expressionInput->clear();
    else ui->expressionInput->backspace();
    resultShown = false;
    message->clear();
    ui->expressionInput->setFocus();
}

void MainWindow::on_actionClear_clicked()
{
    ui->expressionInput->clear();
    ui->displayPanel->setText("0");
    message->clear();
    resultShown = false;
    ui->expressionInput->setFocus();
}

bool MainWindow::currentValue(double &value)
{
    const Calculation result = evaluateExpression(ui->expressionInput->text(), angleMode->currentIndex() == 0);
    if (!result.ok) {
        message->setText(result.error);
        return false;
    }
    value = result.value;
    return true;
}

void MainWindow::on_actionCalc_clicked()
{
    if (ui->expressionInput->text().trimmed().isEmpty() || resultShown) return;
    const QString expression = ui->expressionInput->text();
    double value = 0.0;
    if (!currentValue(value)) {
        ui->displayPanel->setText(tr("Error"));
        return;
    }
    message->clear();
    addHistory(expression, value);
    ui->displayPanel->setText(QString::number(value, 'g', 16));
    ui->expressionInput->setText(number(value));
    resultShown = true;
    ui->expressionInput->setFocus();
}

void MainWindow::on_comma_clicked()
{
    auto *input = ui->expressionInput;
    const QString prefix = resultShown ? QString() : input->text().left(input->cursorPosition());
    const auto match = QRegularExpression("[0-9]+[.,]?[0-9]*$").match(prefix);
    if (!input->hasSelectedText() && match.hasMatch() && (match.captured().contains('.') || match.captured().contains(','))) return;
    insertText(prefix.isEmpty() || !prefix.back().isDigit() ? "0." : ".");
}

void MainWindow::on_actionPercent_clicked()
{
    if (endsOperand(ui->expressionInput->text().left(ui->expressionInput->cursorPosition()))) insertText("%", true);
}

void MainWindow::on_actionSign_clicked()
{
    auto *input = ui->expressionInput;
    if (input->hasSelectedText()) insertText("(-(" + input->selectedText() + "))", true);
    else if (input->text().isEmpty() || !endsOperand(input->text())) insertText("-", true);
    else {
        input->setText("-(" + input->text() + ")");
        resultShown = false;
        message->clear();
    }
    input->setFocus();
}

void MainWindow::applyFunction(const QString &name)
{
    auto *input = ui->expressionInput;
    QString argument = input->selectedText();
    const bool wholeExpression = argument.isEmpty() && endsOperand(input->text());
    if (wholeExpression) argument = input->text();
    QString replacement;
    if (name == "x^2") replacement = "(" + argument + ")^2";
    else if (name == "1/x") replacement = "1/(" + argument + ")";
    else replacement = name + "(" + argument + ")";
    if (wholeExpression) input->selectAll();
    const int start = input->hasSelectedText() ? input->selectionStart() : input->cursorPosition();
    insertText(replacement, true);
    if (argument.isEmpty()) input->setCursorPosition(start + replacement.indexOf('(') + 1);
}

void MainWindow::insertValue(double value)
{
    auto *input = ui->expressionInput;
    const QString prefix = input->text().left(input->cursorPosition());
    // Recall supplies the pending operand; otherwise it starts a new expression.
    if (!resultShown && !prefix.isEmpty() && !endsOperand(prefix)) {
        insertText("(" + number(value) + ")", true);
    } else {
        input->setText(number(value));
        ui->displayPanel->setText(QString::number(value, 'g', 16));
        resultShown = true;
        message->clear();
        input->setFocus();
    }
}

void MainWindow::memoryAction(const QString &action)
{
    if (action == "MC") { memory = 0.0; hasMemory = false; }
    else if (action == "MR") {
        if (hasMemory) insertValue(memory);
    } else {
        double value = 0.0;
        if (!currentValue(value)) return;
        const double next = action == "M+" ? memory + value : memory - value;
        if (!std::isfinite(next)) { message->setText(tr("Memory overflow")); return; }
        memory = next;
        hasMemory = true;
        message->clear();
    }
    updateMemory();
}

void MainWindow::updateMemory()
{
    memoryIndicator->setText(hasMemory ? "M" : "--");
    memoryIndicator->setToolTip(hasMemory ? number(memory) : tr("Memory is empty"));
}

void MainWindow::addHistory(const QString &expression, double value)
{
    auto *item = new QListWidgetItem(expression + " = " + number(value));
    item->setData(Qt::UserRole, value);
    item->setData(Qt::UserRole + 1, expression);
    const QString context = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm") + " / " + angleMode->currentText();
    item->setData(Qt::UserRole + 2, context);
    item->setToolTip(context);
    history->insertItem(0, item);
    while (history->count() > 100) delete history->takeItem(history->count() - 1);
    saveHistory();
}

void MainWindow::saveHistory()
{
    settings->beginWriteArray("history", history->count());
    for (int i = 0; i < history->count(); ++i) {
        settings->setArrayIndex(i);
        auto *item = history->item(i);
        settings->setValue("expression", item->data(Qt::UserRole + 1));
        settings->setValue("value", number(item->data(Qt::UserRole).toDouble()));
        settings->setValue("context", item->data(Qt::UserRole + 2));
    }
    settings->endArray();
    settings->sync();
    if (settings->status() != QSettings::NoError) message->setText(tr("Could not save calculator settings"));
}

void MainWindow::loadHistory()
{
    const int count = qMin(settings->beginReadArray("history"), 100);
    for (int i = 0; i < count; ++i) {
        settings->setArrayIndex(i);
        bool ok = false;
        const double value = settings->value("value").toString().toDouble(&ok);
        const QString expression = settings->value("expression").toString();
        if (!ok || !std::isfinite(value) || expression.isEmpty() || expression.size() > 512) continue;
        auto *item = new QListWidgetItem(expression + " = " + number(value), history);
        item->setData(Qt::UserRole, value);
        item->setData(Qt::UserRole + 1, expression);
        item->setData(Qt::UserRole + 2, settings->value("context"));
        item->setToolTip(settings->value("context").toString());
    }
    settings->endArray();
}

void MainWindow::applyTheme()
{
    const bool dark = darkTheme->isChecked();
    const QString background = dark ? "#18212f" : "#f3f6fb";
    const QString surface = dark ? "#243247" : "#ffffff";
    const QString foreground = dark ? "#edf3ff" : "#17263d";
    const QString border = dark ? "#41536d" : "#c4cfdf";
    setStyleSheet(QString(
        "QWidget { background:%1; color:%3; font-size:14px; }"
        "QPushButton, QLineEdit, QListWidget, QComboBox { background:%2; border:1px solid %4; border-radius:6px; padding:8px; }"
        "QPushButton:hover { border:1px solid #4387e8; }"
        "QPushButton:pressed { background:#3873bd; color:white; }"
        "QPushButton#actionCalc { background:#2166bd; color:white; font-weight:bold; }"
        "QLabel#displayPanel { font-size:28px; font-weight:bold; }"
        "QLabel#message { color:%5; }"
        "QLabel#memoryIndicator { color:%6; font-weight:bold; }"
        "QListWidget::item { padding:10px; border-bottom:1px solid %4; }"
        "QListWidget::item:selected { background:#2166bd; color:white; }"
        "QToolTip { background:%2; color:%3; border:1px solid %4; }")
        .arg(background, surface, foreground, border, dark ? "#ffb2ad" : "#a82323", dark ? "#8ab8ff" : "#2166bd"));
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->expressionInput && event->type() == QEvent::KeyPress) {
        auto *key = static_cast<QKeyEvent *>(event);
        if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter || key->key() == Qt::Key_Equal) {
            on_actionCalc_clicked();
            return true;
        }
        if (key->key() == Qt::Key_Escape) { on_actionClear_clicked(); return true; }
        if (resultShown && !key->text().isEmpty() &&
            !(key->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
            const QChar first = key->text().at(0);
            if (first.isDigit() || first.isLetter() || QString(".,(").contains(first)) ui->expressionInput->clear();
            resultShown = false;
        }
    }
    return QMainWindow::eventFilter(object, event);
}
