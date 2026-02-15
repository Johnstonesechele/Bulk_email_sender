#include "database.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStandardPaths>

Database::Database(QObject *parent)
    : QObject(parent)
    , connected(false)
{
}

Database::~Database()
{
    close();
}

bool Database::initialize()
{
    // Get application data directory
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    
    // Create database file
    QString dbPath = dataPath + "/bulk_email_manager.db";
    
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        setLastError(db.lastError());
        return false;
    }
    
    connected = true;
    
    // Create tables
    if (!createTables()) {
        return false;
    }
    
    return true;
}

bool Database::createTables()
{
    // Create campaigns table
    QString createCampaignsTable = R"(
        CREATE TABLE IF NOT EXISTS campaigns (
            id TEXT PRIMARY KEY,
            subject TEXT NOT NULL,
            body TEXT NOT NULL,
            sender_email TEXT NOT NULL,
            sender_name TEXT NOT NULL,
            status TEXT DEFAULT 'Draft',
            created_date TEXT NOT NULL,
            sent_date TEXT,
            sent_count INTEGER DEFAULT 0,
            failed_count INTEGER DEFAULT 0
        )
    )";
    
    if (!executeQuery(createCampaignsTable)) {
        return false;
    }
    
    // Create emails table
    QString createEmailsTable = R"(
        CREATE TABLE IF NOT EXISTS emails (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT NOT NULL,
            name TEXT,
            campaign_id TEXT NOT NULL,
            status TEXT DEFAULT 'Pending',
            sent_date TEXT,
            error_message TEXT,
            FOREIGN KEY (campaign_id) REFERENCES campaigns (id) ON DELETE CASCADE
        )
    )";
    
    if (!executeQuery(createEmailsTable)) {
        return false;
    }
    
    // Create indexes
    QString createEmailIndex = "CREATE INDEX IF NOT EXISTS idx_emails_campaign_id ON emails (campaign_id)";
    QString createEmailStatusIndex = "CREATE INDEX IF NOT EXISTS idx_emails_status ON emails (status)";
    QString createEmailAddressIndex = "CREATE INDEX IF NOT EXISTS idx_emails_address ON emails (email)";
    
    executeQuery(createEmailIndex);
    executeQuery(createEmailStatusIndex);
    executeQuery(createEmailAddressIndex);
    
    return true;
}

bool Database::isConnected() const
{
    return connected;
}

bool Database::saveCampaign(const EmailCampaign &campaign)
{
    QString query = R"(
        INSERT OR REPLACE INTO campaigns 
        (id, subject, body, sender_email, sender_name, status, created_date, sent_date, sent_count, failed_count)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    QVariantList params;
    params << campaign.getId() << campaign.getSubject() << campaign.getBody()
           << campaign.getSenderEmail() << campaign.getSenderName() << campaign.getStatus()
           << campaign.getCreatedDate().toString(Qt::ISODate)
           << (campaign.getSentDate().isValid() ? campaign.getSentDate().toString(Qt::ISODate) : QString())
           << campaign.getSentCount() << campaign.getFailedCount();
    
    if (executePreparedQuery(query, params)) {
        emit campaignSaved(campaign.getId());
        return true;
    }
    return false;
}

bool Database::updateCampaign(const EmailCampaign &campaign)
{
    return saveCampaign(campaign);
}

bool Database::deleteCampaign(const QString &campaignId)
{
    QString query = "DELETE FROM campaigns WHERE id = ?";
    QVariantList params;
    params << campaignId;
    
    if (executePreparedQuery(query, params)) {
        emit campaignDeleted(campaignId);
        return true;
    }
    return false;
}

EmailCampaign Database::getCampaign(const QString &campaignId)
{
    EmailCampaign campaign;
    
    QString query = "SELECT * FROM campaigns WHERE id = ?";
    QSqlQuery sqlQuery = prepareQuery(query);
    sqlQuery.addBindValue(campaignId);
    
    if (sqlQuery.exec() && sqlQuery.next()) {
        campaign.setId(sqlQuery.value("id").toString());
        campaign.setSubject(sqlQuery.value("subject").toString());
        campaign.setBody(sqlQuery.value("body").toString());
        campaign.setSenderEmail(sqlQuery.value("sender_email").toString());
        campaign.setSenderName(sqlQuery.value("sender_name").toString());
        campaign.setStatus(sqlQuery.value("status").toString());
        campaign.setCreatedDate(QDateTime::fromString(sqlQuery.value("created_date").toString(), Qt::ISODate));
        
        QString sentDate = sqlQuery.value("sent_date").toString();
        if (!sentDate.isEmpty()) {
            campaign.setSentDate(QDateTime::fromString(sentDate, Qt::ISODate));
        }
    }
    
    return campaign;
}

QList<EmailCampaign> Database::getAllCampaigns()
{
    QList<EmailCampaign> campaigns;
    
    QString query = "SELECT * FROM campaigns ORDER BY created_date DESC";
    QSqlQuery sqlQuery = prepareQuery(query);
    
    if (sqlQuery.exec()) {
        while (sqlQuery.next()) {
            EmailCampaign campaign;
            campaign.setId(sqlQuery.value("id").toString());
            campaign.setSubject(sqlQuery.value("subject").toString());
            campaign.setBody(sqlQuery.value("body").toString());
            campaign.setSenderEmail(sqlQuery.value("sender_email").toString());
            campaign.setSenderName(sqlQuery.value("sender_name").toString());
            campaign.setStatus(sqlQuery.value("status").toString());
            campaign.setCreatedDate(QDateTime::fromString(sqlQuery.value("created_date").toString(), Qt::ISODate));
            
            QString sentDate = sqlQuery.value("sent_date").toString();
            if (!sentDate.isEmpty()) {
                campaign.setSentDate(QDateTime::fromString(sentDate, Qt::ISODate));
            }
            
            campaigns.append(campaign);
        }
    }
    
    return campaigns;
}

