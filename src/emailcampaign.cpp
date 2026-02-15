#include "emailcampaign.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>

EmailCampaign::EmailCampaign(QObject *parent)
    : QObject(parent)
    , sentCount(0)
    , failedCount(0)
{
    initialize();
}

EmailCampaign::EmailCampaign(const EmailCampaign &other)
    : QObject(other.parent())
    , id(other.id)
    , subject(other.subject)
    , body(other.body)
    , senderEmail(other.senderEmail)
    , senderName(other.senderName)
    , recipients(other.recipients)
    , createdDate(other.createdDate)
    , sentDate(other.sentDate)
    , status(other.status)
    , sentCount(other.sentCount)
    , failedCount(other.failedCount)
{
}

EmailCampaign::~EmailCampaign()
{
}

void EmailCampaign::initialize()
{
    id = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    createdDate = QDateTime::currentDateTime();
    status = "Draft";
    sentCount = 0;
    failedCount = 0;
}

QString EmailCampaign::getId() const
{
    return id;
}

QString EmailCampaign::getSubject() const
{
    return subject;
}

QString EmailCampaign::getBody() const
{
    return body;
}

QString EmailCampaign::getSenderEmail() const
{
    return senderEmail;
}

QString EmailCampaign::getSenderName() const
{
    return senderName;
}

QList<QString> EmailCampaign::getRecipients() const
{
    return recipients;
}

QDateTime EmailCampaign::getCreatedDate() const
{
    return createdDate;
}

QDateTime EmailCampaign::getSentDate() const
{
    return sentDate;
}

QString EmailCampaign::getStatus() const
{
    return status;
}

int EmailCampaign::getTotalRecipients() const
{
    return recipients.size();
}

int EmailCampaign::getSentCount() const
{
    return sentCount;
}

int EmailCampaign::getFailedCount() const
{
    return failedCount;
}

void EmailCampaign::setId(const QString &id)
{
    this->id = id;
}

void EmailCampaign::setSubject(const QString &subject)
{
    this->subject = subject;
}

void EmailCampaign::setBody(const QString &body)
{
    this->body = body;
}

void EmailCampaign::setSenderEmail(const QString &email)
{
    this->senderEmail = email;
}

void EmailCampaign::setSenderName(const QString &name)
{
    this->senderName = name;
}

void EmailCampaign::setCreatedDate(const QDateTime &date)
{
    this->createdDate = date;
}

void EmailCampaign::setSentDate(const QDateTime &date)
{
    this->sentDate = date;
}

void EmailCampaign::setStatus(const QString &status)
{
    if (this->status != status) {
        this->status = status;
        emit statusChanged(status);
    }
}

void EmailCampaign::addRecipient(const QString &email)
{
    if (!recipients.contains(email)) {
        recipients.append(email);
        emit recipientAdded(email);
    }
}

void EmailCampaign::removeRecipient(const QString &email)
{
    if (recipients.removeOne(email)) {
        emit recipientRemoved(email);
    }
}

void EmailCampaign::clearRecipients()
{
    recipients.clear();
}

void EmailCampaign::setRecipients(const QList<QString> &recipients)
{
    this->recipients = recipients;
}

void EmailCampaign::markAsSent(const QString &email)
{
    sentCount++;
    if (sentCount + failedCount >= recipients.size()) {
        setStatus("Completed");
        emit campaignCompleted();
    }
}

void EmailCampaign::markAsFailed(const QString &email)
{
    failedCount++;
    if (sentCount + failedCount >= recipients.size()) {
        setStatus("Completed");
        emit campaignCompleted();
    }
}

void EmailCampaign::resetStatus()
{
    sentCount = 0;
    failedCount = 0;
    setStatus("Draft");
}

bool EmailCampaign::isValid() const
{
    return !id.isEmpty() && !subject.isEmpty() && !body.isEmpty() && !senderEmail.isEmpty() && !recipients.isEmpty();
}

QString EmailCampaign::toString() const
{
    QString result;
    result += "Campaign ID: " + id + "\n";
    result += "Subject: " + subject + "\n";
    result += "Sender: " + senderName + " <" + senderEmail + ">\n";
    result += "Recipients: " + QString::number(recipients.size()) + "\n";
    result += "Status: " + status + "\n";
    result += "Created: " + createdDate.toString("yyyy-MM-dd hh:mm:ss") + "\n";
    if (sentDate.isValid()) {
        result += "Sent: " + sentDate.toString("yyyy-MM-dd hh:mm:ss") + "\n";
    }
    return result;
}

QJsonObject EmailCampaign::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["subject"] = subject;
    json["body"] = body;
    json["senderEmail"] = senderEmail;
    json["senderName"] = senderName;
    json["status"] = status;
    json["createdDate"] = createdDate.toString(Qt::ISODate);
    if (sentDate.isValid()) {
        json["sentDate"] = sentDate.toString(Qt::ISODate);
    }
    json["sentCount"] = sentCount;
    json["failedCount"] = failedCount;
    
    QJsonArray recipientsArray;
    for (const QString &recipient : recipients) {
        recipientsArray.append(recipient);
    }
    json["recipients"] = recipientsArray;
    
    return json;
}

EmailCampaign EmailCampaign::fromJson(const QJsonObject &json)
{
    EmailCampaign campaign;
    
    if (json.contains("id")) {
        campaign.setId(json["id"].toString());
    }
    if (json.contains("subject")) {
        campaign.setSubject(json["subject"].toString());
    }
    if (json.contains("body")) {
        campaign.setBody(json["body"].toString());
    }
    if (json.contains("senderEmail")) {
        campaign.setSenderEmail(json["senderEmail"].toString());
    }
    if (json.contains("senderName")) {
        campaign.setSenderName(json["senderName"].toString());
    }
    if (json.contains("status")) {
        campaign.setStatus(json["status"].toString());
    }
    if (json.contains("createdDate")) {
        campaign.setCreatedDate(QDateTime::fromString(json["createdDate"].toString(), Qt::ISODate));
    }
    if (json.contains("sentDate")) {
        campaign.setSentDate(QDateTime::fromString(json["sentDate"].toString(), Qt::ISODate));
    }
    
    if (json.contains("recipients")) {
        QJsonArray recipientsArray = json["recipients"].toArray();
        QList<QString> recipients;
        for (const QJsonValue &value : recipientsArray) {
            recipients.append(value.toString());
        }
        campaign.setRecipients(recipients);
    }
    
    return campaign;
}

EmailCampaign& EmailCampaign::operator=(const EmailCampaign &other)
{
    if (this != &other) {
        id = other.id;
        subject = other.subject;
        body = other.body;
        senderEmail = other.senderEmail;
        senderName = other.senderName;
        recipients = other.recipients;
        createdDate = other.createdDate;
        sentDate = other.sentDate;
        status = other.status;
        sentCount = other.sentCount;
        failedCount = other.failedCount;
    }
    return *this;
}

bool EmailCampaign::operator==(const EmailCampaign &other) const
{
    return id == other.id;
}

bool EmailCampaign::operator!=(const EmailCampaign &other) const
{
    return !(*this == other);
}