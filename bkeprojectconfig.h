#ifndef BKEPROJECTCONFIG_H
#define BKEPROJECTCONFIG_H

#include "ParserHelper/ParserHelper.h"
#include <QtCore>

class BkeConfigUiModel;
class BkeProjectConfig : public QObject
{
    Q_OBJECT
    friend class BkeConfigUiModel;
    QString filePath;
    QString projDir;
    QFileSystemWatcher watcher;
private slots:
    void setupWatcher();
public:
    BkeProjectConfig(const QString &projDir, const QString &path);
    BkeProjectConfig();
    QString projectName;
    int resolutionSize[2];
    QString gameTitle;
    QString saveDir;
    QStringList imageAutoSearchPath;
    QStringList audioAutoSearchPath;
    QStringList scriptAutoSearchPath;
    int maxSaveDataNum;
    int defaultFontSize;
    QBkeVariable defaultFontColor;
    QString defaultFontName;
    int debugLevel;
    QString live2DKey;
    QBkeVariable metaData;

    BkeProjectConfig &operator = (const BkeProjectConfig &r);

public slots:
    void readFile();
    void fileChanged();
    void writeFile();

signals:
    void readFileError(QString s);
    void onFileChanged();
};

#endif // BKEPROJECTCONFIG_H
