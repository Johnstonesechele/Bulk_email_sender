#include "emailvalidator.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QDebug>
#include <QFile>
#include <QTextStream>

EmailValidator::EmailValidator(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
{
    loadDisposableDomains();
    loadRoleBasedPrefixes();
    
    connect(networkManager, &QNetworkAccessManager::finished, this, &EmailValidator::onNetworkReplyFinished);
}

EmailValidator::~EmailValidator()
{
}

bool EmailValidator::isValidEmailFormat(const QString &email)
{
    // RFC 5322 compliant email regex
    QRegularExpression emailRegex(R"(
        ^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$
    )", QRegularExpression::CaseInsensitiveOption);
    
    return emailRegex.match(email.trimmed()).hasMatch();
}

bool EmailValidator::isValidDomain(const QString &email)
{
    QString domain = extractDomain(email);
    if (domain.isEmpty()) return false;
    
    // Check if domain has at least one dot and valid characters
    QRegularExpression domainRegex(R"(^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$)");
    return domainRegex.match(domain).hasMatch();
}

bool EmailValidator::hasValidMXRecord(const QString &email)
{
    QString domain = extractDomain(email);
    if (domain.isEmpty()) return false;
    
    return checkMXRecord(domain);
}

bool EmailValidator::isDisposableEmail(const QString &email)
{
    QString domain = extractDomain(email).toLower();
    return disposableDomains.contains(domain);
}

bool EmailValidator::isRoleBasedEmail(const QString &email)
{
    QString localPart = email.split('@').first().toLower();
    
    for (const QString &prefix : roleBasedPrefixes) {
        if (localPart.startsWith(prefix)) {
            return true;
        }
    }
    
    return false;
}

QList<QString> EmailValidator::validateEmailList(const QList<QString> &emails)
{
    QList<QString> validEmails;
    
    for (int i = 0; i < emails.size(); ++i) {
        QString email = emails[i];
        emit validationProgress(i + 1, emails.size());
        
        if (isValidEmailFormat(email) && !isDisposableEmail(email) && !isRoleBasedEmail(email)) {
            validEmails.append(cleanEmail(email));
        }
    }
    
    return validEmails;
}

QMap<QString, QString> EmailValidator::validateEmailsWithDetails(const QList<QString> &emails)
{
    QMap<QString, QString> results;
    
    for (const QString &email : emails) {
        QString cleanedEmail = cleanEmail(email);
        QString status = "Valid";
        
        if (!isValidEmailFormat(cleanedEmail)) {
            status = "Invalid format";
        } else if (isDisposableEmail(cleanedEmail)) {
            status = "Disposable email";
        } else if (isRoleBasedEmail(cleanedEmail)) {
            status = "Role-based email";
        } else if (!isValidDomain(cleanedEmail)) {
            status = "Invalid domain";
        }
        
        results[cleanedEmail] = status;
    }
    
    return results;
}

QString EmailValidator::cleanEmail(const QString &email)
{
    QString cleaned = email.trimmed().toLower();
    
    // Remove extra spaces
    cleaned = cleaned.simplified();
    
    // Remove common invalid characters
    cleaned = cleaned.replace(QRegularExpression("[\\s\\t\\n\\r]"), "");
    
    return cleaned;
}

QList<QString> EmailValidator::cleanEmailList(const QList<QString> &emails)
{
    QList<QString> cleanedEmails;
    QSet<QString> uniqueEmails;
    
    for (const QString &email : emails) {
        QString cleaned = cleanEmail(email);
        if (!cleaned.isEmpty() && !uniqueEmails.contains(cleaned)) {
            cleanedEmails.append(cleaned);
            uniqueEmails.insert(cleaned);
        }
    }
    
    return cleanedEmails;
}

EmailValidator::ValidationStats EmailValidator::getValidationStats(const QList<QString> &emails)
{
    ValidationStats stats;
    stats.total = emails.size();
    stats.valid = 0;
    stats.invalid = 0;
    stats.disposable = 0;
    stats.roleBased = 0;
    stats.duplicate = 0;
    stats.malformed = 0;
    
    QSet<QString> seenEmails;
    
    for (const QString &email : emails) {
        QString cleaned = cleanEmail(email);
        
        if (seenEmails.contains(cleaned)) {
            stats.duplicate++;
            continue;
        }
        seenEmails.insert(cleaned);
        
        if (!isValidEmailFormat(cleaned)) {
            stats.malformed++;
            stats.invalid++;
        } else if (isDisposableEmail(cleaned)) {
            stats.disposable++;
            stats.invalid++;
        } else if (isRoleBasedEmail(cleaned)) {
            stats.roleBased++;
            stats.invalid++;
        } else {
            stats.valid++;
        }
    }
    
    return stats;
}

void EmailValidator::loadDisposableDomains()
{
    // Common disposable email domains
    disposableDomains = {
        "10minutemail.com", "guerrillamail.com", "mailinator.com", "tempmail.org",
        "throwaway.email", "yopmail.com", "mailnesia.com", "sharklasers.com",
        "getairmail.com", "maildrop.cc", "mailinator.net", "tempr.email",
        "dispostable.com", "mailmetrash.com", "trashmail.net", "mailnull.com"
    };
}

void EmailValidator::loadRoleBasedPrefixes()
{
    // Common role-based email prefixes
    roleBasedPrefixes = {
        "admin", "administrator", "info", "contact", "support", "help",
        "sales", "marketing", "noreply", "no-reply", "donotreply", "do-not-reply",
        "webmaster", "postmaster", "hostmaster", "abuse", "security",
        "billing", "accounts", "team", "service", "customer", "client"
    };
}

bool EmailValidator::checkMXRecord(const QString &domain)
{
    // This is a simplified implementation
    // In a real application, you would implement actual DNS MX record checking
    
    // For now, we'll assume all domains are valid
    return !domain.isEmpty();
}

QString EmailValidator::extractDomain(const QString &email)
{
    int atIndex = email.indexOf('@');
    if (atIndex == -1 || atIndex == email.length() - 1) {
        return QString();
    }
    
    return email.mid(atIndex + 1);
}

bool EmailValidator::isDuplicateEmail(const QString &email, const QList<QString> &emailList)
{
    return emailList.contains(email, Qt::CaseInsensitive);
}

void EmailValidator::onNetworkReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        reply->deleteLater();
    }
}