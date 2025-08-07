#include "SMTPClient.h"
#include <QDebug>
#include <QTextStream>
#include <QCryptographicHash>
#include <QSslSocket>
#include <QRandomGenerator>

SMTPClient::SMTPClient(QObject *parent)
    : QObject(parent)
    , m_server("smtp.gmail.com")
    , m_port(587)
    , m_useSSL(true)
    , m_socket(nullptr)
    , m_connected(false)
    , m_isSending(false)
    , m_sendingDelay(5)
    , m_totalEmailsInCampaign(0)
    , m_sentEmailsInCampaign(0)
    , m_smtpState(Disconnected)
    , m_waitingForResponse(false)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &SMTPClient::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &SMTPClient::onSocketDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &SMTPClient::onSocketError);
    connect(m_socket, &QTcpSocket::readyRead, this, &SMTPClient::onSocketReadyRead);
    
    m_sendTimer = new QTimer(this);
    m_sendTimer->setSingleShot(true);
    connect(m_sendTimer, &QTimer::timeout, this, &SMTPClient::onSendNextEmail);
}

SMTPClient::~SMTPClient()
{
    disconnectFromServer();
}

void SMTPClient::setServerSettings(const QString& server, int port, 
                                  const QString& username, const QString& password)
{
    m_server = server;
    m_port = port;
    m_username = username;
    m_password = password;
    m_useSSL = (port == 587 || port == 465);
}

void SMTPClient::setSendingDelay(int seconds)
{
    m_sendingDelay = qMax(1, seconds);
}

void SMTPClient::sendBulkEmails(const QList<Contact>& contacts, 
                               const QString& subject, 
                               const QString& message,
                               const QString& campaignId)
{
    if (m_isSending) {
        emit errorOccurred("Already sending emails. Please wait for current campaign to finish.");
        return;
    }
    
    if (contacts.isEmpty()) {
        emit errorOccurred("No contacts provided for bulk sending.");
        return;
    }
    
    // Clear previous queue
    m_emailQueue.clear();
    
    // Setup campaign tracking
    m_currentCampaignId = campaignId;
    m_totalEmailsInCampaign = contacts.size();
    m_sentEmailsInCampaign = 0;
    
    // Create email messages for each contact
    for (const auto& contact : contacts) {
        EmailMessage email;
        email.to = contact.email;
        email.toName = contact.name;
        email.subject = subject;
        email.body = message;
        email.campaignId = campaignId;
        email.scheduledTime = QDateTime::currentDateTime();
        
        m_emailQueue.enqueue(email);
    }
    
    qDebug() << "Queued" << m_emailQueue.size() << "emails for sending";
    
    // Start sending process
    m_isSending = true;
    processNextEmail();
}

void SMTPClient::sendSingleEmail(const EmailMessage& email)
{
    m_emailQueue.enqueue(email);
    if (!m_isSending) {
        m_isSending = true;
        processNextEmail();
    }
}

bool SMTPClient::testConnection()
{
    if (m_username.isEmpty() || m_password.isEmpty()) {
        emit errorOccurred("Username and password are required for connection test.");
        return false;
    }
    
    // For demo purposes, we'll simulate a successful connection test
    // In a real implementation, you would actually connect to the SMTP server
    emit connectionStatusChanged(true);
    return true;
}

void SMTPClient::onSocketConnected()
{
    qDebug() << "Connected to SMTP server";
    m_connected = true;
    m_smtpState = Connected;
    emit connectionStatusChanged(true);
}

void SMTPClient::onSocketDisconnected()
{
    qDebug() << "Disconnected from SMTP server";
    m_connected = false;
    m_smtpState = Disconnected;
    emit connectionStatusChanged(false);
}

void SMTPClient::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorMsg = QString("Socket error: %1").arg(m_socket->errorString());
    qDebug() << errorMsg;
    emit errorOccurred(errorMsg);
    
    // Mark current email as failed if we were sending
    if (m_isSending && !m_currentEmail.to.isEmpty()) {
        finishCurrentEmail(false);
    }
}

void SMTPClient::onSocketReadyRead()
{
    QByteArray data = m_socket->readAll();
    m_responseBuffer.append(QString::fromUtf8(data));
    
    // Process complete lines
    while (m_responseBuffer.contains("\r\n")) {
        int pos = m_responseBuffer.indexOf("\r\n");
        QString response = m_responseBuffer.left(pos);
        m_responseBuffer.remove(0, pos + 2);
        
        processServerResponse(response);
    }
}

void SMTPClient::onSendNextEmail()
{
    processNextEmail();
}

void SMTPClient::connectToServer()
{
    if (m_connected) {
        return;
    }
    
    qDebug() << "Connecting to" << m_server << ":" << m_port;
    m_socket->connectToHost(m_server, m_port);
}

