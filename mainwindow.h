#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QWindow>
#include <QRadioButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QCoreApplication>
#include <iostream>
#include <QDomDocument>

#include "repoBuilder.h"


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
    void downloadConfig();
    void cloneConfig();

private:
    bool cloneFiles(const QString &repoUrl, const QString &directory, const QString &filesName);
    bool cloneFreeRTOS(const QString &repoUrl, const QString &directory, const QString &repoSubDirectory);
    bool createConfirmDialog();
    bool isCyrillic(wchar_t wch);
    bool getConfig();
    int progress;
    QList<QRadioButton*> radioList;
    QList<std::pair<QString,QString>> mcAndFamilyList;
    QString version = "1.0.4";
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
