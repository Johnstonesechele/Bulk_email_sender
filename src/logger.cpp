#include "logger.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

Logger* Logger::m_instance = nullptr;

Logger* Logger::instance()
{
    if (!m_instance) {
        m_instance = new Logger();
    }
    return m_instance;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_logLevel(LogLevel::Info)
    , m_logToFile(true)
    , m_logToConsole(true)
    , m_maxLogFiles(10)
    , m_maxLogFileSize(10 * 1024 * 1024) // 10MB
    , m_logFile(nullptr)
    , m_logStream(nullptr)
    , m_rotationTimer(new QTimer(this))
{
    // Set default log directory
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "BulkEmailManager";
    }
    
    m_logDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    
    ensureLogDirectory();
    
    // Setup log rotation timer (check every hour)
    m_rotationTimer->setInterval(60 * 60 * 1000); // 1 hour
    connect(m_rotationTimer, &QTimer::timeout, this, &Logger::performLogRotation);
    m_rotationTimer->start();
    
    // Initial setup
    if (m_logToFile) {
        QString logFilePath = getCurrentLogFilePath();
        m_logFile = new QFile(logFilePath, this);
        
        if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            m_logStream = new QTextStream(m_logFile);
            m_logStream->setCodec("UTF-8");
        } else {
            qWarning() << "Failed to open log file:" << logFilePath;
            m_logToFile = false;
        }
    }
    
    // Log startup
    info("Logger", "Logger initialized successfully");
    info("Logger", QString("Log directory: %1").arg(m_logDirectory));
    info("Logger", QString("Max log files: %1, Max file size: %2 bytes").arg(m_maxLogFiles).arg(m_maxLogFileSize));
}

Logger::~Logger()
{
    info("Logger", "Logger shutting down");
    
    if (m_logStream) {
        delete m_logStream;
    }
    
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

void Logger::log(LogLevel level, const QString &category, const QString &message, 
                const QString &function, const QString &file, int line)
{
    if (level < m_logLevel) {
        return; // Below minimum log level
    }
    
    QMutexLocker locker(&m_mutex);
    
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.category = category;
    entry.message = message;
    entry.function = function;
    entry.file = file;
    entry.line = line;
    entry.threadId = reinterpret_cast<qint64>(QThread::currentThreadId());
    
    // Add to memory log
    m_memoryLog.append(entry);
    if (m_memoryLog.size() > MAX_MEMORY_ENTRIES) {
        m_memoryLog.removeFirst();
    }
    
    // Write to outputs
    if (m_logToFile) {
        writeToFile(entry);
    }
    
    if (m_logToConsole) {
        writeToConsole(entry);
    }
    
    emit logEntryAdded(entry);
}

void Logger::debug(const QString &category, const QString &message)
{
    log(LogLevel::Debug, category, message);
}

void Logger::info(const QString &category, const QString &message)
{
    log(LogLevel::Info, category, message);
}

void Logger::warning(const QString &category, const QString &message)
{
    log(LogLevel::Warning, category, message);
}

void Logger::error(const QString &category, const QString &message)
{
    log(LogLevel::Error, category, message);
}

void Logger::critical(const QString &category, const QString &message)
{
    log(LogLevel::Critical, category, message);
}

void Logger::setLogLevel(LogLevel level)
{
    m_logLevel = level;
    info("Logger", QString("Log level set to: %1").arg(logLevelToString(level)));
}

void Logger::setLogToFile(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    
    if (enabled && !m_logToFile) {
        // Enable file logging
        ensureLogDirectory();
        
        QString logFilePath = getCurrentLogFilePath();
        m_logFile = new QFile(logFilePath, this);
        
        if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            m_logStream = new QTextStream(m_logFile);
            m_logStream->setCodec("UTF-8");
            m_logToFile = true;
            info("Logger", "File logging enabled");
        } else {
            qWarning() << "Failed to enable file logging:" << logFilePath;
        }
    } else if (!enabled && m_logToFile) {
        // Disable file logging
        info("Logger", "File logging disabled");
        
        if (m_logStream) {
            delete m_logStream;
            m_logStream = nullptr;
        }
        
        if (m_logFile) {
            m_logFile->close();
            delete m_logFile;
            m_logFile = nullptr;
        }
        
        m_logToFile = false;
    }
}

