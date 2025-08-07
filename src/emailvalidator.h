#ifndef EMAILVALIDATOR_H
#define EMAILVALIDATOR_H

#include <QObject>
#include <QString>
#include <QRegExp>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QThreadPool>
#include <QRunnable>

class EmailValidator : public QObject
{
    Q_OBJECT

public:
    explicit EmailValidator(QObject *parent = nullptr);
    ~EmailValidator();

    struct ValidationResult {
        QString email;
        bool isValid;
        bool isDisposable;
        bool isRoleBased;
        bool hasValidFormat;
        bool hasValidDomain;
        QString errorMessage;
        
        ValidationResult() : isValid(false), isDisposable(false), isRoleBased(false), 
                           hasValidFormat(false), hasValidDomain(false) {}
    };

    ValidationResult validateEmail(const QString &email);
    QList<ValidationResult> validateEmails(const QList<QString> &emails);
    
    void setApiKey(const QString &apiKey);
    QString getApiKey() const;

signals:
    void validationCompleted(const ValidationResult &result);
    void validationProgress(int current, int total);

private slots:
    void handleValidationResponse(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QString apiKey;
    QMutex mutex;
    QThreadPool threadPool;
    
    bool isValidFormat(const QString &email);
    bool isValidDomain(const QString &email);
    bool isDisposableEmail(const QString &email);
    bool isRoleBasedEmail(const QString &email);
    void validateEmailOnline(const QString &email);
};

class EmailValidationWorker : public QObject, public QRunnable
{
    Q_OBJECT

public:
    EmailValidationWorker(const QString &email, EmailValidator *validator, QObject *parent = nullptr);
    
    void run() override;

signals:
    void validationCompleted(const EmailValidator::ValidationResult &result);

private:
    QString email;
    EmailValidator *validator;
};

#endif // EMAILVALIDATOR_H