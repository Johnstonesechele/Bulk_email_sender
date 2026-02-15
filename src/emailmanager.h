#ifndef EMAILMANAGER_H
#define EMAILMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QQueue>
#include "emailcampaign.h"

class EmailManager : public QObject
{
    Q_OBJECT

public:
    explicit EmailManager(QObject *parent = nullptr);
    ~EmailManager();

    // Email sending methods
    bool sendEmail(const QString &to, const QString &subject, const QString &body, const QString &from);
    bool sendBulkEmails(const QList<EmailCampaign> &campaigns);
    
    // Configuration methods
    void setSmtpConfig(const QString &server, int port, const QString &username, const QString &password);
    void setSenderInfo(const QString &name, const QString &email);
    
    // Status methods
    bool isConnected() const;
    QString getLastError() const;
    
    // Campaign management
    void addCampaign(const EmailCampaign &campaign);
    void removeCampaign(const QString &campaignId);
    QList<EmailCampaign> getCampaigns() const;
    
    // Email validation
    bool validateEmail(const QString &email);
    QList<QString> validateEmailList(const QList<QString> &emails);

signals:
    void emailSent(const QString &email, bool success);
    void bulkEmailProgress(int current, int total);
    void bulkEmailCompleted(int successCount, int failureCount);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);

private slots:
    void onNetworkReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);
    void processEmailQueue();

private:
    struct SmtpConfig {
        QString server;
        int port;
        QString username;
        QString password;
        bool isValid;
    };
    
    struct SenderInfo {
        QString name;
        QString email;
        bool isValid;
    };
    
    SmtpConfig smtpConfig;
    SenderInfo senderInfo;
    QNetworkAccessManager *networkManager;
    QQueue<EmailCampaign> emailQueue;
    QTimer *queueTimer;
    bool isConnected_;
    QString lastError;
    QList<EmailCampaign> campaigns;
    
    void initializeNetworkManager();
    void processNextEmail();
    bool authenticateSmtp();
    QString formatEmailMessage(const QString &to, const QString &subject, const QString &body);
};

#endif // EMAILMANAGER_H