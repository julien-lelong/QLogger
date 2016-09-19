#include "qlogger.h"

#include <QDebug>
#include <QDir>
#include <QTextCodec>
#include <QTextStream>
#include <QtConcurrent/QtConcurrentRun>

#define DATE_FORMAT "yyyy-MM-dd-hh-mm-ss"

namespace QLogger
{
    void trace(const QString &message, const QString &name)
    {
        log(name, message, LogLevel::TraceLevel);
    }

    void debug(const QString &message, const QString &name)
    {
        log(name, message, LogLevel::DebugLevel);
    }

    void info(const QString &message, const QString &name)
    {
        log(name, message, LogLevel::InfoLevel);
    }

    void warning(const QString &message, const QString &name)
    {
        log(name, message, LogLevel::WarnLevel);
    }

    void error(const QString &message, const QString &name)
    {
        log(name, message, LogLevel::ErrorLevel);
    }

    void fatal(const QString &message, const QString &name)
    {
        log(name, message, LogLevel::FatalLevel);
    }

    void log(const QString &name, const QString &message, const LogLevel &level)
    {
        QLoggerManager::log(name, message, level);
    }

    void setLimitSize(const qint64 &size, const QString &name)
    {
        QLoggerManager::setLimitSize(size, name);
    }

    // WRITTER
    QLoggerWriter::QLoggerWriter(QObject *parent) : QObject(parent)
    {
        m_fileSizeLimit = 0;
        m_saveDatetime = true;
        m_datetimeFormat = QString(DATE_FORMAT);
        m_level = LogLevel::DebugLevel;
    }

    QLoggerWriter::QLoggerWriter(const QString &filePath, const LogLevel &level, QObject *parent) : QLoggerWriter(parent)
    {
        m_level = level;
        m_filePath = filePath;
    }

    LogLevel QLoggerWriter::level()
    {
        return m_level;
    }

    QString QLoggerWriter::levelToString()
    {
        return levelToString(m_level);
    }

    QString QLoggerWriter::levelToString(const LogLevel &level)
    {
        switch (level) {

        case TraceLevel: return QString("trace");   break;
        case DebugLevel: return QString("debug");   break;
        case InfoLevel : return QString("info");    break;
        case WarnLevel : return QString("warning"); break;
        case ErrorLevel: return QString("error");   break;
        case FatalLevel: return QString("fatal");   break;

        default: return QString("INVALID"); break;
        }
    }

    void QLoggerWriter::setLevel(const LogLevel &level)
    {
        m_level = level;
    }

    QFileInfo QLoggerWriter::fileInfo()
    {
        QFileInfo info(m_filePath);

        return info;
    }

    void QLoggerWriter::setFilePath(const QString &filePath)
    {
        m_filePath = filePath;
    }

    qint64 QLoggerWriter::fileSizeLimit()
    {
        return m_fileSizeLimit;
    }

    void QLoggerWriter::checkFileSizeLimit()
    {
        qint64 fileSizeLimit(this->fileSizeLimit());

        // Pas de limite
        if (fileSizeLimit <= 0) {
            return;
        }

        QFileInfo info(m_filePath);
        QFile file(info.filePath());

        // Le fichier a atteint la taille limite
        if (info.size() >= fileSizeLimit) {

            QString baseName(info.baseName());
            QString suffix(info.completeSuffix());
            QString fileName = baseName + '_' + QDateTime::currentDateTime().toString("ddMMyyyyhhmmss") + '.' + suffix;

            file.rename(info.absoluteDir().absoluteFilePath(fileName));
        }
    }

    void QLoggerWriter::setFileSizeLimit(const qint64 &limit)
    {
        m_fileSizeLimit = limit;
    }

    QString QLoggerWriter::datetimeFormat()
    {
        return m_datetimeFormat;
    }

    void QLoggerWriter::setDatetimeFormat(const QString &format)
    {
        m_datetimeFormat = format;
    }

