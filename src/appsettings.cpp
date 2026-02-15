#include "appsettings.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCryptographicHash>
#include <QDebug>

// Static member definitions
AppSettings* AppSettings::m_instance = nullptr;

// Setting keys
const QString AppSettings::KEY_AUTO_SAVE = "general/autoSave";
const QString AppSettings::KEY_AUTO_SAVE_INTERVAL = "general/autoSaveInterval";
const QString AppSettings::KEY_DEFAULT_SENDER_NAME = "general/defaultSenderName";
const QString AppSettings::KEY_DEFAULT_SENDER_EMAIL = "general/defaultSenderEmail";
const QString AppSettings::KEY_DEFAULT_SIGNATURE = "general/defaultSignature";
const QString AppSettings::KEY_DEFAULT_SMTP_SERVER = "smtp/defaultServer";
const QString AppSettings::KEY_DEFAULT_SMTP_PORT = "smtp/defaultPort";
const QString AppSettings::KEY_DEFAULT_ENCRYPTION = "smtp/defaultEncryption";
const QString AppSettings::KEY_DEFAULT_USERNAME = "smtp/defaultUsername";
const QString AppSettings::KEY_REMEMBER_CREDENTIALS = "smtp/rememberCredentials";
const QString AppSettings::KEY_DEFAULT_PASSWORD = "smtp/defaultPassword";
const QString AppSettings::KEY_EMAIL_SEND_DELAY = "email/sendDelay";
const QString AppSettings::KEY_MAX_RETRIES = "email/maxRetries";
const QString AppSettings::KEY_CONNECTION_TIMEOUT = "email/connectionTimeout";
const QString AppSettings::KEY_CONFIRM_BEFORE_SENDING = "email/confirmBeforeSending";
const QString AppSettings::KEY_PREVIEW_EMAILS = "email/previewEmails";
const QString AppSettings::KEY_THEME = "ui/theme";
const QString AppSettings::KEY_APPLICATION_FONT = "ui/applicationFont";
const QString AppSettings::KEY_WINDOW_SIZE = "ui/windowSize";
const QString AppSettings::KEY_WINDOW_POSITION = "ui/windowPosition";
const QString AppSettings::KEY_WINDOW_MAXIMIZED = "ui/windowMaximized";
const QString AppSettings::KEY_CURRENT_TAB = "ui/currentTab";
const QString AppSettings::KEY_SHOW_LINE_NUMBERS = "ui/showLineNumbers";
const QString AppSettings::KEY_WORD_WRAP = "ui/wordWrap";
const QString AppSettings::KEY_DEFAULT_TEMPLATE_CATEGORY = "templates/defaultCategory";
const QString AppSettings::KEY_RECENT_TEMPLATES = "templates/recentTemplates";
const QString AppSettings::KEY_TEMPLATE_DIRECTORY = "templates/directory";
const QString AppSettings::KEY_LAST_IMPORT_DIRECTORY = "import/lastDirectory";
const QString AppSettings::KEY_LAST_EXPORT_DIRECTORY = "export/lastDirectory";
const QString AppSettings::KEY_DEFAULT_EXPORT_FORMAT = "export/defaultFormat";
const QString AppSettings::KEY_ENABLE_ANALYTICS = "analytics/enabled";
const QString AppSettings::KEY_ANALYTICS_RETENTION_DAYS = "analytics/retentionDays";
const QString AppSettings::KEY_TRACK_EMAIL_OPENS = "analytics/trackEmailOpens";
const QString AppSettings::KEY_TRACK_LINK_CLICKS = "analytics/trackLinkClicks";
const QString AppSettings::KEY_LOG_LEVEL = "logging/level";
const QString AppSettings::KEY_LOG_TO_FILE = "logging/toFile";
const QString AppSettings::KEY_LOG_TO_CONSOLE = "logging/toConsole";
const QString AppSettings::KEY_MAX_LOG_FILES = "logging/maxFiles";
const QString AppSettings::KEY_MAX_LOG_FILE_SIZE = "logging/maxFileSize";
const QString AppSettings::KEY_LOG_DIRECTORY = "logging/directory";
const QString AppSettings::KEY_DATABASE_PATH = "database/path";
const QString AppSettings::KEY_AUTO_BACKUP_DATABASE = "database/autoBackup";
const QString AppSettings::KEY_BACKUP_RETENTION_DAYS = "database/backupRetentionDays";
const QString AppSettings::KEY_BACKUP_DIRECTORY = "database/backupDirectory";
const QString AppSettings::KEY_VALIDATE_EMAILS_ON_IMPORT = "validation/validateEmailsOnImport";
const QString AppSettings::KEY_REMOVE_DUPLICATES_ON_IMPORT = "validation/removeDuplicatesOnImport";
const QString AppSettings::KEY_SKIP_BLACKLISTED_ON_IMPORT = "validation/skipBlacklistedOnImport";
const QString AppSettings::KEY_MAX_CONCURRENT_CONNECTIONS = "advanced/maxConcurrentConnections";
const QString AppSettings::KEY_USE_DNS_CACHE = "advanced/useDnsCache";
const QString AppSettings::KEY_DNS_CACHE_TIMEOUT = "advanced/dnsCacheTimeout";
const QString AppSettings::KEY_SHOW_DEBUG_INFO = "advanced/showDebugInfo";

