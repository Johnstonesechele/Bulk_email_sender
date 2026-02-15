#ifndef EMAILVALIDATOR_H
#define EMAILVALIDATOR_H

#include <QObject>
#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class EmailValidator : public QObject
{
    Q_OBJECT

public:
    explicit EmailValidator(QObject *parent = nullptr);
    ~EmailValidator();

    // Validation methods
    bool isValidEmailFormat(const QString &email);
    bool isValidDomain(const QString &email);
    bool hasValidMXRecord(const QString &email);
    bool isDisposableEmail(const QString &email);
    bool isRoleBasedEmail(const QString &email);
    
    // Batch validation
    QList<QString> validateEmailList(const QList<QString> &emails);
    QMap<QString, QString> validateEmailsWithDetails(const QList<QString> &emails);
    
    // Email cleaning
    QString cleanEmail(const QString &email);
    QList<QString> cleanEmailList(const QList<QString> &emails);
    
    // Statistics
    struct ValidationStats {
        int total;
        int valid;
        int invalid;
        int disposable;
        int roleBased;
        int duplicate;
        int malformed;
    };
    
    ValidationStats getValidationStats(const QList<QString> &emails);

signals:
    void validationProgress(int current, int total);
    void validationCompleted(const QList<QString> &validEmails, const QList<QString> &invalidEmails);
    void errorOccurred(const QString &error);

private slots:
    void onNetworkReplyFinished();

private:
    QNetworkAccessManager *networkManager;
    QList<QString> disposableDomains;
    QList<QString> roleBasedPrefixes;
    
    void loadDisposableDomains();
    void loadRoleBasedPrefixes();
    bool checkMXRecord(const QString &domain);
    QString extractDomain(const QString &email);
    bool isDuplicateEmail(const QString &email, const QList<QString> &emailList);
};

#endif // EMAILVALIDATOR_H