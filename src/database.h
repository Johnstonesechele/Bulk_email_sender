#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QDateTime>
#include <QList>
#include "emailcampaign.h"

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    // Database initialization
    bool initialize();
    bool createTables();
    bool isConnected() const;

    // Campaign management
    bool saveCampaign(const EmailCampaign &campaign);
    bool updateCampaign(const EmailCampaign &campaign);
    bool deleteCampaign(const QString &campaignId);
    EmailCampaign getCampaign(const QString &campaignId);
    QList<EmailCampaign> getAllCampaigns();
    QList<EmailCampaign> getCampaignsByDateRange(const QDateTime &start, const QDateTime &end);

    // Email management
    bool saveEmail(const QString &email, const QString &name, const QString &campaignId);
    bool updateEmailStatus(const QString &email, const QString &status, const QString &campaignId);
    bool deleteEmail(const QString &email);
    QList<QString> getEmailsByCampaign(const QString &campaignId);
    QList<QString> getEmailsByStatus(const QString &status);

    // Statistics
    int getTotalEmails();
    int getSentEmails();
    int getFailedEmails();
    int getPendingEmails();
    QMap<QString, int> getCampaignStatistics();

    // Data cleaning
    bool cleanInvalidEmails();
    bool removeDuplicateEmails();
    bool updateEmailStatuses();

    // Export/Import
    bool exportToCsv(const QString &filename, const QString &campaignId = QString());
    bool importFromCsv(const QString &filename);

    // Utility methods
    QString getLastError() const;
    void close();

signals:
    void databaseError(const QString &error);
    void campaignSaved(const QString &campaignId);
    void campaignDeleted(const QString &campaignId);

private:
    QSqlDatabase db;
    QString lastError;
    bool connected;

    bool executeQuery(const QString &query);
    bool executePreparedQuery(const QString &query, const QVariantList &params);
    QSqlQuery prepareQuery(const QString &query);
    void setLastError(const QSqlError &error);
};

#endif // DATABASE_H