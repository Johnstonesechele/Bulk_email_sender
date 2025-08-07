#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QTimer>
#include <QQueue>
#include <QDateTime>
#include "EmailManager.h"

struct EmailMessage {
    QString to;
    QString toName;
    QString subject;
    QString body;
    QString campaignId;
    QDateTime scheduledTime;
};

class SMTPClient : public QObject
{
    Q_OBJECT

public:
    explicit SMTPClient(QObject *parent = nullptr);
    ~SMTPClient();
    
    // Configuration
    void setServerSettings(const QString& server, int port, 
                          const QString& username, const QString& password);
    void setSendingDelay(int seconds);
    
    // Email sending
    void sendBulkEmails(const QList<Contact>& contacts, 
                       const QString& subject, 
                       const QString& message,
                       const QString& campaignId);
    void sendSingleEmail(const EmailMessage& email);
    
    // Connection testing
    bool testConnection();
    
    // Status
    bool isSending() const { return m_isSending; }
    int getQueueSize() const { return m_emailQueue.size(); }

signals:
    void emailSent(const QString& email, bool success);
    void emailStatusChanged(const QString& email, const QString& status);
    void campaignProgress(int sent, int total);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString& error);

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketReadyRead();
    void onSendNextEmail();

private:
    void connectToServer();
    void disconnectFromServer();
    void sendCommand(const QString& command);
    void processServerResponse(const QString& response);
    void handleAuthenticationResponse(const QString& response);
    void handleMailFromResponse(const QString& response);
    void handleRcptToResponse(const QString& response);
    void handleDataResponse(const QString& response);
    void handleEmailDataResponse(const QString& response);
    void finishCurrentEmail(bool success);
    void processNextEmail();
    
    QString encodeBase64(const QString& text);
    QString formatEmailMessage(const EmailMessage& email);
    
    // Connection settings
    QString m_server;
    int m_port;
    QString m_username;
    QString m_password;
    bool m_useSSL;
    
    // Network
    QTcpSocket *m_socket;
    bool m_connected;
    
    // Email queue and processing
    QQueue<EmailMessage> m_emailQueue;
    EmailMessage m_currentEmail;
    bool m_isSending;
    QTimer *m_sendTimer;
    int m_sendingDelay; // seconds
    
    // Campaign tracking
    QString m_currentCampaignId;
    int m_totalEmailsInCampaign;
    int m_sentEmailsInCampaign;
    
    // SMTP state machine
    enum SMTPState {
        Disconnected,
        Connected,
        Authenticated,
        MailFrom,
        RcptTo,
        Data,
        SendingData,
        Finished
    };
    SMTPState m_smtpState;
    
    // Response tracking
    QString m_responseBuffer;
    bool m_waitingForResponse;
};

#endif // SMTPCLIENT_H