#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QDebug>
#include "emailmanager.h"

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    bool initialize();
    bool isOpen() const;
    
    // Email operations
    bool saveEmail(const EmailData &email);
    bool updateEmailStatus(const QString &email, const QString &status, const QString &errorMessage = "");
    bool updateEmailReceivedDate(const QString &email, const QDateTime &receivedDate);
    QList<EmailData> getEmailsByCampaign(const QString &campaignId);
    QList<EmailData> getAllEmails();
    bool deleteEmail(const QString &email);
    bool cleanInvalidEmails();
    
    // Campaign operations
    bool saveCampaign(const CampaignData &campaign);
    bool updateCampaign(const CampaignData &campaign);
    QList<CampaignData> getCampaigns(const QDateTime &startDate = QDateTime(), const QDateTime &endDate = QDateTime());
    CampaignData getCampaignById(const QString &campaignId);
    bool deleteCampaign(const QString &campaignId);
    
    // Statistics
    int getTotalEmails();
    int getSentEmails();
    int getReceivedEmails();
    int getFailedEmails();
    QMap<QString, int> getEmailStatsByDate(const QDateTime &startDate, const QDateTime &endDate);

private:
    QSqlDatabase db;
    
    bool createTables();
    bool createEmailTable();
    bool createCampaignTable();
    bool createActivityTable();
    
    EmailData emailFromRecord(const QSqlRecord &record);
    CampaignData campaignFromRecord(const QSqlRecord &record);
};

#endif // DATABASE_H