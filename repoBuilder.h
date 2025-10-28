#ifndef REPOBUILDER_H
#define REPOBUILDER_H

#include <QString>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>

class repoBuilder{
public:
    repoBuilder(const QString &directory);
    repoBuilder();
    ~repoBuilder();

    bool buildRepo();
    int cloneRepo(const QString &repoUrl, const QString &path, const QString &repoSubdirectory = "");
    bool cloneFREERTOS(const QString &repoUrl, const QString &path, const QString &repoSubdirectory);
    bool copyDir(const QString &source, const QString &destination, bool override);
    bool deleteDir(const QString &dirName, bool isDeleteOnlyContents = false);
private:
    bool makePath(const QString &path);
    bool makeFile(const QString &filePath);

    QString directory;
};


#endif // REPOBUILDER_H
