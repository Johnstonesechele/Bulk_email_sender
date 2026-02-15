#ifndef EMAILCAMPAIGN_H
#define EMAILCAMPAIGN_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QObject>

class EmailCampaign : public QObject
{
    Q_OBJECT

public:
    explicit EmailCampaign(QObject *parent = nullptr);
    EmailCampaign(const EmailCampaign &other);
    ~EmailCampaign();

    // Getters
    QString getId() const;
    QString getSubject() const;
    QString getBody() const;
    QString getSenderEmail() const;
    QString getSenderName() const;
    QList<QString> getRecipients() const;
    QDateTime getCreatedDate() const;
    QDateTime getSentDate() const;
    QString getStatus() const;
    int getTotalRecipients() const;
    int getSentCount() const;
    int getFailedCount() const;

    // Setters
    void setId(const QString &id);
    void setSubject(const QString &subject);
    void setBody(const QString &body);
    void setSenderEmail(const QString &email);
    void setSenderName(const QString &name);
    void setCreatedDate(const QDateTime &date);
    void setSentDate(const QDateTime &date);
    void setStatus(const QString &status);

    // Recipient management
    void addRecipient(const QString &email);
    void removeRecipient(const QString &email);
    void clearRecipients();
    void setRecipients(const QList<QString> &recipients);

    // Campaign status
    void markAsSent(const QString &email);
    void markAsFailed(const QString &email);
    void resetStatus();

    // Utility methods
    bool isValid() const;
    QString toString() const;
    QJsonObject toJson() const;
    static EmailCampaign fromJson(const QJsonObject &json);

    // Operators
    EmailCampaign& operator=(const EmailCampaign &other);
    bool operator==(const EmailCampaign &other) const;
    bool operator!=(const EmailCampaign &other) const;

signals:
    void statusChanged(const QString &status);
    void recipientAdded(const QString &email);
    void recipientRemoved(const QString &email);
    void campaignCompleted();

private:
    QString id;
    QString subject;
    QString body;
    QString senderEmail;
    QString senderName;
    QList<QString> recipients;
    QDateTime createdDate;
    QDateTime sentDate;
    QString status;
    int sentCount;
    int failedCount;
    
    void initialize();
};

#endif // EMAILCAMPAIGN_H