AppSettings* AppSettings::instance()
{
    if (!m_instance) {
        m_instance = new AppSettings();
    }
    return m_instance;
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "BulkEmailManager";
    }
    
    QString orgName = QCoreApplication::organizationName();
    if (orgName.isEmpty()) {
        orgName = "BulkEmailManager";
    }
    
    settings = new QSettings(orgName, appName, this);
    load();
}

AppSettings::~AppSettings()
{
    save();
}

void AppSettings::loadDefaults()
{
    // General defaults
    settings->setValue(KEY_AUTO_SAVE, true);
    settings->setValue(KEY_AUTO_SAVE_INTERVAL, 5);
    settings->setValue(KEY_DEFAULT_SENDER_NAME, "");
    settings->setValue(KEY_DEFAULT_SENDER_EMAIL, "");
    settings->setValue(KEY_DEFAULT_SIGNATURE, "");
    
    // SMTP defaults
    settings->setValue(KEY_DEFAULT_SMTP_SERVER, "smtp.gmail.com");
    settings->setValue(KEY_DEFAULT_SMTP_PORT, 587);
    settings->setValue(KEY_DEFAULT_ENCRYPTION, "STARTTLS");
    settings->setValue(KEY_DEFAULT_USERNAME, "");
    settings->setValue(KEY_REMEMBER_CREDENTIALS, false);
    settings->setValue(KEY_DEFAULT_PASSWORD, "");
    
    // Email defaults
    settings->setValue(KEY_EMAIL_SEND_DELAY, 1000); // 1 second
    settings->setValue(KEY_MAX_RETRIES, 3);
    settings->setValue(KEY_CONNECTION_TIMEOUT, 30);
    settings->setValue(KEY_CONFIRM_BEFORE_SENDING, true);
    settings->setValue(KEY_PREVIEW_EMAILS, true);
    
    // UI defaults
    settings->setValue(KEY_THEME, "Dark");
    settings->setValue(KEY_APPLICATION_FONT, QFont("Segoe UI", 9));
    settings->setValue(KEY_WINDOW_SIZE, QSize(1200, 800));
    settings->setValue(KEY_WINDOW_POSITION, QPoint(100, 100));
    settings->setValue(KEY_WINDOW_MAXIMIZED, false);
    settings->setValue(KEY_CURRENT_TAB, 0);
    settings->setValue(KEY_SHOW_LINE_NUMBERS, true);
    settings->setValue(KEY_WORD_WRAP, true);
    
    // Template defaults
    settings->setValue(KEY_DEFAULT_TEMPLATE_CATEGORY, "General");
    settings->setValue(KEY_RECENT_TEMPLATES, QStringList());
    settings->setValue(KEY_TEMPLATE_DIRECTORY, 
                      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/templates");
    
    // Import/Export defaults
    settings->setValue(KEY_LAST_IMPORT_DIRECTORY, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    settings->setValue(KEY_LAST_EXPORT_DIRECTORY, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    settings->setValue(KEY_DEFAULT_EXPORT_FORMAT, "CSV");
    
    // Analytics defaults
    settings->setValue(KEY_ENABLE_ANALYTICS, true);
    settings->setValue(KEY_ANALYTICS_RETENTION_DAYS, 365);
    settings->setValue(KEY_TRACK_EMAIL_OPENS, false);
    settings->setValue(KEY_TRACK_LINK_CLICKS, false);
    
    // Logging defaults
    settings->setValue(KEY_LOG_LEVEL, "Info");
    settings->setValue(KEY_LOG_TO_FILE, true);
    settings->setValue(KEY_LOG_TO_CONSOLE, true);
    settings->setValue(KEY_MAX_LOG_FILES, 10);
    settings->setValue(KEY_MAX_LOG_FILE_SIZE, 10); // MB
    settings->setValue(KEY_LOG_DIRECTORY, 
                      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs");
    
    // Database defaults
    settings->setValue(KEY_DATABASE_PATH, 
                      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data");
    settings->setValue(KEY_AUTO_BACKUP_DATABASE, true);
    settings->setValue(KEY_BACKUP_RETENTION_DAYS, 30);
    settings->setValue(KEY_BACKUP_DIRECTORY, 
                      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups");
    
    // Validation defaults
    settings->setValue(KEY_VALIDATE_EMAILS_ON_IMPORT, true);
    settings->setValue(KEY_REMOVE_DUPLICATES_ON_IMPORT, true);
    settings->setValue(KEY_SKIP_BLACKLISTED_ON_IMPORT, true);
    
    // Advanced defaults
    settings->setValue(KEY_MAX_CONCURRENT_CONNECTIONS, 5);
    settings->setValue(KEY_USE_DNS_CACHE, true);
    settings->setValue(KEY_DNS_CACHE_TIMEOUT, 60); // minutes
    settings->setValue(KEY_SHOW_DEBUG_INFO, false);
}

// General Settings
bool AppSettings::getAutoSave() const
{
    return settings->value(KEY_AUTO_SAVE, true).toBool();
}

void AppSettings::setAutoSave(bool enabled)
{
    settings->setValue(KEY_AUTO_SAVE, enabled);
    emit settingsChanged(KEY_AUTO_SAVE, enabled);
}

int AppSettings::getAutoSaveInterval() const
{
    return settings->value(KEY_AUTO_SAVE_INTERVAL, 5).toInt();
}

void AppSettings::setAutoSaveInterval(int minutes)
{
    settings->setValue(KEY_AUTO_SAVE_INTERVAL, minutes);
    emit settingsChanged(KEY_AUTO_SAVE_INTERVAL, minutes);
}

QString AppSettings::getDefaultSenderName() const
{
    return settings->value(KEY_DEFAULT_SENDER_NAME, "").toString();
}

void AppSettings::setDefaultSenderName(const QString &name)
{
    settings->setValue(KEY_DEFAULT_SENDER_NAME, name);
    emit settingsChanged(KEY_DEFAULT_SENDER_NAME, name);
}

QString AppSettings::getDefaultSenderEmail() const
{
    return settings->value(KEY_DEFAULT_SENDER_EMAIL, "").toString();
}

void AppSettings::setDefaultSenderEmail(const QString &email)
{
    settings->setValue(KEY_DEFAULT_SENDER_EMAIL, email);
    emit settingsChanged(KEY_DEFAULT_SENDER_EMAIL, email);
}

QString AppSettings::getDefaultSignature() const
{
    return settings->value(KEY_DEFAULT_SIGNATURE, "").toString();
}

void AppSettings::setDefaultSignature(const QString &signature)
{
    settings->setValue(KEY_DEFAULT_SIGNATURE, signature);
    emit settingsChanged(KEY_DEFAULT_SIGNATURE, signature);
}

// SMTP Settings
QString AppSettings::getDefaultSmtpServer() const
{
    return settings->value(KEY_DEFAULT_SMTP_SERVER, "smtp.gmail.com").toString();
}

void AppSettings::setDefaultSmtpServer(const QString &server)
{
    settings->setValue(KEY_DEFAULT_SMTP_SERVER, server);
    emit settingsChanged(KEY_DEFAULT_SMTP_SERVER, server);
}

int AppSettings::getDefaultSmtpPort() const
{
    return settings->value(KEY_DEFAULT_SMTP_PORT, 587).toInt();
}

void AppSettings::setDefaultSmtpPort(int port)
{
    settings->setValue(KEY_DEFAULT_SMTP_PORT, port);
    emit settingsChanged(KEY_DEFAULT_SMTP_PORT, port);
}

QString AppSettings::getDefaultEncryption() const
{
    return settings->value(KEY_DEFAULT_ENCRYPTION, "STARTTLS").toString();
}

void AppSettings::setDefaultEncryption(const QString &encryption)
{
    settings->setValue(KEY_DEFAULT_ENCRYPTION, encryption);
    emit settingsChanged(KEY_DEFAULT_ENCRYPTION, encryption);
}

QString AppSettings::getDefaultUsername() const
{
    return settings->value(KEY_DEFAULT_USERNAME, "").toString();
}

void AppSettings::setDefaultUsername(const QString &username)
{
    settings->setValue(KEY_DEFAULT_USERNAME, username);
    emit settingsChanged(KEY_DEFAULT_USERNAME, username);
}

bool AppSettings::getRememberCredentials() const
{
    return settings->value(KEY_REMEMBER_CREDENTIALS, false).toBool();
}

void AppSettings::setRememberCredentials(bool remember)
{
    settings->setValue(KEY_REMEMBER_CREDENTIALS, remember);
    emit settingsChanged(KEY_REMEMBER_CREDENTIALS, remember);
}

QString AppSettings::getDefaultPassword() const
{
    QString encrypted = settings->value(KEY_DEFAULT_PASSWORD, "").toString();
    return encrypted.isEmpty() ? "" : decryptPassword(encrypted);
}

void AppSettings::setDefaultPassword(const QString &password)
{
    QString encrypted = password.isEmpty() ? "" : encryptPassword(password);
    settings->setValue(KEY_DEFAULT_PASSWORD, encrypted);
    emit settingsChanged(KEY_DEFAULT_PASSWORD, "[ENCRYPTED]");
}

// Email Settings
int AppSettings::getEmailSendDelay() const
{
    return settings->value(KEY_EMAIL_SEND_DELAY, 1000).toInt();
}

void AppSettings::setEmailSendDelay(int delay)
{
    settings->setValue(KEY_EMAIL_SEND_DELAY, delay);
    emit settingsChanged(KEY_EMAIL_SEND_DELAY, delay);
}

int AppSettings::getMaxRetries() const
{
    return settings->value(KEY_MAX_RETRIES, 3).toInt();
}

void AppSettings::setMaxRetries(int retries)
{
    settings->setValue(KEY_MAX_RETRIES, retries);
    emit settingsChanged(KEY_MAX_RETRIES, retries);
}

int AppSettings::getConnectionTimeout() const
{
    return settings->value(KEY_CONNECTION_TIMEOUT, 30).toInt();
}

void AppSettings::setConnectionTimeout(int timeout)
{
    settings->setValue(KEY_CONNECTION_TIMEOUT, timeout);
    emit settingsChanged(KEY_CONNECTION_TIMEOUT, timeout);
}

bool AppSettings::getConfirmBeforeSending() const
{
    return settings->value(KEY_CONFIRM_BEFORE_SENDING, true).toBool();
}

void AppSettings::setConfirmBeforeSending(bool confirm)
{
    settings->setValue(KEY_CONFIRM_BEFORE_SENDING, confirm);
    emit settingsChanged(KEY_CONFIRM_BEFORE_SENDING, confirm);
}

bool AppSettings::getPreviewEmails() const
{
    return settings->value(KEY_PREVIEW_EMAILS, true).toBool();
}

void AppSettings::setPreviewEmails(bool preview)
{
    settings->setValue(KEY_PREVIEW_EMAILS, preview);
    emit settingsChanged(KEY_PREVIEW_EMAILS, preview);
}

// UI Settings
QString AppSettings::getTheme() const
{
    return settings->value(KEY_THEME, "Dark").toString();
}

void AppSettings::setTheme(const QString &theme)
{
    settings->setValue(KEY_THEME, theme);
    emit themeChanged(theme);
    emit settingsChanged(KEY_THEME, theme);
}

QFont AppSettings::getApplicationFont() const
{
    return settings->value(KEY_APPLICATION_FONT, QFont("Segoe UI", 9)).value<QFont>();
}

void AppSettings::setApplicationFont(const QFont &font)
{
    settings->setValue(KEY_APPLICATION_FONT, font);
    emit fontChanged(font);
    emit settingsChanged(KEY_APPLICATION_FONT, font);
}

QSize AppSettings::getWindowSize() const
{
    return settings->value(KEY_WINDOW_SIZE, QSize(1200, 800)).toSize();
}

void AppSettings::setWindowSize(const QSize &size)
{
    settings->setValue(KEY_WINDOW_SIZE, size);
    emit settingsChanged(KEY_WINDOW_SIZE, size);
}

QPoint AppSettings::getWindowPosition() const
{
    return settings->value(KEY_WINDOW_POSITION, QPoint(100, 100)).toPoint();
}

void AppSettings::setWindowPosition(const QPoint &position)
{
    settings->setValue(KEY_WINDOW_POSITION, position);
    emit settingsChanged(KEY_WINDOW_POSITION, position);
}

bool AppSettings::getWindowMaximized() const
{
    return settings->value(KEY_WINDOW_MAXIMIZED, false).toBool();
}

void AppSettings::setWindowMaximized(bool maximized)
{
    settings->setValue(KEY_WINDOW_MAXIMIZED, maximized);
    emit settingsChanged(KEY_WINDOW_MAXIMIZED, maximized);
}

int AppSettings::getCurrentTab() const
{
    return settings->value(KEY_CURRENT_TAB, 0).toInt();
}

void AppSettings::setCurrentTab(int tab)
{
    settings->setValue(KEY_CURRENT_TAB, tab);
    emit settingsChanged(KEY_CURRENT_TAB, tab);
}

bool AppSettings::getShowLineNumbers() const
{
    return settings->value(KEY_SHOW_LINE_NUMBERS, true).toBool();
}

void AppSettings::setShowLineNumbers(bool show)
{
    settings->setValue(KEY_SHOW_LINE_NUMBERS, show);
    emit settingsChanged(KEY_SHOW_LINE_NUMBERS, show);
}

bool AppSettings::getWordWrap() const
{
    return settings->value(KEY_WORD_WRAP, true).toBool();
}

void AppSettings::setWordWrap(bool wrap)
{
    settings->setValue(KEY_WORD_WRAP, wrap);
    emit settingsChanged(KEY_WORD_WRAP, wrap);
}

// Template Settings
QString AppSettings::getDefaultTemplateCategory() const
{
    return settings->value(KEY_DEFAULT_TEMPLATE_CATEGORY, "General").toString();
}

void AppSettings::setDefaultTemplateCategory(const QString &category)
{
    settings->setValue(KEY_DEFAULT_TEMPLATE_CATEGORY, category);
    emit settingsChanged(KEY_DEFAULT_TEMPLATE_CATEGORY, category);
}

QStringList AppSettings::getRecentTemplates() const
{
    return settings->value(KEY_RECENT_TEMPLATES, QStringList()).toStringList();
}

void AppSettings::addRecentTemplate(const QString &templateName)
{
    QStringList recent = getRecentTemplates();
    recent.removeAll(templateName); // Remove if already exists
    recent.prepend(templateName);
    
    // Keep only last 10 templates
    while (recent.size() > 10) {
        recent.removeLast();
    }
    
    settings->setValue(KEY_RECENT_TEMPLATES, recent);
    emit settingsChanged(KEY_RECENT_TEMPLATES, recent);
}

QString AppSettings::getTemplateDirectory() const
{
    return settings->value(KEY_TEMPLATE_DIRECTORY, 
                          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/templates").toString();
}

void AppSettings::setTemplateDirectory(const QString &directory)
{
    settings->setValue(KEY_TEMPLATE_DIRECTORY, directory);
    emit settingsChanged(KEY_TEMPLATE_DIRECTORY, directory);
}

// Analytics Settings
bool AppSettings::getEnableAnalytics() const
{
    return settings->value(KEY_ENABLE_ANALYTICS, true).toBool();
}

void AppSettings::setEnableAnalytics(bool enabled)
{
    settings->setValue(KEY_ENABLE_ANALYTICS, enabled);
    emit settingsChanged(KEY_ENABLE_ANALYTICS, enabled);
}

int AppSettings::getAnalyticsRetentionDays() const
{
    return settings->value(KEY_ANALYTICS_RETENTION_DAYS, 365).toInt();
}

void AppSettings::setAnalyticsRetentionDays(int days)
{
    settings->setValue(KEY_ANALYTICS_RETENTION_DAYS, days);
    emit settingsChanged(KEY_ANALYTICS_RETENTION_DAYS, days);
}

// Logging Settings
QString AppSettings::getLogLevel() const
{
    return settings->value(KEY_LOG_LEVEL, "Info").toString();
}

void AppSettings::setLogLevel(const QString &level)
{
    settings->setValue(KEY_LOG_LEVEL, level);
    emit settingsChanged(KEY_LOG_LEVEL, level);
}

bool AppSettings::getLogToFile() const
{
    return settings->value(KEY_LOG_TO_FILE, true).toBool();
}

void AppSettings::setLogToFile(bool enabled)
{
    settings->setValue(KEY_LOG_TO_FILE, enabled);
    emit settingsChanged(KEY_LOG_TO_FILE, enabled);
}

bool AppSettings::getLogToConsole() const
{
    return settings->value(KEY_LOG_TO_CONSOLE, true).toBool();
}

void AppSettings::setLogToConsole(bool enabled)
{
    settings->setValue(KEY_LOG_TO_CONSOLE, enabled);
    emit settingsChanged(KEY_LOG_TO_CONSOLE, enabled);
}

int AppSettings::getMaxLogFiles() const
{
    return settings->value(KEY_MAX_LOG_FILES, 10).toInt();
}

void AppSettings::setMaxLogFiles(int maxFiles)
{
    settings->setValue(KEY_MAX_LOG_FILES, maxFiles);
    emit settingsChanged(KEY_MAX_LOG_FILES, maxFiles);
}

int AppSettings::getMaxLogFileSize() const
{
    return settings->value(KEY_MAX_LOG_FILE_SIZE, 10).toInt();
}

void AppSettings::setMaxLogFileSize(int sizeMB)
{
    settings->setValue(KEY_MAX_LOG_FILE_SIZE, sizeMB);
    emit settingsChanged(KEY_MAX_LOG_FILE_SIZE, sizeMB);
}

QString AppSettings::getLogDirectory() const
{
    return settings->value(KEY_LOG_DIRECTORY, 
                          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs").toString();
}

void AppSettings::setLogDirectory(const QString &directory)
{
    settings->setValue(KEY_LOG_DIRECTORY, directory);
    emit settingsChanged(KEY_LOG_DIRECTORY, directory);
}

// Database Settings
QString AppSettings::getDatabasePath() const
{
    return settings->value(KEY_DATABASE_PATH, 
                          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data").toString();
}

void AppSettings::setDatabasePath(const QString &path)
{
    settings->setValue(KEY_DATABASE_PATH, path);
    emit settingsChanged(KEY_DATABASE_PATH, path);
}

bool AppSettings::getAutoBackupDatabase() const
{
    return settings->value(KEY_AUTO_BACKUP_DATABASE, true).toBool();
}

void AppSettings::setAutoBackupDatabase(bool enabled)
{
    settings->setValue(KEY_AUTO_BACKUP_DATABASE, enabled);
    emit settingsChanged(KEY_AUTO_BACKUP_DATABASE, enabled);
}

int AppSettings::getBackupRetentionDays() const
{
    return settings->value(KEY_BACKUP_RETENTION_DAYS, 30).toInt();
}

void AppSettings::setBackupRetentionDays(int days)
{
    settings->setValue(KEY_BACKUP_RETENTION_DAYS, days);
    emit settingsChanged(KEY_BACKUP_RETENTION_DAYS, days);
}

QString AppSettings::getBackupDirectory() const
{
    return settings->value(KEY_BACKUP_DIRECTORY, 
                          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups").toString();
}

void AppSettings::setBackupDirectory(const QString &directory)
{
    settings->setValue(KEY_BACKUP_DIRECTORY, directory);
    emit settingsChanged(KEY_BACKUP_DIRECTORY, directory);
}

// Validation Settings
bool AppSettings::getValidateEmailsOnImport() const
{
    return settings->value(KEY_VALIDATE_EMAILS_ON_IMPORT, true).toBool();
}

void AppSettings::setValidateEmailsOnImport(bool validate)
{
    settings->setValue(KEY_VALIDATE_EMAILS_ON_IMPORT, validate);
    emit settingsChanged(KEY_VALIDATE_EMAILS_ON_IMPORT, validate);
}

bool AppSettings::getRemoveDuplicatesOnImport() const
{
    return settings->value(KEY_REMOVE_DUPLICATES_ON_IMPORT, true).toBool();
}

void AppSettings::setRemoveDuplicatesOnImport(bool remove)
{
    settings->setValue(KEY_REMOVE_DUPLICATES_ON_IMPORT, remove);
    emit settingsChanged(KEY_REMOVE_DUPLICATES_ON_IMPORT, remove);
}

bool AppSettings::getSkipBlacklistedOnImport() const
{
    return settings->value(KEY_SKIP_BLACKLISTED_ON_IMPORT, true).toBool();
}

void AppSettings::setSkipBlacklistedOnImport(bool skip)
{
    settings->setValue(KEY_SKIP_BLACKLISTED_ON_IMPORT, skip);
    emit settingsChanged(KEY_SKIP_BLACKLISTED_ON_IMPORT, skip);
}

// Advanced Settings
int AppSettings::getMaxConcurrentConnections() const
{
    return settings->value(KEY_MAX_CONCURRENT_CONNECTIONS, 5).toInt();
}

void AppSettings::setMaxConcurrentConnections(int maxConnections)
{
    settings->setValue(KEY_MAX_CONCURRENT_CONNECTIONS, maxConnections);
    emit settingsChanged(KEY_MAX_CONCURRENT_CONNECTIONS, maxConnections);
}

bool AppSettings::getUseDnsCache() const
{
    return settings->value(KEY_USE_DNS_CACHE, true).toBool();
}

void AppSettings::setUseDnsCache(bool useCache)
{
    settings->setValue(KEY_USE_DNS_CACHE, useCache);
    emit settingsChanged(KEY_USE_DNS_CACHE, useCache);
}

int AppSettings::getDnsCacheTimeout() const
{
    return settings->value(KEY_DNS_CACHE_TIMEOUT, 60).toInt();
}

void AppSettings::setDnsCacheTimeout(int timeout)
{
    settings->setValue(KEY_DNS_CACHE_TIMEOUT, timeout);
    emit settingsChanged(KEY_DNS_CACHE_TIMEOUT, timeout);
}

bool AppSettings::getShowDebugInfo() const
{
    return settings->value(KEY_SHOW_DEBUG_INFO, false).toBool();
}

void AppSettings::setShowDebugInfo(bool show)
{
    settings->setValue(KEY_SHOW_DEBUG_INFO, show);
    emit settingsChanged(KEY_SHOW_DEBUG_INFO, show);
}

// Backup and Restore
bool AppSettings::exportSettings(const QString &filePath) const
{
    QJsonObject jsonSettings;
    
    // Export all settings
    settings->sync();
    QStringList keys = settings->allKeys();
    
    for (const QString &key : keys) {
        QVariant value = settings->value(key);
        
        // Handle different data types
        if (value.type() == QVariant::String) {
            jsonSettings[key] = value.toString();
        } else if (value.type() == QVariant::Int) {
            jsonSettings[key] = value.toInt();
        } else if (value.type() == QVariant::Bool) {
            jsonSettings[key] = value.toBool();
        } else if (value.type() == QVariant::StringList) {
            QJsonArray array;
            for (const QString &str : value.toStringList()) {
                array.append(str);
            }
            jsonSettings[key] = array;
        }
        // Add more types as needed
    }
    
    QJsonDocument doc(jsonSettings);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

bool AppSettings::importSettings(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        return false;
    }
    
    QJsonObject jsonSettings = doc.object();
    
    // Import all settings
    for (auto it = jsonSettings.constBegin(); it != jsonSettings.constEnd(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();
        
        if (value.isString()) {
            settings->setValue(key, value.toString());
        } else if (value.isDouble()) {
            settings->setValue(key, value.toInt());
        } else if (value.isBool()) {
            settings->setValue(key, value.toBool());
        } else if (value.isArray()) {
            QStringList list;
            for (const QJsonValue &v : value.toArray()) {
                list.append(v.toString());
            }
            settings->setValue(key, list);
        }
    }
    
    settings->sync();
    emit settingsChanged("ALL", QVariant());
    
    return true;
}

void AppSettings::resetToDefaults()
{
    settings->clear();
    loadDefaults();
    settings->sync();
    emit settingsChanged("ALL", QVariant());
}

bool AppSettings::validateSettings() const
{
    // Basic validation of settings
    QStringList errors = getValidationErrors();
    return errors.isEmpty();
}

QStringList AppSettings::getValidationErrors() const
{
    QStringList errors;
    
    // Validate email settings
    if (getEmailSendDelay() < 0) {
        errors << "Email send delay must be non-negative";
    }
    
    if (getMaxRetries() < 0) {
        errors << "Max retries must be non-negative";
    }
    
    if (getConnectionTimeout() < 1) {
        errors << "Connection timeout must be at least 1 second";
    }
    
    // Validate analytics settings
    if (getAnalyticsRetentionDays() < 1) {
        errors << "Analytics retention days must be at least 1";
    }
    
    // Validate logging settings
    if (getMaxLogFiles() < 1) {
        errors << "Max log files must be at least 1";
    }
    
    if (getMaxLogFileSize() < 1) {
        errors << "Max log file size must be at least 1 MB";
    }
    
    return errors;
}

void AppSettings::save()
{
    settings->sync();
}

void AppSettings::load()
{
    // Load defaults if this is the first run
    if (settings->allKeys().isEmpty()) {
        loadDefaults();
    }
}

QString AppSettings::encryptPassword(const QString &password) const
{
    // Simple XOR encryption (for basic obfuscation)
    // In a production environment, use proper encryption
    QByteArray data = password.toUtf8();
    QByteArray key = "BulkEmailManagerSecretKey2024";
    
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ key[i % key.size()];
    }
    
    return data.toBase64();
}

QString AppSettings::decryptPassword(const QString &encryptedPassword) const
{
    QByteArray data = QByteArray::fromBase64(encryptedPassword.toUtf8());
    QByteArray key = "BulkEmailManagerSecretKey2024";
    
    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ key[i % key.size()];
    }
    
    return QString::fromUtf8(data);
}
