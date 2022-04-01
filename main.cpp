#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QCryptographicHash>

struct XFileInfo
{
    QFileInfo info;
    QByteArray md5;
};

// Returns empty QByteArray() on failure.
QByteArray fileChecksum(const QString &fileName,
                        QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f)) {
            return hash.result();
        }
    }
    return QByteArray();
}

void MatchFile(QList<XFileInfo*>& infos, const QFileInfo& fileInfo)
{
    if (fileInfo.size() == 0)
    {
        QFile::remove(fileInfo.absoluteFilePath());
        qDebug() << "delete file " << fileInfo.absoluteFilePath();
        return;
    }

    QByteArray fileMd5;
    for (QList<XFileInfo*>::iterator i = infos.begin(); i != infos.end(); ++i)
    {
        XFileInfo* x = *i;
        if (x->info.size() == fileInfo.size())
        {
            if (fileMd5.isEmpty())
            {
                fileMd5 = fileChecksum(fileInfo.absoluteFilePath(), QCryptographicHash::Md5);
            }

            if(x->md5 == fileMd5)
            {
                QFile::remove(fileInfo.absoluteFilePath());
                qDebug() << "delete file " << fileInfo.absoluteFilePath();
                return;
            }
        }
    }

    if (fileMd5.isEmpty())
    {
        fileMd5 = fileChecksum(fileInfo.absoluteFilePath(), QCryptographicHash::Md5);
    }

    XFileInfo* x = new XFileInfo;
    x->info = fileInfo;
    x->md5 = fileMd5;
    infos.append(x);
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
    QList<XFileInfo*> infos;

    foreach (QFileInfo fileInfo, dirFileList)
    {
        MatchFile(infos, fileInfo);
    }

    qDeleteAll(infos);

    return 0;
}
