#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "LoggingCategories.h"

/// Работа с qradiobuttontree:
/// Создание категории:
///     QTreeWidgetItem* pItem;
///     pItem = ui->treeWidget->addItem("1");
/// Создание элемента категории:
///     QTreeWidgetItem* pW02 = ui->treeWidget->addItem(pItem, "First property");
/// Обращение к категории:
///       QTreeWidgetItem *item = ui->treeWidget->topLevelItem( i );
/// Обращение к элементу категории:
///       QWidget *widget = ui->treeWidget->itemWidget(item->child(j), 0);
///       QRadioButton* btn = qobject_cast<QRadioButton*>(widget);
///
///
///  FREERTOS: https://gitlab.borisblade.ru/common1/arm-firmware-sources/freertos.git
///  vscode: https://gitlab.borisblade.ru/common1/arm-firmware-sources/vs-code-template.git
///  src: https://gitlab.borisblade.ru/common1/arm-firmware-sources/src.git
/// кнопка загрузить: клоним, читаем .yaml получаем словарь, проходимся по всем элементам под ключом microcontrollers и закидываем
/// в соответствующие ветки дерева в зависимости от family. Если указанного family нет в дереве, то создается новая категория
/// кнопка создать: по названию radiobutton, совпадающую с ключом получаем из элемента словаря под ключом microcontrollers всю информацию, далее по накатанной

bool MainWindow::isCyrillic(wchar_t wch)
{
    int code = (int)wch;
    return (code >= 0x400 && code <= 0x4ff);
}

bool createAddConfigNotification(QWidget *parent = nullptr)
{
    QMessageBox msgBox(parent);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("Нет конфигурации!");
    msgBox.setText("Не найден конфигурационный файл.\nНажмите одну из кнопок в категории \"Загрузить список\" чтобы загрузить конфигурацию");

    QPushButton *buttonTrue = msgBox.addButton("Ок", QMessageBox::YesRole);
    msgBox.exec();

    if (msgBox.clickedButton() == buttonTrue)
        return true;
    else
        return false;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    statusBar()->showMessage(tr("Версия ПО: ") + this->version);
    if (!getConfig()){
        createAddConfigNotification(this);
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::getConfig()
{
    QString dir = QCoreApplication::applicationDirPath()+"/config.xml";
    QFile file(dir);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QDomDocument doc("document");
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    mcAndFamilyList.clear();
    radioList.clear();
    ui->radioTree->clear();
    QRadioButton* radio;
    bool isFirstRadio = true;
    QList<QString> familyList;
    QDomNode familyXml = docElem.firstChild();
    while(!familyXml.isNull()) {
        QDomElement e = familyXml.toElement();
        QString familyName = e.tagName();
        familyList.append(familyName);
        QTreeWidgetItem* newFamily = ui->radioTree->addItem(familyName);
        QDomNode mcXml = familyXml.firstChild();
        while(!mcXml.isNull()) {
            QDomElement i = mcXml.toElement();
            if(!i.isNull()) {
                QString mcName = i.tagName();
                mcAndFamilyList.append(std::pair<QString,QString>(mcName, familyName));
                QTreeWidgetItem* newMc = ui->radioTree->addRadio(newFamily, mcName);
                QWidget *widget = ui->radioTree->itemWidget(newMc, 0);
                radio = qobject_cast<QRadioButton*>(widget);
                radio->setStyleSheet("QRadioButton::indicator"
                                                        "{"
                                                        "width : 20px;"
                                                        "height : 20px;"
                                                        "}");
                if(isFirstRadio == true) {
                    radio->setChecked(true);
                }
                radioList.append(radio);
            }
            if(isFirstRadio) isFirstRadio = false;
            mcXml = mcXml.nextSibling();
        }
        familyXml = familyXml.nextSibling();
    }
    return true;
}

void MainWindow::downloadConfig()
{
    QString dir = QFileDialog::getOpenFileName(this,
          tr(""), "/home", tr("Configs (*.xml)"));
    if (dir == "") {return;}
    QFile::remove(QCoreApplication::applicationDirPath()+"/config.xml");
    QFile::copy(dir, QCoreApplication::applicationDirPath()+"/config.xml");
    getConfig();
}

QString getTextFromDialog(QWidget *parent = nullptr, const QString &title = "Введите текст",
                         const QString &labelText = "Текст:")
{
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.setFixedSize(600, 250);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *label = new QLabel(labelText);
    QLineEdit *lineEdit = new QLineEdit();
    QPushButton *okButton = new QPushButton("OK");

    layout->addWidget(label);
    layout->addWidget(lineEdit);
    layout->addWidget(okButton);

    // Подключить сигнал кнопки к закрытию диалога
    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    // Фокус на lineEdit
    lineEdit->setFocus();

    // Модальное выполнение
    if (dialog.exec() == QDialog::Accepted) {
        return lineEdit->text();
    }

    return QString(); // Пустая строка при отмене
}



void MainWindow::cloneConfig()
{
    QString repo = getTextFromDialog(this, "Загрузка конфигурации", "Введите адрес директория с конфигурацией");
    repoBuilder rB = repoBuilder();
    QFile::remove(QCoreApplication::applicationDirPath()+"/config.xml");
    rB.cloneRepo(repo, QCoreApplication::applicationDirPath());
    getConfig();
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
    ui->labelProgress->clear();
    if (ui->lineEditPath->text() == "") return;

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
        else {
            qDebug(logInfo()) << "Создание отменено";
            return;
        }
    }
    int index = 1;
    QString mcName;
    QString familyName;
    for (index; index<=radioList.count(); index++){
        QRadioButton* radio = radioList.at(index-1);
        if (radio->isChecked()){
            mcName = radio->text();
            for(auto pair: mcAndFamilyList){if(pair.first == mcName) {familyName = pair.second; break;}}
            break;
        }
    }
    QString confDir = QCoreApplication::applicationDirPath()+"/config.xml";
    QFile file(confDir);
    file.open(QIODevice::ReadOnly);
    QDomDocument doc("document");
    doc.setContent(&file);
    QDomElement docElem = doc.documentElement();
    QDomElement familyXml = docElem.firstChildElement(familyName);
    QDomElement mcXml = familyXml.firstChildElement(mcName);
    QString HALUrl = mcXml.attribute("HAL");
    QString BSPUrl = familyXml.attribute("BSP");
    QString freeRTOSUrl = docElem.attribute("freeRTOS");
    QString vscodeUrl = docElem.attribute("vscode");
    QString srcUrl = docElem.attribute("src");
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
                       "portable/GCC/" + mcXml.attribute("architecture"))) return;

    qDebug(logInfo()) << "Создание файлов и папок...";
    ui->labelProgress->setText("Создание файлов и папок...");
    rB.buildRepo();
    qDebug(logInfo()) << "Папки и файлы созданы";
    ui->labelProgress->setText("Папки и файлы созданы");
    progress = 100;
    ui->progressBar->setValue(progress);
}










