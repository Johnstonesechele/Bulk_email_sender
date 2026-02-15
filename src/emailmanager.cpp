#include "emailmanager.h"
#include <QNetworkProxy>
#include <QSslConfiguration>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>

EmailManager::EmailManager(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
    , queueTimer(new QTimer(this))
    , isConnected_(false)
{
    initializeNetworkManager();
    
    // Set up queue timer
    queueTimer->setInterval(1000); // Process emails every second
    connect(queueTimer, &QTimer::timeout, this, &EmailManager::processEmailQueue);
    
    // Connect network signals
    connect(networkManager, &QNetworkAccessManager::finished, this, &EmailManager::onNetworkReplyFinished);
}

EmailManager::~EmailManager()
{
    if (queueTimer->isActive()) {
        queueTimer->stop();
    }
}

void EmailManager::initializeNetworkManager()
{
    // Configure SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    
    // Set up proxy if needed
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    networkManager->setProxy(proxy);
}

bool EmailManager::sendEmail(const QString &to, const QString &subject, const QString &body, const QString &from)
{
    if (!smtpConfig.isValid) {
        lastError = "SMTP configuration not set";
        emit errorOccurred(lastError);
        return false;
    }
    
    if (!validateEmail(to)) {
        lastError = "Invalid recipient email address";
        emit errorOccurred(lastError);
        return false;
    }
    
    // Create email campaign for single email
    EmailCampaign campaign;
    campaign.setId(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    campaign.setSubject(subject);
    campaign.setBody(body);
    campaign.setSenderEmail(from.isEmpty() ? senderInfo.email : from);
    campaign.setSenderName(senderInfo.name);
    campaign.addRecipient(to);
    
    // Add to queue
    emailQueue.enqueue(campaign);
    
    if (!queueTimer->isActive()) {
        queueTimer->start();
    }
    
    return true;
}

bool EmailManager::sendBulkEmails(const QList<EmailCampaign> &campaigns)
{
    if (!smtpConfig.isValid) {
        lastError = "SMTP configuration not set";
        emit errorOccurred(lastError);
        return false;
    }
    
    // Add all campaigns to queue
    for (const EmailCampaign &campaign : campaigns) {
        emailQueue.enqueue(campaign);
    }
    
    if (!queueTimer->isActive()) {
        queueTimer->start();
    }
    
    return true;
}

void EmailManager::setSmtpConfig(const QString &server, int port, const QString &username, const QString &password)
{
    smtpConfig.server = server;
    smtpConfig.port = port;
    smtpConfig.username = username;
    smtpConfig.password = password;
    smtpConfig.isValid = !server.isEmpty() && port > 0 && !username.isEmpty() && !password.isEmpty();
    
    if (smtpConfig.isValid) {
        isConnected_ = true;
        emit connectionStatusChanged(true);
    } else {
        isConnected_ = false;
        emit connectionStatusChanged(false);
    }
}

void EmailManager::setSenderInfo(const QString &name, const QString &email)
{
    senderInfo.name = name;
    senderInfo.email = email;
    senderInfo.isValid = !name.isEmpty() && validateEmail(email);
}

bool EmailManager::isConnected() const
{
    return isConnected_;
}

QString EmailManager::getLastError() const
{
    return lastError;
}

void EmailManager::addCampaign(const EmailCampaign &campaign)
{
    campaigns.append(campaign);
}

void EmailManager::removeCampaign(const QString &campaignId)
{
    for (int i = 0; i < campaigns.size(); ++i) {
        if (campaigns[i].getId() == campaignId) {
            campaigns.removeAt(i);
            break;
        }
    }
}

QList<EmailCampaign> EmailManager::getCampaigns() const
{
    return campaigns;
}

bool EmailManager::validateEmail(const QString &email)
{
    QRegularExpression emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return emailRegex.match(email).hasMatch();
}

QList<QString> EmailManager::validateEmailList(const QList<QString> &emails)
{
    QList<QString> invalidEmails;
    for (const QString &email : emails) {
        if (!validateEmail(email)) {
            invalidEmails.append(email);
        }
    }
    return invalidEmails;
}

void EmailManager::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QString response = QString::fromUtf8(reply->readAll());
        qDebug() << "Email sent successfully:" << response;
        
        // Extract email from the request
        QString email = reply->property("email").toString();
        emit emailSent(email, true);
    } else {
        QString errorString = reply->errorString();
        qDebug() << "Email sending failed:" << errorString;
        
        QString email = reply->property("email").toString();
        emit emailSent(email, false);
        emit errorOccurred(errorString);
    }
    
    reply->deleteLater();
}

void EmailManager::onNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        lastError = reply->errorString();
        emit errorOccurred(lastError);
        reply->deleteLater();
    }
}

void EmailManager::processEmailQueue()
{
    if (emailQueue.isEmpty()) {
        queueTimer->stop();
        return;
    }
    
    EmailCampaign campaign = emailQueue.dequeue();
    processNextEmail();
}

void EmailManager::processNextEmail()
{
    // This is a simplified implementation
    // In a real application, you would implement proper SMTP protocol handling
    
    // For now, we'll simulate email sending
    QTimer::singleShot(100, [this]() {
        // Simulate email processing
        emit emailSent("test@example.com", true);
    });
}

bool EmailManager::authenticateSmtp()
{
    // This would implement actual SMTP authentication
    // For now, return true to simulate successful authentication
    return smtpConfig.isValid;
}

QString EmailManager::formatEmailMessage(const QString &to, const QString &subject, const QString &body)
{
    QString message;
    message += "From: " + senderInfo.name + " <" + senderInfo.email + ">\r\n";
    message += "To: " + to + "\r\n";
    message += "Subject: " + subject + "\r\n";
    message += "Date: " + QDateTime::currentDateTime().toString("ddd, dd MMM yyyy hh:mm:ss +0000") + "\r\n";
    message += "MIME-Version: 1.0\r\n";
    message += "Content-Type: text/plain; charset=UTF-8\r\n";
    message += "Content-Transfer-Encoding: 8bit\r\n\r\n";
    message += body + "\r\n";
    
    return message;
}