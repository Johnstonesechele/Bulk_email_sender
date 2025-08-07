#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QVariant>

Database::Database(QObject *parent)
    : QObject(parent)
{
    initializeDatabase();
}

Database::~Database()
{
    if (QSqlDatabase::contains("bulk_email_db")) {
        QSqlDatabase::removeDatabase("bulk_email_db");
    }
}

bool Database::initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "bulk_email_db");
    
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    db.setDatabaseName(dbPath + "/bulk_email.db");
    
    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return false;
    }
    
    // Create tables
    QSqlQuery query(db);
    
    // Emails table
    if (!query.exec("CREATE TABLE IF NOT EXISTS emails ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "email TEXT NOT NULL,"
                   "name TEXT,"
                   "status TEXT DEFAULT 'Pending',"
                   "sent_date DATETIME,"
                   "received_date DATETIME,"
                   "campaign_id TEXT,"
                   "error_message TEXT,"
                   "is_valid BOOLEAN DEFAULT 0,"
                   "created_date DATETIME DEFAULT CURRENT_TIMESTAMP"
                   ")")) {
        qDebug() << "Failed to create emails table:" << query.lastError().text();
        return false;
    }
    
    // Campaigns table
    if (!query.exec("CREATE TABLE IF NOT EXISTS campaigns ("
                   "id TEXT PRIMARY KEY,"
                   "name TEXT NOT NULL,"
                   "subject TEXT NOT NULL,"
                   "sent_date DATETIME NOT NULL,"
                   "total_emails INTEGER DEFAULT 0,"
                   "sent_emails INTEGER DEFAULT 0,"
                   "received_emails INTEGER DEFAULT 0,"
                   "failed_emails INTEGER DEFAULT 0,"
                   "status TEXT DEFAULT 'Pending',"
                   "created_date DATETIME DEFAULT CURRENT_TIMESTAMP"
                   ")")) {
        qDebug() << "Failed to create campaigns table:" << query.lastError().text();
        return false;
    }
    
    // Settings table
    if (!query.exec("CREATE TABLE IF NOT EXISTS settings ("
                   "key TEXT PRIMARY KEY,"
                   "value TEXT NOT NULL,"
                   "updated_date DATETIME DEFAULT CURRENT_TIMESTAMP"
                   ")")) {
        qDebug() << "Failed to create settings table:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::saveEmail(const EmailData &email)
{
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    query.prepare("INSERT OR REPLACE INTO emails "
                  "(email, name, status, sent_date, received_date, campaign_id, error_message, is_valid) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(email.email);
    query.addBindValue(email.name);
    query.addBindValue(email.status);
    query.addBindValue(email.sentDate.isValid() ? email.sentDate.toString(Qt::ISODate) : QVariant());
    query.addBindValue(email.receivedDate.isValid() ? email.receivedDate.toString(Qt::ISODate) : QVariant());
    query.addBindValue(email.campaignId);
    query.addBindValue(email.errorMessage);
    query.addBindValue(email.isValid);
    
    if (!query.exec()) {
        qDebug() << "Failed to save email:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::updateEmail(const EmailData &email)
{
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    query.prepare("UPDATE emails SET "
                  "name = ?, status = ?, sent_date = ?, received_date = ?, "
                  "campaign_id = ?, error_message = ?, is_valid = ? "
                  "WHERE email = ?");
    
    query.addBindValue(email.name);
    query.addBindValue(email.status);
    query.addBindValue(email.sentDate.isValid() ? email.sentDate.toString(Qt::ISODate) : QVariant());
    query.addBindValue(email.receivedDate.isValid() ? email.receivedDate.toString(Qt::ISODate) : QVariant());
    query.addBindValue(email.campaignId);
    query.addBindValue(email.errorMessage);
    query.addBindValue(email.isValid);
    query.addBindValue(email.email);
    
    if (!query.exec()) {
        qDebug() << "Failed to update email:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QList<EmailData> Database::getEmails(const QString &status)
{
    QList<EmailData> emails;
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    QString sql = "SELECT email, name, status, sent_date, received_date, campaign_id, error_message, is_valid "
                  "FROM emails";
    
    if (!status.isEmpty()) {
        sql += " WHERE status = ?";
        query.prepare(sql);
        query.addBindValue(status);
    } else {
        query.prepare(sql);
    }
    
    if (!query.exec()) {
        qDebug() << "Failed to get emails:" << query.lastError().text();
        return emails;
    }
    
    while (query.next()) {
        EmailData email;
        email.email = query.value("email").toString();
        email.name = query.value("name").toString();
        email.status = query.value("status").toString();
        email.sentDate = QDateTime::fromString(query.value("sent_date").toString(), Qt::ISODate);
        email.receivedDate = QDateTime::fromString(query.value("received_date").toString(), Qt::ISODate);
        email.campaignId = query.value("campaign_id").toString();
        email.errorMessage = query.value("error_message").toString();
        email.isValid = query.value("is_valid").toBool();
        
        emails.append(email);
    }
    
    return emails;
}

bool Database::saveCampaign(const CampaignData &campaign)
{
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    query.prepare("INSERT OR REPLACE INTO campaigns "
                  "(id, name, subject, sent_date, total_emails, sent_emails, received_emails, failed_emails, status) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(campaign.id);
    query.addBindValue(campaign.name);
    query.addBindValue(campaign.subject);
    query.addBindValue(campaign.sentDate.toString(Qt::ISODate));
    query.addBindValue(campaign.totalEmails);
    query.addBindValue(campaign.sentEmails);
    query.addBindValue(campaign.receivedEmails);
    query.addBindValue(campaign.failedEmails);
    query.addBindValue(campaign.status);
    
    if (!query.exec()) {
        qDebug() << "Failed to save campaign:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QList<CampaignData> Database::getCampaigns(const QDateTime &startDate, const QDateTime &endDate)
{
    QList<CampaignData> campaigns;
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    QString sql = "SELECT id, name, subject, sent_date, total_emails, sent_emails, received_emails, failed_emails, status "
                  "FROM campaigns";
    
    if (startDate.isValid() && endDate.isValid()) {
        sql += " WHERE sent_date BETWEEN ? AND ?";
        query.prepare(sql);
        query.addBindValue(startDate.toString(Qt::ISODate));
        query.addBindValue(endDate.toString(Qt::ISODate));
    } else {
        query.prepare(sql);
    }
    
    if (!query.exec()) {
        qDebug() << "Failed to get campaigns:" << query.lastError().text();
        return campaigns;
    }
    
    while (query.next()) {
        CampaignData campaign;
        campaign.id = query.value("id").toString();
        campaign.name = query.value("name").toString();
        campaign.subject = query.value("subject").toString();
        campaign.sentDate = QDateTime::fromString(query.value("sent_date").toString(), Qt::ISODate);
        campaign.totalEmails = query.value("total_emails").toInt();
        campaign.sentEmails = query.value("sent_emails").toInt();
        campaign.receivedEmails = query.value("received_emails").toInt();
        campaign.failedEmails = query.value("failed_emails").toInt();
        campaign.status = query.value("status").toString();
        
        campaigns.append(campaign);
    }
    
    return campaigns;
}

bool Database::saveSetting(const QString &key, const QString &value)
{
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)");
    query.addBindValue(key);
    query.addBindValue(value);
    
    if (!query.exec()) {
        qDebug() << "Failed to save setting:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QString Database::getSetting(const QString &key, const QString &defaultValue)
{
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    query.prepare("SELECT value FROM settings WHERE key = ?");
    query.addBindValue(key);
    
    if (!query.exec() || !query.next()) {
        return defaultValue;
    }
    
    return query.value("value").toString();
}

bool Database::deleteEmails(const QStringList &emails)
{
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    QString placeholders = QString("?").repeated(emails.size());
    query.prepare(QString("DELETE FROM emails WHERE email IN (%1)").arg(placeholders));
    
    for (const QString &email : emails) {
        query.addBindValue(email);
    }
    
    if (!query.exec()) {
        qDebug() << "Failed to delete emails:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::cleanInvalidEmails()
{
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    if (!query.exec("DELETE FROM emails WHERE is_valid = 0 OR status = 'Invalid'")) {
        qDebug() << "Failed to clean invalid emails:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QMap<QString, int> Database::getEmailStats()
{
    QMap<QString, int> stats;
    QSqlDatabase db = QSqlDatabase::database("bulk_email_db");
    QSqlQuery query(db);
    
    // Total emails
    if (query.exec("SELECT COUNT(*) as total FROM emails")) {
        if (query.next()) {
            stats["total"] = query.value("total").toInt();
        }
    }
    
    // Valid emails
    if (query.exec("SELECT COUNT(*) as valid FROM emails WHERE is_valid = 1")) {
        if (query.next()) {
            stats["valid"] = query.value("valid").toInt();
        }
    }
    
    // Sent emails
    if (query.exec("SELECT COUNT(*) as sent FROM emails WHERE status = 'Sent' OR status = 'Received'")) {
        if (query.next()) {
            stats["sent"] = query.value("sent").toInt();
        }
    }
    
    // Received emails
    if (query.exec("SELECT COUNT(*) as received FROM emails WHERE status = 'Received'")) {
        if (query.next()) {
            stats["received"] = query.value("received").toInt();
        }
    }
    
    // Failed emails
    if (query.exec("SELECT COUNT(*) as failed FROM emails WHERE status = 'Failed'")) {
        if (query.next()) {
            stats["failed"] = query.value("failed").toInt();
        }
    }
    
    return stats;
}