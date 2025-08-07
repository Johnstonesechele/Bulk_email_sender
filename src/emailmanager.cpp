#include "emailmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThreadPool>
#include <QMutexLocker>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>

EmailManager::EmailManager(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
    , threadPool(new QThreadPool(this))
{
    threadPool->setMaxThreadCount(10);
}

EmailManager::~EmailManager()
{
    threadPool->waitForDone();
}

void EmailManager::sendBulkEmails(const QList<EmailData> &emails, const QString &subject, 
                                 const QString &content, const SmtpSettings &settings)
{
    currentCampaign = CampaignData();
    currentCampaign.id = QUuid::createUuid().toString();
    currentCampaign.name = subject;
    currentCampaign.subject = subject;
    currentCampaign.sentDate = QDateTime::currentDateTime();
    currentCampaign.totalEmails = emails.size();
    currentCampaign.status = "Sending";
    
    emit campaignStarted(currentCampaign);
    
    for (const EmailData &email : emails) {
        EmailSender *sender = new EmailSender(email, subject, content, settings, this);
        connect(sender, &EmailSender::emailSent, this, &EmailManager::onEmailSent);
        connect(sender, &EmailSender::emailFailed, this, &EmailManager::onEmailFailed);
        threadPool->start(sender);
    }
}

void EmailManager::validateEmails(const QList<EmailData> &emails)
{
    for (const EmailData &email : emails) {
        EmailValidationWorker *worker = new EmailValidationWorker(email, this);
        connect(worker, &EmailValidationWorker::validationComplete, 
                this, &EmailManager::onValidationComplete);
        threadPool->start(worker);
    }
}

void EmailManager::testSmtpConnection(const SmtpSettings &settings)
{
    // Create a test email
    EmailData testEmail;
    testEmail.email = settings.username;
    testEmail.name = "Test";
    
    EmailSender *sender = new EmailSender(testEmail, "Test Connection", 
                                        "This is a test email to verify SMTP settings.", 
                                        settings, this);
    connect(sender, &EmailSender::emailSent, [this]() {
        emit smtpTestResult(true, "Connection successful!");
    });
    connect(sender, &EmailSender::emailFailed, [this](const QString &error) {
        emit smtpTestResult(false, error);
    });
    
    threadPool->start(sender);
}

void EmailManager::onEmailSent(const EmailData &email)
{
    QMutexLocker locker(&mutex);
    
    currentCampaign.sentEmails++;
    if (email.status == "Received") {
        currentCampaign.receivedEmails++;
    }
    
    emit emailStatusUpdated(email);
    
    if (currentCampaign.sentEmails >= currentCampaign.totalEmails) {
        currentCampaign.status = "Completed";
        emit campaignCompleted(currentCampaign);
    }
}

void EmailManager::onEmailFailed(const EmailData &email, const QString &error)
{
    QMutexLocker locker(&mutex);
    
    currentCampaign.failedEmails++;
    emit emailStatusUpdated(email);
    
    if (currentCampaign.sentEmails + currentCampaign.failedEmails >= currentCampaign.totalEmails) {
        currentCampaign.status = "Completed";
        emit campaignCompleted(currentCampaign);
    }
}

void EmailManager::onValidationComplete(const EmailData &email, const ValidationResult &result)
{
    EmailData updatedEmail = email;
    updatedEmail.isValid = result.isValid;
    updatedEmail.status = result.isValid ? "Valid" : "Invalid";
    updatedEmail.errorMessage = result.errorMessage;
    
    emit emailValidated(updatedEmail);
}

// EmailSender Implementation
EmailSender::EmailSender(const EmailData &email, const QString &subject, 
                        const QString &content, const SmtpSettings &settings, QObject *parent)
    : QRunnable()
    , email(email)
    , subject(subject)
    , content(content)
    , settings(settings)
    , parent(parent)
{
    setAutoDelete(true);
}

void EmailSender::run()
{
    // Simulate SMTP connection and email sending
    // In a real implementation, you would use a proper SMTP library
    // like libcurl or a Qt-based SMTP library
    
    QThread::msleep(100); // Simulate network delay
    
    // Simulate success/failure based on email domain
    bool success = !email.email.contains("invalid") && !email.email.contains("test");
    
    if (success) {
        EmailData updatedEmail = email;
        updatedEmail.status = "Sent";
        updatedEmail.sentDate = QDateTime::currentDateTime();
        
        // Simulate delivery confirmation
        if (QDateTime::currentDateTime().msecsSinceStartOfDay() % 3 == 0) {
            updatedEmail.status = "Received";
            updatedEmail.receivedDate = QDateTime::currentDateTime();
        }
        
        emit emailSent(updatedEmail);
    } else {
        emit emailFailed(email, "SMTP connection failed");
    }
}

// EmailValidationWorker Implementation
EmailValidationWorker::EmailValidationWorker(const EmailData &email, QObject *parent)
    : QRunnable()
    , email(email)
    , parent(parent)
{
    setAutoDelete(true);
}

void EmailValidationWorker::run()
{
    ValidationResult result;
    
    // Basic email format validation
    QRegExp emailRegex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}\\b");
    if (!emailRegex.exactMatch(email.email)) {
        result.isValid = false;
        result.errorMessage = "Invalid email format";
        emit validationComplete(email, result);
        return;
    }
    
    // Check for disposable email domains
    QStringList disposableDomains = {
        "10minutemail.com", "guerrillamail.com", "tempmail.org",
        "mailinator.com", "throwaway.email", "temp-mail.org"
    };
    
    QString domain = email.email.split("@").last().toLower();
    if (disposableDomains.contains(domain)) {
        result.isValid = false;
        result.errorMessage = "Disposable email domain detected";
        emit validationComplete(email, result);
        return;
    }
    
    // Check for role-based emails
    QStringList rolePrefixes = {"admin", "info", "support", "contact", "sales", "help"};
    QString localPart = email.email.split("@").first().toLower();
    for (const QString &prefix : rolePrefixes) {
        if (localPart == prefix) {
            result.isValid = false;
            result.errorMessage = "Role-based email detected";
            emit validationComplete(email, result);
            return;
        }
    }
    
    result.isValid = true;
    result.errorMessage = "Valid email";
    emit validationComplete(email, result);
}