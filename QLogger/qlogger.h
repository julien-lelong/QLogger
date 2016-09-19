#ifndef QLOGGER_H
#define QLOGGER_H

#include <QObject>

#include <QMap>
#include <QMutex>
#include <QFileInfo>
#include <QDateTime>
#include <QSharedPointer>

namespace QLogger
{
    /*!
     * \brief The LogLevel enum,
     *
     * Définit les niveaux de logs supportés
     */
    enum LogLevel { TraceLevel = 0, DebugLevel = 1, InfoLevel = 2, WarnLevel = 3, ErrorLevel = 4, FatalLevel = 5 };

    void trace(const QString &message, const QString &name = "default");
    void debug( const QString &message, const QString &name = "default");
    void info(const QString &message, const QString &name = "default");
    void warning(const QString &message, const QString &name = "default");
    void error(const QString &message, const QString &name = "default");
    void fatal(const QString &message, const QString &name = "default");

    void log(const QString &name, const QString &message, const LogLevel &level);

    void setLimitSize(const qint64 &size, const QString &name = "default");

    /*!
     * \brief The QLoggerWriter class
     *
     * Permet l'écriture d'un message dans un fichier en fonction d'un niveau de log définit par LogLevel
     */
    class QLoggerWriter : public QObject
    {
        Q_OBJECT

        public:
            /*!
             * \brief constructeur QLoggerWriter
             *
             * \param parent
             */
            explicit QLoggerWriter(QObject *parent = 0);

            /*!
             * \brief constructeur QLoggerWriter
             *
             * \param filePath Lien du fichier qui sera utilisé pour logger les messages
             * \param level    niveau de log
             * \param parent
             */
            explicit QLoggerWriter(const QString &filePath, const LogLevel &level = DebugLevel, QObject *parent = 0);

            /*!
             * \brief level
             *
             * Retourne le niveau minimun de log
             *
             * \return LogLevel
             */
            LogLevel level();

            /*!
             * \brief levelToString
             *
             * Retourne un objet QString représentant le niveau de log minimun m_level
             *
             * \return QString
             */
            QString levelToString();

            /*!
             * \brief levelToString
             *
             * Retourne un objet QString représentant le niveau de log level
             *
             * \return QString
             */
            QString levelToString(const LogLevel &level);

            /*!
             * \brief setLevel
             *
             * Modifie le niveau minimun de log
             *
             * \param level nouveau niveau de log
             */
            void setLevel(const LogLevel &level);

            /*!
             * \brief fileInfo
             *
             * Retourne un objet QFileInfo du fichier de log
             *
             * \return QFileInfo
             */
            QFileInfo fileInfo();

            /*!
             * \brief setFilePath
             *
             * Modifie le lien du fichier de logs
             *
             * \param filePath nouveau lien du fichier de log
             */
            void setFilePath(const QString &filePath);

            /*!
             * \brief fileSizeLimit
             * \return
             */
            qint64 fileSizeLimit();

            /*!
             * \brief checkFileSizeLimit
             * Test si la taille du fichier de log n'a pas atteint fileSizeLimit
             * Sinon renomme le fichier avec la date courante
             */
            void checkFileSizeLimit();

            /*!
             * \brief setFileSizeLimit
             * \param limit taille max du fichier
             */
            void setFileSizeLimit(const qint64 &limit);

            /*!
             * \brief saveDatetime
             *
             * Retourne true si la date est enregistrée dans le fichier de log, sinon false.
             *
             * \return bool
             */
            bool saveDatetime();

            /*!
             * \brief setSaveDatetime
             *
             * Indique si la date doit être enregistrée dans le fichier de log
             */
            void setSaveDatetime(const bool &save);

            /*!
             * \brief datetimeFormat
             *
             * Récupère le format utilisé pour écrire la date dans le fichier de log
             *
             * \return QString
             */
            QString datetimeFormat();

