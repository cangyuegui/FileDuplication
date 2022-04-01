#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QCryptographicHash>

// Returns empty QByteArray() on failure.
QByteArray fileChecksum(const QString &fileName,
                        QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f)) {
            f.close();
            return hash.result();
        }
    }
    return QByteArray("error md5");
}

QByteArray GetRapidNumber(const QFileInfo& info)
{
    if (info.size() < 1024 * 1024)
    {
        return fileChecksum(info.absoluteFilePath(), QCryptographicHash::Md5);
    }

    QByteArray array64;
    QFile file(info.absoluteFilePath());
    if (!file.open(QFile::ReadOnly))
    {
        array64 = "error 64";
        return array64;
    }

    qint64 step = (info.size() - 100) / 256;
    for (qint64 i = 0; i < 256; ++i)
    {
        qint64 start = step * i;
        file.seek(start);
        array64 += file.read(8);
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(array64);
    array64 = hash.result();
    QString pNumber = QString::number(info.size(), 16);
    QString stringData("****************");
    for (int i = 0; i < pNumber.size(); ++i)
    {
        stringData[i] = pNumber[i];
    }

    array64.prepend(stringData.toUtf8());
    return array64;
}

QByteArray Get64Info(const QString &fileName)
{
    QByteArray array64;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        array64 = "error 64";
    }

    if (file.size() <= 64)
    {
        array64 = file.readAll();
    }
    else
    {
        array64 = file.read(64);
    }

    file.close();
    return array64;
}

struct FileCell
{
    QByteArray array64;
    QByteArray md5;
    QFileInfo info;

    void UpdateMd5()
    {
        md5 = GetRapidNumber(info.absoluteFilePath());
    }

    void Update64()
    {
        array64 = Get64Info(info.absoluteFilePath());
    }
};



void MatchFile(QMap<qint64, FileCell*>& sizeInfos,
               QSet<QByteArray>& d64Infos,
               QSet<QByteArray>& md5Infos,
               const QFileInfo& fileInfo)
{
    qint64 fileSize = fileInfo.size();
    QString absPath = fileInfo.absoluteFilePath();
    if (fileSize == 0)
    {
        QFile::remove(absPath);
        qDebug() << "delete file " << absPath;
        return;
    }

    FileCell* fc = nullptr;

    if (!sizeInfos.contains(fileSize))
    {
        fc = new FileCell;
        fc->info = fileInfo;
        sizeInfos.insert(fileSize, fc);
        return;
    }
    else
    {
        fc = sizeInfos.value(fileSize);
    }

    if (fc->array64.isEmpty())
    {
        fc->Update64();
        d64Infos.insert(fc->array64);
    }

    QByteArray file64 = Get64Info(absPath);
    if (!d64Infos.contains(file64))
    {
        d64Infos.insert(file64);
        return;
    }

    if (fc->md5.isEmpty())
    {
        fc->UpdateMd5();
        md5Infos.insert(fc->md5);
    }

    QByteArray md5 = GetRapidNumber(absPath);
    if (!md5Infos.contains(md5))
    {
        md5Infos.insert(md5);
        return;
    }

    QFile::remove(absPath);
    qDebug() << "delete file " << absPath;

}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString doPath;

    if (a.arguments().size() < 2)
    {
        qDebug() << "error param";
        return 0;
    }

    QDir dir(a.arguments().at(1));
    if (!dir.exists())
    {
        qDebug() << "error dir: "<< a.arguments().at(1);
        return 0;
    }

    QFileInfoList dirFileList = dir.entryInfoList(QDir::Files | QDir::Writable);
    QMap<qint64, FileCell*> sizeInfos;
    QSet<QByteArray> d64Infos;
    QSet<QByteArray> md5Infos;

    foreach (QFileInfo fileInfo, dirFileList)
    {
        MatchFile(sizeInfos, d64Infos, md5Infos, fileInfo);
    }

    qDeleteAll(sizeInfos);

    qDebug() << "Finish";

    return 0;
}
