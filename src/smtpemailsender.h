#ifndef SMTPEMAILSENDER_H
#define SMTPEMAILSENDER_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QTimer>
#include <QString>
#include <QQueue>
#include <QDateTime>

enum class SmtpEncryption {
    None,
    SSL,
    TLS,
    STARTTLS
};

enum class SmtpAuthMethod {
    None,
    Plain,
    Login,
    CramMD5
};

struct SmtpConfiguration {
    QString server;
    int port;
    QString username;
    QString password;
    SmtpEncryption encryption;
    SmtpAuthMethod authMethod;
    int timeout; // seconds
    
    bool isValid() const {
        return !server.isEmpty() && port > 0;
    }
};

struct EmailMessage {
    QString from;
    QString fromName;
    QString to;
    QString toName;
    QString subject;
    QString htmlBody;
    QString textBody;
    QString messageId;
    QDateTime timestamp;
    QMap<QString, QString> headers;
    
    QString getId() const { return messageId; }
    bool isValid() const { return !from.isEmpty() && !to.isEmpty() && !subject.isEmpty(); }
};

class SmtpEmailSender : public QObject
{
    Q_OBJECT

public:
    explicit SmtpEmailSender(QObject *parent = nullptr);
    ~SmtpEmailSender();
    
    // Configuration
    void setConfiguration(const SmtpConfiguration &config);
    SmtpConfiguration getConfiguration() const { return smtpConfig; }
    
    // Connection management
    bool connectToServer();
    void disconnectFromServer();
    bool isConnected() const;
    bool testConnection();
    
    // Email sending
    bool sendEmail(const EmailMessage &message);
    bool sendEmails(const QList<EmailMessage> &messages);
    void addToQueue(const EmailMessage &message);
    void clearQueue();
    
    // Queue management
    void startQueue();
    void stopQueue();
    void pauseQueue();
    void resumeQueue();
    bool isQueueRunning() const;
    int getQueueSize() const;
    int getQueueProgress() const;
    
    // Error handling
    QString getLastError() const { return lastError; }
    QStringList getErrorLog() const { return errorLog; }
    
    // Statistics
    int getSentCount() const { return sentCount; }
    int getFailedCount() const { return failedCount; }
    void resetCounters();

signals:
    void connected();
    void disconnected();
    void emailSent(const QString &messageId, const QString &recipient);
    void emailFailed(const QString &messageId, const QString &recipient, const QString &error);
    void queueProgress(int current, int total);
    void queueCompleted(int sent, int failed);
    void errorOccurred(const QString &error);
    void statusChanged(const QString &status);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);
    void onSocketReadyRead();
    void onConnectionTimeout();
    void processNextEmail();

private:
    SmtpConfiguration smtpConfig;
    QSslSocket *socket;
    QTimer *connectionTimer;
    QTimer *queueTimer;
    
    // Queue management
    QQueue<EmailMessage> emailQueue;
    bool queueRunning;
    bool queuePaused;
    int currentQueueIndex;
    EmailMessage currentMessage;
    
    // Connection state
    enum SmtpState {
        Disconnected,
        Connecting,
        Connected,
        Authenticating,
        Ready,
        Sending,
        Error
    };
    SmtpState currentState;
    
    // SMTP protocol
    QString lastResponse;
    QString lastError;
    QStringList errorLog;
    int sentCount;
    int failedCount;
    
    // SMTP protocol methods
    bool sendCommand(const QString &command);
    bool waitForResponse(int expectedCode, int timeoutMs = 30000);
    bool authenticate();
    bool sendSingleEmail(const EmailMessage &message);
    
    // Message formatting
    QString formatEmailMessage(const EmailMessage &message);
    QString encodeBase64(const QString &text);
    QString formatAddress(const QString &email, const QString &name);
    QString generateMessageId();
    QString formatDateTime(const QDateTime &dateTime);
    
    // Utility methods
    void setState(SmtpState state);
    void logError(const QString &error);
    void resetConnection();
    int getExpectedResponseCode() const;
};

#endif // SMTPEMAILSENDER_H
