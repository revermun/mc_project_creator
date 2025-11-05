#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRadioButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include "repoBuilder.h"
#include <thread>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:
    void checkCyrillic();
    void createProject();
    void editPath();

private:
    bool cloneFiles(const QString &repoUrl, const QString &directory, const QString &filesName);
    bool cloneFreeRTOS(const QString &repoUrl, const QString &directory, const QString &repoSubDirectory);
    bool isCyrillic(wchar_t wch);
    int progress;
    QString version = "1.0.0";
    QList<QRadioButton*> radioList;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