void Logger::setLogToConsole(bool enabled)
{
    m_logToConsole = enabled;
    info("Logger", QString("Console logging %1").arg(enabled ? "enabled" : "disabled"));
}

void Logger::setMaxLogFiles(int maxFiles)
{
    m_maxLogFiles = maxFiles;
    info("Logger", QString("Max log files set to: %1").arg(maxFiles));
}

void Logger::setMaxLogFileSize(qint64 maxSize)
{
    m_maxLogFileSize = maxSize;
    info("Logger", QString("Max log file size set to: %1 bytes").arg(maxSize));
}

void Logger::setLogDirectory(const QString &directory)
{
    QMutexLocker locker(&m_mutex);
    
    QString oldDirectory = m_logDirectory;
    m_logDirectory = directory;
    
    ensureLogDirectory();
    
    // If file logging is enabled, reopen file in new directory
    if (m_logToFile) {
        if (m_logStream) {
            delete m_logStream;
            m_logStream = nullptr;
        }
        
        if (m_logFile) {
            m_logFile->close();
            delete m_logFile;
            m_logFile = nullptr;
        }
        
        QString logFilePath = getCurrentLogFilePath();
        m_logFile = new QFile(logFilePath, this);
        
        if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            m_logStream = new QTextStream(m_logFile);
            m_logStream->setCodec("UTF-8");
        } else {
            qWarning() << "Failed to open log file in new directory:" << logFilePath;
            m_logToFile = false;
        }
    }
    
    info("Logger", QString("Log directory changed from %1 to %2").arg(oldDirectory, directory));
}

void Logger::rotateLogFile()
{
    if (!m_logToFile || !m_logFile) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Close current file
    if (m_logStream) {
        delete m_logStream;
        m_logStream = nullptr;
    }
    
    if (m_logFile) {
        m_logFile->close();
        QString currentPath = m_logFile->fileName();
        delete m_logFile;
        m_logFile = nullptr;
        
        // Rename current log file with timestamp
        QFileInfo fileInfo(currentPath);
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString rotatedPath = QDir(m_logDirectory).filePath(
            QString("bulk_email_manager_%1.log").arg(timestamp));
        
        QFile::rename(currentPath, rotatedPath);
        info("Logger", QString("Log file rotated to: %1").arg(rotatedPath));
    }
    
    // Open new log file
    QString logFilePath = getCurrentLogFilePath();
    m_logFile = new QFile(logFilePath, this);
    
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_logStream = new QTextStream(m_logFile);
        m_logStream->setCodec("UTF-8");
        info("Logger", QString("New log file created: %1").arg(logFilePath));
    } else {
        qWarning() << "Failed to create new log file:" << logFilePath;
        m_logToFile = false;
    }
    
    // Cleanup old logs
    cleanupOldLogs();
}

void Logger::cleanupOldLogs()
{
    QDir logDir(m_logDirectory);
    if (!logDir.exists()) {
        return;
    }
    
    // Get all log files
    QStringList filters;
    filters << "bulk_email_manager_*.log";
    QFileInfoList logFiles = logDir.entryInfoList(filters, QDir::Files, QDir::Time);
    
    // Remove oldest files if we exceed the maximum
    while (logFiles.size() > m_maxLogFiles) {
        QFileInfo oldestFile = logFiles.takeLast();
        if (QFile::remove(oldestFile.absoluteFilePath())) {
            info("Logger", QString("Removed old log file: %1").arg(oldestFile.fileName()));
        } else {
            warning("Logger", QString("Failed to remove old log file: %1").arg(oldestFile.fileName()));
        }
    }
}

QVector<LogEntry> Logger::getLogEntries(LogLevel minLevel, const QString &category,
                                      const QDateTime &start, const QDateTime &end) const
{
    QMutexLocker locker(&m_mutex);
    
    QVector<LogEntry> filteredEntries;
    
    for (const LogEntry &entry : m_memoryLog) {
        // Filter by level
        if (entry.level < minLevel) {
            continue;
        }
        
        // Filter by category
        if (!category.isEmpty() && entry.category != category) {
            continue;
        }
        
        // Filter by date range
        if (start.isValid() && entry.timestamp < start) {
            continue;
        }
        
        if (end.isValid() && entry.timestamp > end) {
            continue;
        }
        
        filteredEntries.append(entry);
    }
    
    return filteredEntries;
}