bool Database::saveEmail(const QString &email, const QString &name, const QString &campaignId)
{
    QString query = "INSERT OR REPLACE INTO emails (email, name, campaign_id) VALUES (?, ?, ?)";
    QVariantList params;
    params << email << name << campaignId;
    
    return executePreparedQuery(query, params);
}

bool Database::updateEmailStatus(const QString &email, const QString &status, const QString &campaignId)
{
    QString query = "UPDATE emails SET status = ?, sent_date = ? WHERE email = ? AND campaign_id = ?";
    QVariantList params;
    params << status << QDateTime::currentDateTime().toString(Qt::ISODate) << email << campaignId;
    
    return executePreparedQuery(query, params);
}

QList<QString> Database::getEmailsByCampaign(const QString &campaignId)
{
    QList<QString> emails;
    
    QString query = "SELECT email FROM emails WHERE campaign_id = ?";
    QSqlQuery sqlQuery = prepareQuery(query);
    sqlQuery.addBindValue(campaignId);
    
    if (sqlQuery.exec()) {
        while (sqlQuery.next()) {
            emails.append(sqlQuery.value("email").toString());
        }
    }
    
    return emails;
}

int Database::getTotalEmails()
{
    QString query = "SELECT COUNT(*) FROM emails";
    QSqlQuery sqlQuery = prepareQuery(query);
    
    if (sqlQuery.exec() && sqlQuery.next()) {
        return sqlQuery.value(0).toInt();
    }
    
    return 0;
}

int Database::getSentEmails()
{
    QString query = "SELECT COUNT(*) FROM emails WHERE status = 'Sent'";
    QSqlQuery sqlQuery = prepareQuery(query);
    
    if (sqlQuery.exec() && sqlQuery.next()) {
        return sqlQuery.value(0).toInt();
    }
    
    return 0;
}

int Database::getFailedEmails()
{
    QString query = "SELECT COUNT(*) FROM emails WHERE status = 'Failed'";
    QSqlQuery sqlQuery = prepareQuery(query);
    
    if (sqlQuery.exec() && sqlQuery.next()) {
        return sqlQuery.value(0).toInt();
    }
    
    return 0;
}

int Database::getPendingEmails()
{
    QString query = "SELECT COUNT(*) FROM emails WHERE status = 'Pending'";
    QSqlQuery sqlQuery = prepareQuery(query);
    
    if (sqlQuery.exec() && sqlQuery.next()) {
        return sqlQuery.value(0).toInt();
    }
    
    return 0;
}

bool Database::cleanInvalidEmails()
{
    QString query = "DELETE FROM emails WHERE email NOT LIKE '%@%.%'";
    return executeQuery(query);
}

bool Database::removeDuplicateEmails()
{
    QString query = R"(
        DELETE FROM emails 
        WHERE id NOT IN (
            SELECT MIN(id) 
            FROM emails 
            GROUP BY email, campaign_id
        )
    )";
    
    return executeQuery(query);
}

bool Database::exportToCsv(const QString &filename, const QString &campaignId)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "Email,Name,Campaign ID,Status,Sent Date,Error Message\n";
    
    // Write data
    QString query = campaignId.isEmpty() 
        ? "SELECT * FROM emails ORDER BY campaign_id, email"
        : "SELECT * FROM emails WHERE campaign_id = ? ORDER BY email";
    
    QSqlQuery sqlQuery = prepareQuery(query);
    if (!campaignId.isEmpty()) {
        sqlQuery.addBindValue(campaignId);
    }
    
    if (sqlQuery.exec()) {
        while (sqlQuery.next()) {
            QString line = QString("\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\"\n")
                .arg(sqlQuery.value("email").toString())
                .arg(sqlQuery.value("name").toString())
                .arg(sqlQuery.value("campaign_id").toString())
                .arg(sqlQuery.value("status").toString())
                .arg(sqlQuery.value("sent_date").toString())
                .arg(sqlQuery.value("error_message").toString());
            
            out << line;
        }
    }
    
    file.close();
    return true;
}

QString Database::getLastError() const
{
    return lastError;
}

void Database::close()
{
    if (connected) {
        db.close();
        connected = false;
    }
}

bool Database::executeQuery(const QString &query)
{
    QSqlQuery sqlQuery = prepareQuery(query);
    if (!sqlQuery.exec()) {
        setLastError(sqlQuery.lastError());
        return false;
    }
    return true;
}

bool Database::executePreparedQuery(const QString &query, const QVariantList &params)
{
    QSqlQuery sqlQuery = prepareQuery(query);
    
    for (const QVariant &param : params) {
        sqlQuery.addBindValue(param);
    }
    
    if (!sqlQuery.exec()) {
        setLastError(sqlQuery.lastError());
        return false;
    }
    return true;
}

QSqlQuery Database::prepareQuery(const QString &query)
{
    QSqlQuery sqlQuery(db);
    sqlQuery.prepare(query);
    return sqlQuery;
}

void Database::setLastError(const QSqlError &error)
{
    lastError = error.text();
    emit databaseError(lastError);
}