            /*!
             * \brief setDatetimeFormat
             *
             * Modifie le format de date enregistré dans le fichier de log
             *
             * \param format
             */
            void setDatetimeFormat(const QString &format);

            /*!
             * \brief write
             *
             * Si le niveau de log est inférieur à m_level, le message sera enregistré dans le fichier de log,
             * sinon le message ne sera pas enregistré.
             *
             * par ex: m_level = DebugLevel, si level = TraceLevel le message ne sera pas enregistré,
             * sinon si level = InfoLevel|WarnLevel|ErrorLevel|FatalLevel le message sera enregistré.
             *
             * \param message message a enregistrer
             * \param level   niveau de log
             */
            void write(const QString &message, const LogLevel &level);

        private:
            QString m_filePath; /*!< Lien du fichier qui sera utilisé pour logger les messages */
            qint64 m_fileSizeLimit; /*< Taille maxi du fichier */

            bool m_saveDatetime; /*!< Indique si la date doit être enregistrée, par default m_saveDatetime = true; */
            QString m_datetimeFormat; /*!< Format de date, par default "yyyy-MM-dd-hh-mm-ss" */

            LogLevel m_level; /*!< niveau de log, par default DebugLevel */
    };

    /*!
     * \brief The QLoggerManager class
     *
     * Singleton permettant la gestion de plusieurs fichiers de logs de maniere thread-safe.
     */
    class QLoggerManager
    {
        public:
            static QMutex mutex; /*!< Utiliser pour rendre QLoggerManager thread safe */

            /*!
             * \brief getInstance
             *
             * Retourne une référence de m_instance
             *
             * \return
             */
            static QLoggerManager& getInstance();

            /*!
             * \brief getInstancePointer
             *
             * Retourne un pointeur de m_instance
             *
             * \return
             */
            static QLoggerManager* getInstancePointer();

            /*!
             * \brief add
             *
             * Ajoute un nouvelle élément QLoggerWriter* dans la QMap m_loggers
             *
             * \param name     : clef de QMap
             * \param filePath : lien du fichier qui sera utilisé pour logger les messages
             * \param level    : niveau de log
             */
            void add(const QString &filePath, const QString &name = "default", const LogLevel &level = DebugLevel);

            /*!
             * \brief remove
             *
             * Supprime un élément de la QMap m_loggers
             *
             * \param name clef correspondante au QLoggerWriter à supprimer
             */
            void remove(const QString &name = "default");

            /*!
             * \brief logWriter
             *
             * Retourne le QLoggerWriter correspondant à la clef name
             *
             * \param name clef permettant de retrouver le QLoggerWriter enregistré dans m_loggers
             * \return QLoggerWriter*
             */
            QLoggerWriter* logWriter(const QString &name = "default");

            /*!
             * \brief log
             *
             * Appel la méthode QLoggerManager::privateLog dans un autre thread avec QtConcurrent::run
             *
             * \param name    clef permettant de retrouver le QLoggerWriter enregistré dans m_loggers
             * \param message message a enregistrer
             * \param level   niveau de log
             */
            static void log(const QString &name, const QString &message, const LogLevel &level);
            static void setLimitSize(const qint64 &size, const QString &name);

        private:
            static QSharedPointer<QLoggerManager> m_instance; /*!< instance de QLoggerManager */

            /*!
             * \brief Constructeur de QLoggerManager
             */
            QLoggerManager();

            QMap<QString, QSharedPointer<QLoggerWriter>> m_loggers; /*!< Liste des QLoggerWritter actifs */

            /*!
             * \brief privateLog
             *
             * Appel la méthode QLoggerWriter::write permettant l'écriture du message dans le fichier log
             *
             * \param name    clef permettant de retrouver le QLoggerWriter enregistré dans m_loggers
             * \param message message a enregistrer
             * \param level   niveau de log
             */
            static void privateLog(const QString &name, const QString &message, const LogLevel &level);
    };
}
#endif // QLOGGER_H
