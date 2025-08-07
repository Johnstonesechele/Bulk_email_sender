#ifndef EMAILMANAGER_H
#define EMAILMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>

struct EmailData {
    QString email;
    QString name;
    QString status;
    QDateTime sentDate;
    QDateTime receivedDate;
    QString campaignId;
    QString errorMessage;
    bool isValid;
    
    EmailData() : isValid(false) {}
    EmailData(const QString &email, const QString &name = "") 
        : email(email), name(name), status("Pending"), isValid(false) {}
};

struct CampaignData {
    QString id;
    QString name;
    QString subject;
    QDateTime sentDate;
    int totalEmails;
    int sentEmails;
    int receivedEmails;
    int failedEmails;
    QString status;
    
    CampaignData() : totalEmails(0), sentEmails(0), receivedEmails(0), failedEmails(0) {}
};

struct SmtpSettings {
    QString server;
    int port;
    QString username;
    QString password;
    bool useSsl;
    
    SmtpSettings() : port(587), useSsl(true) {}
};

class EmailManager : public QObject
{
    Q_OBJECT

public:
    explicit EmailManager(QObject *parent = nullptr);
    ~EmailManager();

    bool sendEmail(const EmailData &emailData, const QString &subject, const QString &content);
    bool sendBulkEmails(const QList<EmailData> &emails, const QString &subject, const QString &content);
    void validateEmails(QList<EmailData> &emails);
    void cleanEmailData(QList<EmailData> &emails);
    
    void setSmtpSettings(const SmtpSettings &settings);
    SmtpSettings getSmtpSettings() const;
    
    bool testConnection();

signals:
    void emailSent(const EmailData &email);
    void emailFailed(const EmailData &email, const QString &error);
    void progressUpdated(int current, int total);
    void campaignCompleted(const CampaignData &campaign);
    void connectionTested(bool success, const QString &message);

private slots:
    void handleEmailResponse(QNetworkReply *reply);
    void handleConnectionTest(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    SmtpSettings smtpSettings;
    QMutex mutex;
    QThreadPool threadPool;
    
    QString generateCampaignId();
    bool isValidEmail(const QString &email);
    void logEmailActivity(const EmailData &email, const QString &action);
};

class EmailSender : public QObject, public QRunnable
{
    Q_OBJECT

public:
    EmailSender(const EmailData &email, const QString &subject, const QString &content, 
                const SmtpSettings &settings, QObject *parent = nullptr);
    
    void run() override;

signals:
    void emailSent(const EmailData &email);
    void emailFailed(const EmailData &email, const QString &error);

private:
    EmailData emailData;
    QString subject;
    QString content;
    SmtpSettings smtpSettings;
};

#endif // EMAILMANAGER_H