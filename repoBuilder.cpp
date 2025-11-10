#include "repoBuilder.h"

repoBuilder::repoBuilder(const QString &directory){
    this->directory = directory;
}

repoBuilder::repoBuilder(){
}

repoBuilder::~repoBuilder(){

}

bool repoBuilder::makePath(const QString &path)
{
    if(QDir().mkpath(path)) return true;
    else {return false;}
}

bool repoBuilder::makeFile(const QString &filePath)
{
   QFile file(filePath);
   if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {file.close(); return true;}
   else {return false;}
}

bool repoBuilder::buildRepo()
{
    bool res = true;
    QString defaultPath = directory;

    QString vscodePath = defaultPath + '/' + ".vscode";
    QString binPath = defaultPath + '/' + "bin";
    QString bootloaderPath = defaultPath +'/' + "bootloader";
    QString docPath = defaultPath + '/' + "doc";

    QString srcPath = defaultPath + '/' + "src";

    QString appPath = srcPath + '/' + "app";
    QString freeRTOSPath = srcPath + '/' + "freeRTOS";

    QString bspPath = srcPath + '/' + "bsp";
    QString mcuPath = bspPath + '/' + "mcu";
    QString devicesPath = bspPath + '/' + "devices";

    QString halPath = srcPath + '/' + "hal";
    QString SPLPath = halPath + '/' + "SPL";
    QString cmsisPath = halPath + '/' + "cmsis";
    QString corePath = cmsisPath + '/' + "core";
    QString devicePath = cmsisPath + '/' + "device";
    QString driverPath = cmsisPath + '/' + "driver";

    if(!makePath(vscodePath)) res = false;
    if(!makePath(binPath)) res = false;
    if(!makePath(bootloaderPath)) res = false;
    if(!makePath(docPath)) res = false;
    if(!makePath(srcPath)) res = false;
    if(!makePath(appPath)) res = false;
    if(!makePath(freeRTOSPath)) res = false;
    if(!makePath(bspPath)) res = false;
    if(!makePath(mcuPath)) res = false;
    if(!makePath(devicesPath)) res = false;
    if(!makePath(halPath)) res = false;
    if(!makePath(SPLPath)) res = false;
    if(!makePath(cmsisPath)) res = false;
    if(!makePath(corePath)) res = false;
    if(!makePath(devicePath)) res = false;
    if(!makePath(driverPath)) res = false;

    QString readmeFile = defaultPath + '/' + "README.md";

    if(!makeFile(readmeFile)) res = false;

    return res;
}

int repoBuilder::cloneRepo(const QString &repoUrl, const QString &path, const QString &repoSubdirectory)
{
    int res = 0;
    makePath(path);
    QString tmpPath = path + "/tmp";
    makePath(tmpPath);
    QProcess process;
    process.start("git", {"clone", "--filter=blob:none", repoUrl, tmpPath});
    process.waitForFinished();
    if (!(process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)) {
        res = 1; //не удалось склонировать директорию
        return res;
    }
    if(repoSubdirectory!=""){
        process.setWorkingDirectory(tmpPath);
        process.start("git", {"sparse-checkout", "init"});
        process.waitForFinished();
        process.start("git", {"sparse-checkout", "set", repoSubdirectory});
        process.waitForFinished();
        if (!(process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)) {
            res = 2; //не удалось произвести sparce-checkout
            return res;
        }
    }
    if (!deleteDir(tmpPath + "/.git")) res = 3; // не удалось удалить удалить .git
    QDir().remove(tmpPath + "/README.md");
    if (!copyDir(tmpPath,path, true)) res = 4; // не удалось скопировать файлы
    deleteDir(tmpPath);
    return res;

}

bool repoBuilder::copyDir(const QString &source, const QString &destination, bool override)
{
    QDir directory(source);
    if (!directory.exists())
    {
        return false;
    }

    QString srcPath = QDir::toNativeSeparators(source);
    if (!srcPath.endsWith(QDir::separator()))
        srcPath += QDir::separator();
    QString dstPath = QDir::toNativeSeparators(destination);
    if (!dstPath.endsWith(QDir::separator()))
        dstPath += QDir::separator();

    bool error = false;
    QStringList fileNames = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    //for (QStringList::size_type i=0; i != fileNames.size(); ++i)
    for (QStringList::size_type i=0; i != fileNames.size(); ++i)
    {
        QString fileName = fileNames.at(i);
        QString srcFilePath = srcPath + fileName;
        QString dstFilePath = dstPath + fileName;
        QFileInfo fileInfo(srcFilePath);
        if (fileInfo.isFile() || fileInfo.isSymLink())
        {
            if (override)
            {
                QFile::setPermissions(dstFilePath, QFile::WriteOwner);
            }
            QFile::copy(srcFilePath, dstFilePath);
        }
        else if (fileInfo.isDir())
        {
            QDir dstDir(dstFilePath);
            dstDir.mkpath(dstFilePath);
            if (!copyDir(srcFilePath, dstFilePath, override))
            {
                error = true;
            }
        }
    }

    return !error;
}

bool repoBuilder::deleteDir(const QString &dirName, bool isDeleteOnlyContents)
{
    QDir directory(dirName);
    if (!directory.exists())
    {
        return true;
    }

    QString srcPath = QDir::toNativeSeparators(dirName);
    if (!srcPath.endsWith(QDir::separator()))
        srcPath += QDir::separator();

    QStringList fileNames = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    bool error = false;
    for (QStringList::size_type i=0; i != fileNames.size(); ++i)
    {
        QString filePath = srcPath + fileNames.at(i);
        QFileInfo fileInfo(filePath);
        if (fileInfo.isFile() || fileInfo.isSymLink())
        {
            QFile::setPermissions(filePath, QFile::WriteOwner);
            if (!QFile::remove(filePath))
            {
                qDebug() << "remove file" << filePath << " failed!";
                error = true;
            }
        }
        else if (fileInfo.isDir())
        {
            if (!deleteDir(filePath))
            {
                error = true;
            }
        }
    }
    if (!isDeleteOnlyContents){
        if (!directory.rmdir(QDir::toNativeSeparators(directory.path())))
        {
            qDebug() << "remove dir" << directory.path() << " faild!";
            error = true;
        }
    }
    return !error;
}

bool repoBuilder::cloneFREERTOS(const QString &repoUrl, const QString &path, const QString &repoSubdirectory)
{
    bool res = true;
    if (cloneRepo(repoUrl, path) != 0) res = false;
    if (!copyDir(path + '/' + repoSubdirectory,path + "/include", true)) res = false;
    if (!deleteDir(path + "/portable")) res = false;
    return res;
}