void SMTPClient::disconnectFromServer()
{
    if (m_socket && m_connected) {
        m_socket->disconnectFromHost();
    }
}

void SMTPClient::sendCommand(const QString& command)
{
    if (!m_connected) {
        return;
    }
    
    QString cmd = command + "\r\n";
    m_socket->write(cmd.toUtf8());
    qDebug() << "SENT:" << command;
}

void SMTPClient::processServerResponse(const QString& response)
{
    qDebug() << "RECV:" << response;
    
    // For demo purposes, we'll simulate SMTP responses
    // In a real implementation, you would parse actual SMTP response codes
    
    switch (m_smtpState) {
        case Connected:
            handleAuthenticationResponse(response);
            break;
        case Authenticated:
            handleMailFromResponse(response);
            break;
        case MailFrom:
            handleRcptToResponse(response);
            break;
        case RcptTo:
            handleDataResponse(response);
            break;
        case Data:
            handleEmailDataResponse(response);
            break;
        default:
            break;
    }
}

void SMTPClient::handleAuthenticationResponse(const QString& response)
{
    // Simulate successful authentication
    m_smtpState = Authenticated;
    
    // Start sending the current email
    if (!m_currentEmail.to.isEmpty()) {
        sendCommand(QString("MAIL FROM:<%1>").arg(m_username));
        m_smtpState = MailFrom;
    }
}

void SMTPClient::handleMailFromResponse(const QString& response)
{
    // Simulate successful MAIL FROM
    sendCommand(QString("RCPT TO:<%1>").arg(m_currentEmail.to));
    m_smtpState = RcptTo;
}

void SMTPClient::handleRcptToResponse(const QString& response)
{
    // Simulate successful RCPT TO
    sendCommand("DATA");
    m_smtpState = Data;
}

void SMTPClient::handleDataResponse(const QString& response)
{
    // Send the actual email content
    QString emailContent = formatEmailMessage(m_currentEmail);
    m_socket->write(emailContent.toUtf8());
    m_socket->write("\r\n.\r\n"); // End of message
    m_smtpState = SendingData;
}

void SMTPClient::handleEmailDataResponse(const QString& response)
{
    // Email sent successfully
    finishCurrentEmail(true);
}

void SMTPClient::finishCurrentEmail(bool success)
{
    if (m_currentEmail.to.isEmpty()) {
        return;
    }
    
    QString status = success ? "delivered" : "bounced";
    
    emit emailSent(m_currentEmail.to, success);
    emit emailStatusChanged(m_currentEmail.to, status);
    
    if (success) {
        m_sentEmailsInCampaign++;
        qDebug() << "Email sent successfully to" << m_currentEmail.to;
    } else {
        qDebug() << "Failed to send email to" << m_currentEmail.to;
    }
    
    emit campaignProgress(m_sentEmailsInCampaign, m_totalEmailsInCampaign);
    
    // Clear current email
    m_currentEmail = EmailMessage();
    m_smtpState = Authenticated;
    
    // Schedule next email or finish campaign
    if (!m_emailQueue.isEmpty()) {
        m_sendTimer->start(m_sendingDelay * 1000);
    } else {
        // Campaign finished
        m_isSending = false;
        disconnectFromServer();
        qDebug() << "Bulk email campaign completed. Sent:" << m_sentEmailsInCampaign 
                 << "out of" << m_totalEmailsInCampaign;
    }
}

void SMTPClient::processNextEmail()
{
    if (m_emailQueue.isEmpty()) {
        m_isSending = false;
        return;
    }
    
    m_currentEmail = m_emailQueue.dequeue();
    
    // For demo purposes, we'll simulate the sending process
    // In a real implementation, you would connect to SMTP server and send
    
    qDebug() << "Processing email to" << m_currentEmail.to;
    
    // Simulate network delay and then mark as sent
    QTimer::singleShot(1000, this, [this]() {
        // Simulate 90% success rate
        bool success = (QRandomGenerator::global()->bounded(100) < 90);
        finishCurrentEmail(success);
    });
}

QString SMTPClient::encodeBase64(const QString& text)
{
    return text.toUtf8().toBase64();
}

QString SMTPClient::formatEmailMessage(const EmailMessage& email)
{
    QString message;
    QTextStream stream(&message);
    
    // Email headers
    stream << "From: " << m_username << "\r\n";
    stream << "To: " << email.toName << " <" << email.to << ">\r\n";
    stream << "Subject: " << email.subject << "\r\n";
    stream << "Date: " << QDateTime::currentDateTime().toString(Qt::RFC2822Date) << "\r\n";
    stream << "MIME-Version: 1.0\r\n";
    stream << "Content-Type: text/plain; charset=utf-8\r\n";
    stream << "Content-Transfer-Encoding: 8bit\r\n";
    stream << "\r\n";
    
    // Email body
    stream << email.body << "\r\n";
    
    return message;
}

#include "SMTPClient.moc"