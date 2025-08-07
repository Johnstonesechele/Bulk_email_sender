#include "DatabaseManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFileInfo>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_databaseReady(false)
{
    m_dataDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    initializeDatabase();
}

DatabaseManager::~DatabaseManager()
{
    saveSettings();
}

bool DatabaseManager::initializeDatabase()
{
    createDirectoryStructure();
    loadSettings();
    
    m_databaseReady = true;
    emit databaseInitialized();
    
    qDebug() << "Database initialized at:" << m_dataDirectory;
    return true;
}

void DatabaseManager::saveSetting(const QString& key, const QVariant& value)
{
    m_settings[key] = value;
    saveSettings();
}

QVariant DatabaseManager::loadSetting(const QString& key, const QVariant& defaultValue)
{
    return m_settings.value(key, defaultValue);
}

QString DatabaseManager::getDataDirectory() const
{
    return m_dataDirectory;
}

QString DatabaseManager::getContactsFile() const
{
    return m_dataDirectory + "/contacts.csv";
}

QString DatabaseManager::getCampaignsFile() const
{
    return m_dataDirectory + "/campaigns.json";
}

QString DatabaseManager::getSettingsFile() const
{
    return m_dataDirectory + "/settings.json";
}

bool DatabaseManager::createBackup(const QString& backupPath)
{
    QDir backupDir(backupPath);
    if (!backupDir.exists()) {
        if (!backupDir.mkpath(backupPath)) {
            emit databaseError("Failed to create backup directory: " + backupPath);
            return false;
        }
    }
    
    // Copy all data files to backup directory
    QStringList filesToBackup = {
        "contacts.csv",
        "campaigns.json",
        "settings.json"
    };
    
    bool success = true;
    for (const QString& fileName : filesToBackup) {
        QString sourceFile = m_dataDirectory + "/" + fileName;
        QString backupFile = backupPath + "/" + fileName;
        
        if (QFile::exists(sourceFile)) {
            if (QFile::exists(backupFile)) {
                QFile::remove(backupFile);
            }
            
            if (!QFile::copy(sourceFile, backupFile)) {
                emit databaseError("Failed to backup file: " + fileName);
                success = false;
            }
        }
    }
    
    if (success) {
        qDebug() << "Backup created successfully at:" << backupPath;
    }
    
    return success;
}

bool DatabaseManager::restoreFromBackup(const QString& backupPath)
{
    QDir backupDir(backupPath);
    if (!backupDir.exists()) {
        emit databaseError("Backup directory does not exist: " + backupPath);
        return false;
    }
    
    // Restore all backup files
    QStringList filesToRestore = {
        "contacts.csv",
        "campaigns.json",
        "settings.json"
    };
    
    bool success = true;
    for (const QString& fileName : filesToRestore) {
        QString backupFile = backupPath + "/" + fileName;
        QString targetFile = m_dataDirectory + "/" + fileName;
        
        if (QFile::exists(backupFile)) {
            if (QFile::exists(targetFile)) {
                QFile::remove(targetFile);
            }
            
            if (!QFile::copy(backupFile, targetFile)) {
                emit databaseError("Failed to restore file: " + fileName);
                success = false;
            }
        }
    }
    
    if (success) {
        loadSettings(); // Reload settings after restore
        qDebug() << "Database restored successfully from:" << backupPath;
    }
    
    return success;
}

bool DatabaseManager::cleanupOldData(int daysToKeep)
{
    // For now, we'll just implement basic cleanup
    // In a real application, you might clean up old campaign data, logs, etc.
    
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-daysToKeep);
    
    // This is a placeholder - implement actual cleanup logic based on your needs
    qDebug() << "Cleanup completed. Kept data newer than:" << cutoffDate.toString();
    
    return true;
}

qint64 DatabaseManager::getDatabaseSize() const
{
    qint64 totalSize = 0;
    
    QDir dataDir(m_dataDirectory);
    QFileInfoList fileList = dataDir.entryInfoList(QDir::Files);
    
    for (const QFileInfo& fileInfo : fileList) {
        totalSize += fileInfo.size();
    }
    
    return totalSize;
}

int DatabaseManager::getCurrentSchemaVersion() const
{
    return loadSetting("schema_version", 0).toInt();
}

bool DatabaseManager::migrateToLatestSchema()
{
    int currentVersion = getCurrentSchemaVersion();
    
    if (currentVersion < CURRENT_SCHEMA_VERSION) {
        if (migrateFromVersion(currentVersion, CURRENT_SCHEMA_VERSION)) {
            saveSetting("schema_version", CURRENT_SCHEMA_VERSION);
            qDebug() << "Schema migrated from version" << currentVersion 
                     << "to" << CURRENT_SCHEMA_VERSION;
            return true;
        } else {
            emit databaseError("Failed to migrate schema");
            return false;
        }
    }
    
    return true; // Already at latest version
}

void DatabaseManager::createDirectoryStructure()
{
    QDir dir;
    if (!dir.exists(m_dataDirectory)) {
        if (!dir.mkpath(m_dataDirectory)) {
            emit databaseError("Failed to create data directory: " + m_dataDirectory);
            return;
        }
    }
    
    // Create subdirectories if needed
    QStringList subdirs = {"backups", "exports", "temp"};
    for (const QString& subdir : subdirs) {
        QString fullPath = m_dataDirectory + "/" + subdir;
        if (!dir.exists(fullPath)) {
            dir.mkpath(fullPath);
        }
    }
}

void DatabaseManager::loadSettings()
{
    QString settingsFile = getSettingsFile();
    
    QFile file(settingsFile);
    if (!file.exists()) {
        // Create default settings
        m_settings["schema_version"] = CURRENT_SCHEMA_VERSION;
        m_settings["smtp_server"] = "smtp.gmail.com";
        m_settings["smtp_port"] = 587;
        m_settings["smtp_username"] = "";
        m_settings["smtp_password"] = "";
        m_settings["sending_delay"] = 5;
        m_settings["auto_backup"] = true;
        m_settings["backup_interval_days"] = 7;
        
        saveSettings();
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit databaseError("Failed to read settings file: " + settingsFile);
        return;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        emit databaseError("Invalid JSON in settings file");
        return;
    }
    
    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        m_settings[it.key()] = it.value().toVariant();
    }
}

void DatabaseManager::saveSettings()
{
    QString settingsFile = getSettingsFile();
    
    QJsonObject obj;
    for (auto it = m_settings.begin(); it != m_settings.end(); ++it) {
        obj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    
    QJsonDocument doc(obj);
    
    QFile file(settingsFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit databaseError("Failed to write settings file: " + settingsFile);
        return;
    }
    
    file.write(doc.toJson());
    file.close();
}

bool DatabaseManager::migrateFromVersion(int fromVersion, int toVersion)
{
    // Implement migration logic here
    // For now, we'll just return true as we're at version 1
    
    if (fromVersion == 0 && toVersion == 1) {
        // Migration from initial version to version 1
        // Add any necessary data transformations here
        return true;
    }
    
    return false;
}

#include "DatabaseManager.moc"