QString Logger::formatLogEntry(const LogEntry &entry) const
{
    QString levelStr = logLevelToString(entry.level).toUpper();
    QString timestamp = entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
    
    QString formatted = QString("[%1] [%2] [%3] %4")
                       .arg(timestamp)
                       .arg(levelStr)
                       .arg(entry.category)
                       .arg(entry.message);
    
    if (!entry.function.isEmpty()) {
        formatted += QString(" (%1:%2)").arg(entry.function).arg(entry.line);
    }
    
    return formatted;
}

bool Logger::exportLogs(const QString &filePath, LogLevel minLevel, const QString &category) const
{
    QFile exportFile(filePath);
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&exportFile);
    stream.setCodec("UTF-8");
    
    // Write header
    stream << "=== BULK EMAIL MANAGER LOG EXPORT ===" << "\n";
    stream << "Generated: " << QDateTime::currentDateTime().toString() << "\n";
    stream << "Min Level: " << logLevelToString(minLevel) << "\n";
    if (!category.isEmpty()) {
        stream << "Category Filter: " << category << "\n";
    }
    stream << "======================================" << "\n\n";
    
    // Write log entries
    QVector<LogEntry> entries = getLogEntries(minLevel, category);
    for (const LogEntry &entry : entries) {
        stream << formatLogEntry(entry) << "\n";
    }
    
    return true;
}

void Logger::performLogRotation()
{
    if (!m_logToFile || !m_logFile) {
        return;
    }
    
    // Check if current log file exceeds maximum size
    if (m_logFile->size() > m_maxLogFileSize) {
        info("Logger", QString("Log file size (%1 bytes) exceeds maximum (%2 bytes), rotating...")
             .arg(m_logFile->size()).arg(m_maxLogFileSize));
        rotateLogFile();
    }
}

void Logger::writeToFile(const LogEntry &entry)
{
    if (!m_logStream) {
        return;
    }
    
    QString formatted = formatLogEntry(entry);
    *m_logStream << formatted << "\n";
    m_logStream->flush();
    
    // Check for rotation after writing
    if (m_logFile && m_logFile->size() > m_maxLogFileSize) {
        QTimer::singleShot(0, this, &Logger::performLogRotation);
    }
}

void Logger::writeToConsole(const LogEntry &entry)
{
    QString formatted = formatLogEntry(entry);
    
    // Use different Qt debug functions based on log level
    switch (entry.level) {
        case LogLevel::Debug:
            qDebug().noquote() << formatted;
            break;
        case LogLevel::Info:
            qInfo().noquote() << formatted;
            break;
        case LogLevel::Warning:
            qWarning().noquote() << formatted;
            break;
        case LogLevel::Error:
        case LogLevel::Critical:
            qCritical().noquote() << formatted;
            break;
    }
}

void Logger::ensureLogDirectory()
{
    QDir dir;
    if (!dir.exists(m_logDirectory)) {
        if (dir.mkpath(m_logDirectory)) {
            qDebug() << "Created log directory:" << m_logDirectory;
        } else {
            qWarning() << "Failed to create log directory:" << m_logDirectory;
        }
    }
}

QString Logger::getCurrentLogFilePath() const
{
    return QDir(m_logDirectory).filePath("bulk_email_manager.log");
}

QString Logger::logLevelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

LogLevel Logger::stringToLogLevel(const QString &levelStr) const
{
    QString upper = levelStr.toUpper();
    if (upper == "DEBUG") return LogLevel::Debug;
    if (upper == "INFO") return LogLevel::Info;
    if (upper == "WARNING") return LogLevel::Warning;
    if (upper == "ERROR") return LogLevel::Error;
    if (upper == "CRITICAL") return LogLevel::Critical;
    return LogLevel::Info; // Default
}