    void QLoggerWriter::write(const QString &message, const LogLevel &level)
    {
        // Si le niveau minimun de QLoggerWriter est supérieur au niveau de log à enregistrer.
        if (m_level > level) {
            return;
        }

        // Si le fichier n'existe pas
        if (m_filePath.isEmpty()) {
            qDebug() << QObject::tr("QLogger : Le lien du fichier ne peut être vide.");
            return;
        }

        // Test si le fichier n'est pas trop grand
        checkFileSizeLimit();

        QString log(levelToString(level) + " : " + message);

        if (m_saveDatetime) {
            QString datetime(QDateTime::currentDateTime().toString(DATE_FORMAT));
            log = datetime + " : " + log;
        }

        // Affiche les logs dans la console en debug
        #ifdef QT_DEBUG
            qDebug() << log;
        #endif

        QFile file(m_filePath);
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream flux(&file);
            flux.setCodec(QTextCodec::codecForName("UTF-8"));

            flux << log << endl;

            file.close();
        } else {
            qDebug() << tr("QLogger : Impossible d'ouvrir le fichier %1, Erreur : %2.")
                        .arg(m_filePath).arg(file.errorString());
        }
    }

    // MANAGER
    QMutex QLoggerManager::mutex(QMutex::Recursive);
    QSharedPointer<QLoggerManager> QLoggerManager::m_instance(0);

    QLoggerManager& QLoggerManager::getInstance()
    {
        return *getInstancePointer();
    }

    QLoggerManager* QLoggerManager::getInstancePointer()
    {
        if (!m_instance) {
            QMutex mutex;
            QMutexLocker locker(&mutex);  // to unlock mutex on exit

            if (!m_instance) {
                m_instance.reset(new QLoggerManager);
            }
        }

        return m_instance.data();
    }

    void QLoggerManager::add(const QString &filePath, const QString &name, const LogLevel &level)
    {
        QMutexLocker locker(&QLoggerManager::mutex);

        if (m_loggers[name]) {

            qDebug() << QObject::tr("QLoggerManager::add, %1 existe déjà !").arg(name);

        } else {

            QSharedPointer<QLoggerWriter> writer(new QLoggerWriter(filePath, level));
            m_loggers.insert(name, writer);
        }
    }

    void QLoggerManager::remove(const QString &name)
    {
        QMutexLocker locker(&QLoggerManager::mutex);

        m_loggers.remove(name);
    }

    QLoggerWriter* QLoggerManager::logWriter(const QString &name)
    {
        QMutexLocker locker(&QLoggerManager::mutex);

        return m_loggers.value(name).data();
    }

    void QLoggerManager::log(const QString &name, const QString &message, const LogLevel &level)
    {
        privateLog(name, message, level);
        // QtConcurrent::run(QLoggerManager::privateLog, name, message, level);
    }

    void QLoggerManager::setLimitSize(const qint64 &size, const QString &name)
    {
        QMutexLocker locker(&QLoggerManager::mutex);

        // Récupère une instance de QLoggerManager
        QLoggerManager &manager(QLoggerManager::getInstance());

        // Récupère le QLoggerWriter correspondant à name
        QLoggerWriter *logWriter(manager.logWriter(name));

        if (logWriter) {
            logWriter->setFileSizeLimit(size);
        } else {
            qDebug() << QObject::tr("QLogger : %1 n'est pas enregistré dans QLoggerManager.").arg(name);
        }
    }

    QLoggerManager::QLoggerManager()
    {

    }

    void QLoggerManager::privateLog(const QString &name, const QString &message, const LogLevel &level)
    {
        QMutexLocker locker(&QLoggerManager::mutex);

        // Récupère une instance de QLoggerManager
        QLoggerManager &manager(QLoggerManager::getInstance());

        // Récupère le QLoggerWriter correspondant à name
        QLoggerWriter *logWriter(manager.logWriter(name));

        if (logWriter) {
            // Enregistre le message
            logWriter->write(message, level);
        } else {
            qDebug() << QObject::tr("QLogger : %1 n'est pas enregistré dans QLoggerManager.").arg(name);
        }
    }
}
