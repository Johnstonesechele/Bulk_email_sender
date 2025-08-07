#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariant>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    // Database initialization
    bool initializeDatabase();
    bool isDatabaseReady() const { return m_databaseReady; }
    
    // Settings management
    void saveSetting(const QString& key, const QVariant& value);
    QVariant loadSetting(const QString& key, const QVariant& defaultValue = QVariant());
    
    // Application data paths
    QString getDataDirectory() const;
    QString getContactsFile() const;
    QString getCampaignsFile() const;
    QString getSettingsFile() const;
    
    // Backup and restore
    bool createBackup(const QString& backupPath);
    bool restoreFromBackup(const QString& backupPath);
    
    // Database maintenance
    bool cleanupOldData(int daysToKeep = 365);
    qint64 getDatabaseSize() const;
    
    // Migration support
    int getCurrentSchemaVersion() const;
    bool migrateToLatestSchema();

signals:
    void databaseError(const QString& error);
    void databaseInitialized();

private:
    void createDirectoryStructure();
    void loadSettings();
    void saveSettings();
    bool migrateFromVersion(int fromVersion, int toVersion);
    
    QString m_dataDirectory;
    QVariantMap m_settings;
    bool m_databaseReady;
    static const int CURRENT_SCHEMA_VERSION = 1;
};

#endif // DATABASEMANAGER_H