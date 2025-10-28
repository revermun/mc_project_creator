#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "LoggingCategories.h"

///TODO: Добавить логирование при создании https://evileg.com/ru/post/154/
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

/// Из гита клонить HAL, BSP, FREETROS
/// HAL и BSP определяются по модели МК(microcontrollers в конфиге), FREERTOS архитектурой ядра(architecture в конфиге)
/// Директории: FREERTOS - src/FreeRTOS; HAL - src/hal; BSP - src/bsp
void MainWindow::createProject()
{
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
    if (repoName!="") directory = QDir::fromNativeSeparators(ui->lineEditPath->text()) + '/' + repoName;
    else directory = QDir::fromNativeSeparators(ui->lineEditPath->text());
    qDebug(logInfo()) << "Создание проекта " + repoName + " в директории " + directory + "...";
    repoBuilder rB = repoBuilder(directory);
    QSettings set(QCoreApplication::applicationDirPath()+"/config.ini",QSettings::IniFormat);
    QString HALUrl = set.value("HAL/mc"+QString::number(index),QString("empty string")).toString();
    QString BSPUrl = set.value("BSP/mc"+QString::number(index),QString("empty string")).toString();
    QString freeRTOSURL = set.value("freeRTOS/url",QString("empty string")).toString();
    QString HALDirectory = directory + '/' + "src/hal";
    QString BSPDirectory = directory + '/' + "src/bsp";
    QString freeRTOSDirectory = directory + '/' + "src/FreeRTOS";
    qDebug(logInfo()) << "Клонирование hal в директорию " + HALDirectory + "...";
    if (!rB.cloneRepo(HALUrl, HALDirectory)){
           qDebug(logWarning()) << "Ошибка при клонировании hal!";
    }
    else qDebug(logInfo()) << "hal успешно склонированно";

    qDebug(logInfo()) << "Клонирование bsp в директорию " + BSPDirectory + "...";
    if(!rB.cloneRepo(BSPUrl, BSPDirectory)){
        qDebug(logWarning()) << "Ошибка при клонировании bsp!";
    }
    else qDebug(logInfo()) << "bsp успешно склонированно";

    qDebug(logInfo()) << "Клонирование freeRTOS в директорию " + freeRTOSDirectory + "...";
    if(!rB.cloneFREERTOS(freeRTOSURL,freeRTOSDirectory,
                     "portable/GCC/" + set.value("architecture/mc" + QString::number(index),QString("empty string")).toString())){
        qDebug(logWarning()) << "Ошибка при клонировании freeRTOS!";
    }
    else qDebug(logInfo()) << "freeRTOS успешно склонированно";

    qDebug(logInfo()) << "Создание файлов и папок...";
    rB.buildRepo();
    qDebug(logInfo()) << "Папки и файлы созданы";
}










