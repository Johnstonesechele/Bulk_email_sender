#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QColor>
#include <QFont>
#include <QSize>
#include <QPoint>

class AppSettings : public QObject
{
    Q_OBJECT

public:
    static AppSettings* instance();
    
    // General Settings
    bool getAutoSave() const;
    void setAutoSave(bool enabled);
    
    int getAutoSaveInterval() const; // in minutes
    void setAutoSaveInterval(int minutes);
    
    QString getDefaultSenderName() const;
    void setDefaultSenderName(const QString &name);
    
    QString getDefaultSenderEmail() const;
    void setDefaultSenderEmail(const QString &email);
    
    QString getDefaultSignature() const;
    void setDefaultSignature(const QString &signature);
    
    // SMTP Settings
    QString getDefaultSmtpServer() const;
    void setDefaultSmtpServer(const QString &server);
    
    int getDefaultSmtpPort() const;
    void setDefaultSmtpPort(int port);
    
    QString getDefaultEncryption() const;
    void setDefaultEncryption(const QString &encryption);
    
    QString getDefaultUsername() const;
    void setDefaultUsername(const QString &username);
    
    bool getRememberCredentials() const;
    void setRememberCredentials(bool remember);
    
    QString getDefaultPassword() const; // Encrypted
    void setDefaultPassword(const QString &password); // Will be encrypted
    
    // Email Settings
    int getEmailSendDelay() const; // milliseconds between emails
    void setEmailSendDelay(int delay);
    
    int getMaxRetries() const;
    void setMaxRetries(int retries);
    
    int getConnectionTimeout() const; // seconds
    void setConnectionTimeout(int timeout);
    
    bool getConfirmBeforeSending() const;
    void setConfirmBeforeSending(bool confirm);
    
    bool getPreviewEmails() const;
    void setPreviewEmails(bool preview);
    
    // UI Settings
    QString getTheme() const;
    void setTheme(const QString &theme);
    
    QFont getApplicationFont() const;
    void setApplicationFont(const QFont &font);
    
    QSize getWindowSize() const;
    void setWindowSize(const QSize &size);
    
    QPoint getWindowPosition() const;
    void setWindowPosition(const QPoint &position);
    
    bool getWindowMaximized() const;
    void setWindowMaximized(bool maximized);
    
    int getCurrentTab() const;
    void setCurrentTab(int tab);
    
    bool getShowLineNumbers() const;
    void setShowLineNumbers(bool show);
    
    bool getWordWrap() const;
    void setWordWrap(bool wrap);
    
    // Template Settings
    QString getDefaultTemplateCategory() const;
    void setDefaultTemplateCategory(const QString &category);
    
    QStringList getRecentTemplates() const;
    void addRecentTemplate(const QString &templateName);
    
    QString getTemplateDirectory() const;
    void setTemplateDirectory(const QString &directory);
    
    // Import/Export Settings
    QString getLastImportDirectory() const;
    void setLastImportDirectory(const QString &directory);
    
    QString getLastExportDirectory() const;
    void setLastExportDirectory(const QString &directory);
    
    QString getDefaultExportFormat() const;
    void setDefaultExportFormat(const QString &format);
    
    // Analytics Settings
    bool getEnableAnalytics() const;
    void setEnableAnalytics(bool enabled);
    
    int getAnalyticsRetentionDays() const;
    void setAnalyticsRetentionDays(int days);
    
    bool getTrackEmailOpens() const;
    void setTrackEmailOpens(bool track);
    
    bool getTrackLinkClicks() const;
    void setTrackLinkClicks(bool track);
    
    // Logging Settings
    QString getLogLevel() const;
    void setLogLevel(const QString &level);
    
    bool getLogToFile() const;
    void setLogToFile(bool enabled);
    
    bool getLogToConsole() const;
    void setLogToConsole(bool enabled);
    
    int getMaxLogFiles() const;
    void setMaxLogFiles(int maxFiles);
    
    int getMaxLogFileSize() const; // MB
    void setMaxLogFileSize(int sizeMB);
    
    QString getLogDirectory() const;
    void setLogDirectory(const QString &directory);
    
    // Database Settings
    QString getDatabasePath() const;
    void setDatabasePath(const QString &path);
    
    bool getAutoBackupDatabase() const;
    void setAutoBackupDatabase(bool enabled);
    
    int getBackupRetentionDays() const;
    void setBackupRetentionDays(int days);
    
    QString getBackupDirectory() const;
    void setBackupDirectory(const QString &directory);
    
    // Validation Settings
    bool getValidateEmailsOnImport() const;
    void setValidateEmailsOnImport(bool validate);
    
