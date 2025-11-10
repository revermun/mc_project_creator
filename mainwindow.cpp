#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "LoggingCategories.h"

/*
1. папка build лежит в верхнем уровне, а не в src. её даже создавать не надо, её при сборке CMake создаст.      check
2. Надо добавить .gitignore в верхнем уровне. он стандартный и одинаковый, я его залил в репу vscode.           check
3. И наверное просто создать README.md файл, он будет лежать в верхнем уровне                                   check
4. Логи немного обрезаны, первые буквы как будто обрезаны                                                       непонял
5. Добавить проверку на существование проекта в папке, и спрашивать надо ли перезаписывать.
Я попытался в существующий проект в папке заново создать, он дал. Но я не понял он перезаписывает?              check
*/

bool MainWindow::isCyrillic(wchar_t wch)
{
    int code = (int)wch;
    return (code >= 0x400 && code <= 0x4ff);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    statusBar()->showMessage(tr("Версия ПО: ") + this->version);
    QSettings set(QCoreApplication::applicationDirPath()+"/config.ini",QSettings::IniFormat);
    set.setValue("microcontrollers/mc1",QString("STM32H7"));
    set.setValue("microcontrollers/mc2",QString("1986E9x"));
    set.setValue("architecture/mc1",QString("ARM_CM7/r0p1"));
    set.setValue("architecture/mc2",QString("ARM_CM3"));
    set.setValue("HAL/mc1",QString("https://gitlab.borisblade.ru/common1/arm-firmware-sources/hardware-abstract-level/stm32h745.git"));
    set.setValue("HAL/mc2",QString("https://gitlab.borisblade.ru/common1/arm-firmware-sources/hardware-abstract-level/mdr1986ve9x.git"));
    set.setValue("BSP/mc1",QString("https://gitlab.borisblade.ru/common1/arm-firmware-sources/bsp-templates/stm32h745.git"));
    set.setValue("BSP/mc2",QString("https://gitlab.borisblade.ru/common1/arm-firmware-sources/bsp-templates/mdr1986ve9x.git"));
    set.setValue("freeRTOS/url",QString("https://gitlab.borisblade.ru/common1/arm-firmware-sources/freertos.git"));
    set.setValue("vscode/url",QString("https://gitlab.borisblade.ru/common1/arm-firmware-sources/vs-code-template.git"));
    set.setValue("src/url",QString("https://gitlab.borisblade.ru/common1/arm-firmware-sources/src.git"));
    int index = 1;
    while(set.value("microcontrollers/mc"+QString::number(index),QString("empty string")).toString()!="empty string"){
        QRadioButton* radio = new QRadioButton();
        radio->setText(set.value("microcontrollers/mc"+QString::number(index),QString("empty string")).toString());
        if(index == 1) radio->setChecked(true);
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

    bool isContainCyrillic = false;
    for(QChar ch: dir){
        if (isCyrillic(ch.unicode())) isContainCyrillic = true;
    }
    if (!isContainCyrillic) {ui->pushButtonCreate->setEnabled(true); ui->lineEditPath->setText(dir);}
    else {ui->pushButtonCreate->setEnabled(false); ui->lineEditPath->setText("Обнаружена кирилица! Измените путь для проекта");}
}

void MainWindow::checkCyrillic(){
    bool isContainCyrillic = false;
    QString name = ui->lineEditProjectName->text();
    for(QChar ch: name){
        if (isCyrillic(ch.unicode())) isContainCyrillic = true;
    }
    if (!isContainCyrillic) ui->pushButtonCreate->setEnabled(true);
    else ui->pushButtonCreate->setEnabled(false);
}

bool MainWindow::cloneFiles(const QString &repoUrl, const QString &directory, const QString &filesName)
{
    repoBuilder rB = repoBuilder();
    qDebug(logInfo()) << "Клонирование" << filesName << "в директорию" << directory << "...";
    ui->labelProgress->setText("Клонирование " + filesName + " в директорию " + directory + "...");
    int cloneRes = rB.cloneRepo(repoUrl, directory);
    if( cloneRes != 0){
        qDebug(logWarning()) << "Ошибка при клонировании" << filesName << "!";
        ui->labelProgress->setText("Ошибка при клонировании " + filesName + "! Подробности в логе.");
        switch(cloneRes){
        case 1: qDebug(logWarning()) << "Не удалось склонировать дирректорию"; break;
        case 2: qDebug(logWarning()) << "Не удалось произвести sparce-checkout"; break;
        case 3: qDebug(logWarning()) << "Не удалось удалить удалить .git"; break;
        case 4: qDebug(logWarning()) << "Не удалось скопировать файлы"; break;
        }
        return false;
    }
    else qDebug(logInfo()) << filesName << "успешно склонированно";
    progress += 15;
    ui->progressBar->setValue(progress);
    ui->progressBar->update();
    return true;
}

bool MainWindow::cloneFreeRTOS(const QString &repoUrl, const QString &directory, const QString &repoSubDirectory)
{
    repoBuilder rB = repoBuilder();
    qDebug(logInfo()) << "Клонирование freeRTOS в директорию" << directory << "...";
    ui->labelProgress->setText("Клонирование freeRTOS в директорию " + directory + "...");
    if(!rB.cloneFREERTOS(repoUrl,
                         directory,
                         repoSubDirectory)){
        qDebug(logWarning()) << "Ошибка при клонировании freeRTOS!";
        ui->labelProgress->setText("Ошибка при клонировании freeRTOS!");
        return false;
    }
    else qDebug(logInfo()) << "freeRTOS успешно склонированно";
    progress += 15;
    ui->progressBar->setValue(progress);
    ui->progressBar->update();
    return true;
}


bool createChooseDirectoryClearmsgBox(QWidget *parent = nullptr)
{
    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("Выберите вариант");
    msgBox.setText("В заданной папке есть файлы. Очистить папку и продолжить?");

    QPushButton *buttonTrue = msgBox.addButton("Да", QMessageBox::AcceptRole);
    QPushButton *buttonFalse = msgBox.addButton("Нет", QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == buttonTrue)
        return true;
    else
        return false;
}

/// Из гита клонить HAL, BSP, FREETROS
/// HAL и BSP определяются по модели МК(microcontrollers в конфиге), FREERTOS архитектурой ядра(architecture в конфиге)
/// Директории: FREERTOS - src/FreeRTOS; HAL - src/hal; BSP - src/bsp
void MainWindow::createProject()
{
    progress = 0;
    ui->progressBar->setValue(progress);
    if (ui->lineEditPath->text() == "") return;
    int index = 1;
    for(int i = 0; i<ui->verticalLayout->count(); i++){
        QWidget *w = ui->verticalLayout->itemAt(i)->widget();
        QRadioButton *rb = qobject_cast<QRadioButton *>(w);
        if (rb->isChecked()) {
                break;
            }
        else (index++);
    }
    QString repoName = ui->lineEditProjectName->text();
    QString directory;
    if (repoName!="") {
        directory = QDir::fromNativeSeparators(ui->lineEditPath->text()) + '/' + repoName;
        qDebug(logInfo()) << "Создание проекта" << repoName << "в директории" << directory << "...";}
    else {
        directory = QDir::fromNativeSeparators(ui->lineEditPath->text());
        qDebug(logInfo()) << "Создание проекта в директории" << directory << "...";}

    repoBuilder rB = repoBuilder(directory);
    QDir dir = QDir(directory);
    if (!dir.isEmpty()) {
        qDebug(logInfo()) << "В папке есть файлы";
        bool res = createChooseDirectoryClearmsgBox(this);
        if (res) {
            qDebug(logInfo()) << "Удаление содержимого...";
            rB.deleteDir(directory, true);
        }
        else return;
    }



    QSettings set(QCoreApplication::applicationDirPath()+"/config.ini",QSettings::IniFormat);

    QString HALUrl = set.value("HAL/mc"+QString::number(index),QString("empty string")).toString();
    QString BSPUrl = set.value("BSP/mc"+QString::number(index),QString("empty string")).toString();
    QString freeRTOSUrl = set.value("freeRTOS/url",QString("empty string")).toString();
    QString vscodeUrl = set.value("vscode/url",QString("empty string")).toString();
    QString srcUrl = set.value("src/url",QString("empty string")).toString();
    QString HALDirectory = directory + '/' + "src/hal";
    QString BSPDirectory = directory + '/' + "src/bsp";
    QString freeRTOSDirectory = directory + '/' + "src/FreeRTOS";
    QString vscodeDirectory = directory;
    QString srcDirectory = directory + '/' + "src/";


    if (!cloneFiles(HALUrl, HALDirectory, "HAL")) return;
    if (!cloneFiles(BSPUrl, BSPDirectory, "BSP")) return;
    if (!cloneFiles(vscodeUrl, vscodeDirectory, ".vscode")) return;
    if (!cloneFiles(srcUrl, srcDirectory, "src")) return;

    if (!cloneFreeRTOS(freeRTOSUrl,
                       freeRTOSDirectory,
                       "portable/GCC/" + set.value("architecture/mc" + QString::number(index),QString("empty string")).toString())) return;

    qDebug(logInfo()) << "Создание файлов и папок...";
    ui->labelProgress->setText("Создание файлов и папок...");
    rB.buildRepo();
    qDebug(logInfo()) << "Папки и файлы созданы";
    ui->labelProgress->setText("Папки и файлы созданы");
    progress = 100;
    ui->progressBar->setValue(progress);
}










