#include "emailvalidator.h"
#include <QRegExp>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDebug>
#include <QThread>

EmailValidator::EmailValidator(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
{
    // Initialize disposable email domains
    disposableDomains = {
        "10minutemail.com", "guerrillamail.com", "tempmail.org",
        "mailinator.com", "throwaway.email", "temp-mail.org",
        "sharklasers.com", "guerrillamailblock.com", "pokemail.net",
        "spam4.me", "bccto.me", "chacuo.net", "dispostable.com",
        "fakeinbox.com", "getairmail.com", "mailnesia.com",
        "maildrop.cc", "mailinator.net", "mailmetrash.com",
        "tempr.email", "tmpeml.com", "yopmail.com", "yopmail.net"
    };
    
    // Initialize role-based email prefixes
    rolePrefixes = {
        "admin", "info", "support", "contact", "sales", "help",
        "service", "customer", "feedback", "noreply", "no-reply",
        "donotreply", "do-not-reply", "webmaster", "postmaster",
        "abuse", "security", "billing", "accounts", "hr", "jobs"
    };
}

EmailValidator::~EmailValidator()
{
}

ValidationResult EmailValidator::validateEmail(const QString &email)
{
    ValidationResult result;
    result.email = email;
    result.isValid = true;
    result.errorMessage = "Valid email";
    
    // Basic format validation
    if (!isValidFormat(email)) {
        result.isValid = false;
        result.errorMessage = "Invalid email format";
        return result;
    }
    
    // Check for disposable email domains
    if (isDisposableEmail(email)) {
        result.isValid = false;
        result.errorMessage = "Disposable email domain detected";
        return result;
    }
    
    // Check for role-based emails
    if (isRoleBasedEmail(email)) {
        result.isValid = false;
        result.errorMessage = "Role-based email detected";
        return result;
    }
    
    // Check for common spam patterns
    if (hasSpamPatterns(email)) {
        result.isValid = false;
        result.errorMessage = "Suspicious email pattern detected";
        return result;
    }
    
    return result;
}

bool EmailValidator::isValidFormat(const QString &email)
{
    // RFC 5322 compliant email regex
    QRegExp emailRegex("^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$");
    
    if (!emailRegex.exactMatch(email)) {
        return false;
    }
    
    // Additional checks
    QStringList parts = email.split("@");
    if (parts.size() != 2) {
        return false;
    }
    
    QString localPart = parts[0];
    QString domain = parts[1];
    
    // Check local part length
    if (localPart.length() > 64 || localPart.length() == 0) {
        return false;
    }
    
    // Check domain length
    if (domain.length() > 253 || domain.length() == 0) {
        return false;
    }
    
    // Check for consecutive dots
    if (localPart.contains("..") || domain.contains("..")) {
        return false;
    }
    
    // Check for dots at start/end
    if (localPart.startsWith(".") || localPart.endsWith(".") ||
        domain.startsWith(".") || domain.endsWith(".")) {
        return false;
    }
    
    return true;
}

bool EmailValidator::isDisposableEmail(const QString &email)
{
    QString domain = email.split("@").last().toLower();
    return disposableDomains.contains(domain);
}

bool EmailValidator::isRoleBasedEmail(const QString &email)
{
    QString localPart = email.split("@").first().toLower();
    
    // Check for exact matches
    if (rolePrefixes.contains(localPart)) {
        return true;
    }
    
    // Check for role-based patterns with numbers or special characters
    for (const QString &prefix : rolePrefixes) {
        if (localPart.startsWith(prefix + ".") || 
            localPart.startsWith(prefix + "_") ||
            localPart.startsWith(prefix + "-") ||
            localPart.startsWith(prefix + "1") ||
            localPart.startsWith(prefix + "2")) {
            return true;
        }
    }
    
    return false;
}

bool EmailValidator::hasSpamPatterns(const QString &email)
{
    QString localPart = email.split("@").first().toLower();
    
    // Check for excessive numbers
    int digitCount = 0;
    for (QChar ch : localPart) {
        if (ch.isDigit()) {
            digitCount++;
        }
    }
    
    if (digitCount > localPart.length() * 0.5) {
        return true;
    }
    
    // Check for excessive special characters
    int specialCount = 0;
    for (QChar ch : localPart) {
        if (!ch.isLetterOrNumber() && ch != '.' && ch != '_' && ch != '-') {
            specialCount++;
        }
    }
    
    if (specialCount > 2) {
        return true;
    }
    
    // Check for suspicious patterns
    QStringList suspiciousPatterns = {
        "test", "demo", "example", "sample", "fake", "dummy",
        "spam", "bot", "auto", "system", "noreply", "no-reply"
    };
    
    for (const QString &pattern : suspiciousPatterns) {
        if (localPart.contains(pattern)) {
            return true;
        }
    }
    
    return false;
}

void EmailValidator::validateEmailAsync(const QString &email)
{
    // For now, we'll do synchronous validation
    // In a real implementation, you might want to add online validation
    ValidationResult result = validateEmail(email);
    emit validationComplete(result);
}

void EmailValidator::validateEmailsAsync(const QStringList &emails)
{
    for (const QString &email : emails) {
        validateEmailAsync(email);
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
    EmailValidator validator;
    ValidationResult result = validator.validateEmail(email.email);
    
    EmailData updatedEmail = email;
    updatedEmail.isValid = result.isValid;
    updatedEmail.status = result.isValid ? "Valid" : "Invalid";
    updatedEmail.errorMessage = result.errorMessage;
    
    emit validationComplete(updatedEmail, result);
}

QStringList EmailValidator::getDisposableDomains() const
{
    return disposableDomains;
}

void EmailValidator::addDisposableDomain(const QString &domain)
{
    if (!disposableDomains.contains(domain.toLower())) {
        disposableDomains.append(domain.toLower());
    }
}

void EmailValidator::removeDisposableDomain(const QString &domain)
{
    disposableDomains.removeAll(domain.toLower());
}

QStringList EmailValidator::getRolePrefixes() const
{
    return rolePrefixes;
}

void EmailValidator::addRolePrefix(const QString &prefix)
{
    if (!rolePrefixes.contains(prefix.toLower())) {
        rolePrefixes.append(prefix.toLower());
    }
}

void EmailValidator::removeRolePrefix(const QString &prefix)
{
    rolePrefixes.removeAll(prefix.toLower());
}

bool EmailValidator::validateDomain(const QString &domain)
{
    // Basic domain validation
    if (domain.isEmpty() || domain.length() > 253) {
        return false;
    }
    
    // Check for valid characters
    QRegExp domainRegex("^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$");
    if (!domainRegex.exactMatch(domain)) {
        return false;
    }
    
    // Check for consecutive dots
    if (domain.contains("..")) {
        return false;
    }
    
    // Check for dots at start/end
    if (domain.startsWith(".") || domain.endsWith(".")) {
        return false;
    }
    
    return true;
}

QString EmailValidator::extractDomain(const QString &email)
{
    QStringList parts = email.split("@");
    if (parts.size() == 2) {
        return parts[1].toLower();
    }
    return QString();
}

QString EmailValidator::extractLocalPart(const QString &email)
{
    QStringList parts = email.split("@");
    if (parts.size() == 2) {
        return parts[0];
    }
    return QString();
}