#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAbstractButton;
class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QSettings;
namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void numberGroup_clicked(QAbstractButton *button);
    void actionGroup_clicked(QAbstractButton *button);
    void on_actionDel_clicked();
    void on_actionCalc_clicked();
    void on_comma_clicked();
    void on_actionClear_clicked();
    void on_actionPercent_clicked();
    void on_actionSign_clicked();

private:
    Ui::MainWindow *ui;
    QSettings *settings;
    QListWidget *history;
    QLabel *memoryIndicator;
    QLabel *message;
    QComboBox *angleMode;
    QCheckBox *darkTheme;
    QWidget *sciencePanel;
    bool resultShown = false;
    bool hasMemory = false;
    double memory = 0.0;

    void insertText(QString text, bool continueResult = false);
    void insertValue(double value);
    void applyFunction(const QString &name);
    bool currentValue(double &value);
    void memoryAction(const QString &action);
    void updateMemory();
    void applyTheme();
    void addHistory(const QString &expression, double value);
    void loadHistory();
    void saveHistory();
};

#endif
