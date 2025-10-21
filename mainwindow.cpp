#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings set(QCoreApplication::applicationDirPath()+"/config.ini",QSettings::IniFormat);
    set.setValue("microcontrollers/mc1",QString("STM32H7"));
    set.setValue("microcontrollers/mc2",QString("1986E9x"));
    set.setValue("architecture/mc1",QString("ARM_CM7"));
    set.setValue("architecture/mc2",QString("ARM_CM3"));
    int index = 1;
    while(set.value("microcontrollers/mc"+QString::number(index),QString("empty string")).toString()!="empty string"){
        QCheckBox* radio = new QCheckBox();
        radio->setText(set.value("microcontrollers/mc"+QString::number(index),QString("empty string")).toString());
        radioList.append(radio);
        index++;
    }
    for(auto radio: radioList){
            ui->verticalLayout->addWidget(radio);
            radio->show();
    }

    QSpacerItem* sItem = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    ui->groupBox->layout()->addItem(sItem);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::editPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (dir == "") {return;}
//    ui->lineEditPath->setText(QDir::toNativeSeparators(dir));
    ui->lineEditPath->setText(dir);

}
///TODO: добавить в функцию добавление папок согласно структуре
void MainWindow::createProject()
{
    if (ui->lineEditPath->text() == "") return;
    QProcess process;
    QString repoUrl = "https://gitlab.borisblade.ru/common1/arm-firmware-sources/hardware-abstract-level/stm32h745.git";
    QString directory = QDir::fromNativeSeparators(ui->lineEditPath->text() + '/' + ui->lineEditProjectName->text());
    process.start("git", {"clone", "--filter=blob:none", repoUrl, directory});
    process.waitForFinished();
    process.start("cd", {directory});
    process.waitForFinished();

    // Можно проверить результат выполнения
    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
        // Успешно
    } else {
        // Обработка ошибок
    }
}