    bool getRemoveDuplicatesOnImport() const;
    void setRemoveDuplicatesOnImport(bool remove);
    
    bool getSkipBlacklistedOnImport() const;
    void setSkipBlacklistedOnImport(bool skip);
    
    // Advanced Settings
    int getMaxConcurrentConnections() const;
    void setMaxConcurrentConnections(int maxConnections);
    
    bool getUseDnsCache() const;
    void setUseDnsCache(bool useCache);
    
    int getDnsCacheTimeout() const; // minutes
    void setDnsCacheTimeout(int timeout);
    
    bool getShowDebugInfo() const;
    void setShowDebugInfo(bool show);
    
    // Backup and Restore
    bool exportSettings(const QString &filePath) const;
    bool importSettings(const QString &filePath);
    void resetToDefaults();
    
    // Validation
    bool validateSettings() const;
    QStringList getValidationErrors() const;

signals:
    void settingsChanged(const QString &key, const QVariant &value);
    void themeChanged(const QString &theme);
    void fontChanged(const QFont &font);

public slots:
    void save();
    void load();

private:
    explicit AppSettings(QObject *parent = nullptr);
    ~AppSettings();
    
    void loadDefaults();
    QString encryptPassword(const QString &password) const;
    QString decryptPassword(const QString &encryptedPassword) const;
    
    static AppSettings *m_instance;
    QSettings *settings;
    
    // Setting keys
    static const QString KEY_AUTO_SAVE;
    static const QString KEY_AUTO_SAVE_INTERVAL;
    static const QString KEY_DEFAULT_SENDER_NAME;
    static const QString KEY_DEFAULT_SENDER_EMAIL;
    static const QString KEY_DEFAULT_SIGNATURE;
    static const QString KEY_DEFAULT_SMTP_SERVER;
    static const QString KEY_DEFAULT_SMTP_PORT;
    static const QString KEY_DEFAULT_ENCRYPTION;
    static const QString KEY_DEFAULT_USERNAME;
    static const QString KEY_REMEMBER_CREDENTIALS;
    static const QString KEY_DEFAULT_PASSWORD;
    static const QString KEY_EMAIL_SEND_DELAY;
    static const QString KEY_MAX_RETRIES;
    static const QString KEY_CONNECTION_TIMEOUT;
    static const QString KEY_CONFIRM_BEFORE_SENDING;
    static const QString KEY_PREVIEW_EMAILS;
    static const QString KEY_THEME;
    static const QString KEY_APPLICATION_FONT;
    static const QString KEY_WINDOW_SIZE;
    static const QString KEY_WINDOW_POSITION;
    static const QString KEY_WINDOW_MAXIMIZED;
    static const QString KEY_CURRENT_TAB;
    static const QString KEY_SHOW_LINE_NUMBERS;
    static const QString KEY_WORD_WRAP;
    static const QString KEY_DEFAULT_TEMPLATE_CATEGORY;
    static const QString KEY_RECENT_TEMPLATES;
    static const QString KEY_TEMPLATE_DIRECTORY;
    static const QString KEY_LAST_IMPORT_DIRECTORY;
    static const QString KEY_LAST_EXPORT_DIRECTORY;
    static const QString KEY_DEFAULT_EXPORT_FORMAT;
    static const QString KEY_ENABLE_ANALYTICS;
    static const QString KEY_ANALYTICS_RETENTION_DAYS;
    static const QString KEY_TRACK_EMAIL_OPENS;
    static const QString KEY_TRACK_LINK_CLICKS;
    static const QString KEY_LOG_LEVEL;
    static const QString KEY_LOG_TO_FILE;
    static const QString KEY_LOG_TO_CONSOLE;
    static const QString KEY_MAX_LOG_FILES;
    static const QString KEY_MAX_LOG_FILE_SIZE;
    static const QString KEY_LOG_DIRECTORY;
    static const QString KEY_DATABASE_PATH;
    static const QString KEY_AUTO_BACKUP_DATABASE;
    static const QString KEY_BACKUP_RETENTION_DAYS;
    static const QString KEY_BACKUP_DIRECTORY;
    static const QString KEY_VALIDATE_EMAILS_ON_IMPORT;
    static const QString KEY_REMOVE_DUPLICATES_ON_IMPORT;
    static const QString KEY_SKIP_BLACKLISTED_ON_IMPORT;
    static const QString KEY_MAX_CONCURRENT_CONNECTIONS;
    static const QString KEY_USE_DNS_CACHE;
    static const QString KEY_DNS_CACHE_TIMEOUT;
    static const QString KEY_SHOW_DEBUG_INFO;
};

#endif // APPSETTINGS_H
