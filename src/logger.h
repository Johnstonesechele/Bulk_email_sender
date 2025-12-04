#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QTimer>
#include <QDir>

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4
};

struct LogEntry {
    QDateTime timestamp;
    LogLevel level;
    QString category;
    QString message;
    QString function;
    QString file;
    int line;
    qint64 threadId;
};

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger* instance();
    
    // Logging methods
    void log(LogLevel level, const QString &category, const QString &message, 
             const QString &function = QString(), const QString &file = QString(), int line = 0);
    
    void debug(const QString &category, const QString &message);
    void info(const QString &category, const QString &message);
    void warning(const QString &category, const QString &message);
    void error(const QString &category, const QString &message);
    void critical(const QString &category, const QString &message);
    
    // Configuration
    void setLogLevel(LogLevel level);
    void setLogToFile(bool enabled);
    void setLogToConsole(bool enabled);
    void setMaxLogFiles(int maxFiles);
    void setMaxLogFileSize(qint64 maxSize);
    void setLogDirectory(const QString &directory);
    
    // File management
    void rotateLogFile();
    void cleanupOldLogs();
    
    // Retrieval
    QVector<LogEntry> getLogEntries(LogLevel minLevel = LogLevel::Debug, 
                                   const QString &category = QString(),
                                   const QDateTime &start = QDateTime(),
                                   const QDateTime &end = QDateTime()) const;
    
    QString formatLogEntry(const LogEntry &entry) const;
    bool exportLogs(const QString &filePath, LogLevel minLevel = LogLevel::Debug, 
                   const QString &category = QString()) const;

signals:
    void logEntryAdded(const LogEntry &entry);

private slots:
    void performLogRotation();

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
    
    void writeToFile(const LogEntry &entry);
    void writeToConsole(const LogEntry &entry);
    void ensureLogDirectory();
    QString getCurrentLogFilePath() const;
    QString logLevelToString(LogLevel level) const;
    LogLevel stringToLogLevel(const QString &levelStr) const;
    
    static Logger *m_instance;
    
    LogLevel m_logLevel;
    bool m_logToFile;
    bool m_logToConsole;
    int m_maxLogFiles;
    qint64 m_maxLogFileSize;
    QString m_logDirectory;
    
    QFile *m_logFile;
    QTextStream *m_logStream;
    QMutex m_mutex;
    QTimer *m_rotationTimer;
    
    mutable QVector<LogEntry> m_memoryLog; // Keep recent entries in memory
    static const int MAX_MEMORY_ENTRIES = 10000;
};

// Convenience macros
#define LOG_DEBUG(category, message) Logger::instance()->debug(category, message)
#define LOG_INFO(category, message) Logger::instance()->info(category, message)
#define LOG_WARNING(category, message) Logger::instance()->warning(category, message)
#define LOG_ERROR(category, message) Logger::instance()->error(category, message)
#define LOG_CRITICAL(category, message) Logger::instance()->critical(category, message)

#define LOG_FUNC_DEBUG(category, message) Logger::instance()->log(LogLevel::Debug, category, message, Q_FUNC_INFO, __FILE__, __LINE__)
#define LOG_FUNC_INFO(category, message) Logger::instance()->log(LogLevel::Info, category, message, Q_FUNC_INFO, __FILE__, __LINE__)
#define LOG_FUNC_WARNING(category, message) Logger::instance()->log(LogLevel::Warning, category, message, Q_FUNC_INFO, __FILE__, __LINE__)
#define LOG_FUNC_ERROR(category, message) Logger::instance()->log(LogLevel::Error, category, message, Q_FUNC_INFO, __FILE__, __LINE__)
#define LOG_FUNC_CRITICAL(category, message) Logger::instance()->log(LogLevel::Critical, category, message, Q_FUNC_INFO, __FILE__, __LINE__)

#endif // LOGGER_H
