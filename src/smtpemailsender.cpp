#include "smtpemailsender.h"
#include <QDebug>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QUuid>
#include <QRegularExpression>
#include <QEventLoop>
#include <QHostAddress>

SmtpEmailSender::SmtpEmailSender(QObject *parent)
    : QObject(parent)
    , socket(nullptr)
    , connectionTimer(new QTimer(this))
    , queueTimer(new QTimer(this))
    , queueRunning(false)
    , queuePaused(false)
    , currentQueueIndex(0)
    , currentState(Disconnected)
    , sentCount(0)
    , failedCount(0)
{
    // Setup connection timer
    connectionTimer->setSingleShot(true);
    connectionTimer->setInterval(30000); // 30 seconds timeout
    connect(connectionTimer, &QTimer::timeout, this, &SmtpEmailSender::onConnectionTimeout);
    
    // Setup queue timer
    queueTimer->setSingleShot(true);
    queueTimer->setInterval(1000); // 1 second delay between emails
    connect(queueTimer, &QTimer::timeout, this, &SmtpEmailSender::processNextEmail);
    
    // Initialize SSL socket
    socket = new QSslSocket(this);
    connect(socket, &QSslSocket::connected, this, &SmtpEmailSender::onSocketConnected);
    connect(socket, &QSslSocket::disconnected, this, &SmtpEmailSender::onSocketDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::error), 
            this, &SmtpEmailSender::onSocketError);
    connect(socket, &QSslSocket::sslErrors, this, &SmtpEmailSender::onSslErrors);
    connect(socket, &QSslSocket::readyRead, this, &SmtpEmailSender::onSocketReadyRead);
}

SmtpEmailSender::~SmtpEmailSender()
{
    disconnectFromServer();
}

void SmtpEmailSender::setConfiguration(const SmtpConfiguration &config)
{
    smtpConfig = config;
    if (socket && socket->state() != QAbstractSocket::UnconnectedState) {
        disconnectFromServer();
    }
}

bool SmtpEmailSender::connectToServer()
{
    if (!smtpConfig.isValid()) {
        logError("Invalid SMTP configuration");
        return false;
    }
    
    if (isConnected()) {
        return true;
    }
    
    setState(Connecting);
    emit statusChanged("Connecting to SMTP server...");
    
    connectionTimer->start();
    
    // Connect based on encryption type
    switch (smtpConfig.encryption) {
        case SmtpEncryption::SSL:
            socket->connectToHostEncrypted(smtpConfig.server, smtpConfig.port);
            break;
        case SmtpEncryption::TLS:
            socket->connectToHostEncrypted(smtpConfig.server, smtpConfig.port);
            break;
        default:
            socket->connectToHost(smtpConfig.server, smtpConfig.port);
            break;
    }
    
    return true;
}

void SmtpEmailSender::disconnectFromServer()
{
    if (socket) {
        connectionTimer->stop();
        queueTimer->stop();
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(3000);
        }
    }
    setState(Disconnected);
}

bool SmtpEmailSender::isConnected() const
{
    return socket && socket->state() == QAbstractSocket::ConnectedState && 
           currentState == Ready;
}

bool SmtpEmailSender::testConnection()
{
    if (!connectToServer()) {
        return false;
    }
    
    // Wait for connection to establish
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(10000); // 10 seconds
    
    connect(this, &SmtpEmailSender::connected, &loop, &QEventLoop::quit);
    connect(this, &SmtpEmailSender::errorOccurred, &loop, &QEventLoop::quit);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timeout.start();
    loop.exec();
    
    bool success = isConnected();
    disconnectFromServer();
    return success;
}

bool SmtpEmailSender::sendEmail(const EmailMessage &message)
{
    if (!message.isValid()) {
        logError("Invalid email message");
        return false;
    }
    
    if (!isConnected() && !connectToServer()) {
        return false;
    }
    
    return sendSingleEmail(message);
}

bool SmtpEmailSender::sendEmails(const QList<EmailMessage> &messages)
{
    clearQueue();
    for (const auto &message : messages) {
        addToQueue(message);
    }
    startQueue();
    return true;
}

void SmtpEmailSender::addToQueue(const EmailMessage &message)
{
    if (message.isValid()) {
        emailQueue.enqueue(message);
        emit statusChanged(QString("Added email to queue. Queue size: %1").arg(emailQueue.size()));
    }
}

void SmtpEmailSender::clearQueue()
{
    emailQueue.clear();
    currentQueueIndex = 0;
    emit statusChanged("Queue cleared");
}

void SmtpEmailSender::startQueue()
{
    if (emailQueue.isEmpty()) {
        emit statusChanged("No emails in queue");
        return;
    }
    
    queueRunning = true;
    queuePaused = false;
    currentQueueIndex = 0;
    
    emit statusChanged("Starting email queue...");
    
    if (!isConnected() && !connectToServer()) {
        stopQueue();
        return;
    }
    
    processNextEmail();
}

void SmtpEmailSender::stopQueue()
{
    queueRunning = false;
    queuePaused = false;
    queueTimer->stop();
    
    if (!emailQueue.isEmpty()) {
        emit queueCompleted(sentCount, failedCount);
    }
    
    emit statusChanged("Queue stopped");
}

void SmtpEmailSender::pauseQueue()
{
    queuePaused = true;
    queueTimer->stop();
    emit statusChanged("Queue paused");
}

void SmtpEmailSender::resumeQueue()
{
    if (queueRunning && queuePaused) {
        queuePaused = false;
        emit statusChanged("Queue resumed");
        queueTimer->start();
    }
}

bool SmtpEmailSender::isQueueRunning() const
{
    return queueRunning && !queuePaused;
}

int SmtpEmailSender::getQueueSize() const
{
    return emailQueue.size();
}

int SmtpEmailSender::getQueueProgress() const
{
    return currentQueueIndex;
}

void SmtpEmailSender::resetCounters()
{
    sentCount = 0;
    failedCount = 0;
}

void SmtpEmailSender::onSocketConnected()
{
    connectionTimer->stop();
    setState(Connected);
    emit statusChanged("Connected to SMTP server");
    
    // Wait for server greeting
    if (!waitForResponse(220)) {
        logError("Failed to receive server greeting");
        setState(Error);
        return;
    }
    
    // Send EHLO command
    QString hostname = QHostAddress(QHostAddress::LocalHost).toString();
    if (!sendCommand(QString("EHLO %1").arg(hostname))) {
        logError("Failed to send EHLO command");
        setState(Error);
        return;
    }
    
    if (!waitForResponse(250)) {
        logError("Server rejected EHLO command");
        setState(Error);
        return;
    }
    
    // Handle STARTTLS if required
    if (smtpConfig.encryption == SmtpEncryption::STARTTLS) {
        if (!sendCommand("STARTTLS")) {
            logError("Failed to send STARTTLS command");
            setState(Error);
            return;
        }
        
        if (!waitForResponse(220)) {
            logError("Server rejected STARTTLS command");
            setState(Error);
            return;
        }
        
        socket->startClientEncryption();
        if (!socket->waitForEncrypted(10000)) {
            logError("Failed to establish TLS connection");
            setState(Error);
            return;
        }
        
        // Send EHLO again after TLS
        if (!sendCommand(QString("EHLO %1").arg(hostname))) {
            logError("Failed to send EHLO command after TLS");
            setState(Error);
            return;
        }
        
        if (!waitForResponse(250)) {
            logError("Server rejected EHLO command after TLS");
            setState(Error);
            return;
        }
    }
    
    // Authenticate if required
    if (!smtpConfig.username.isEmpty()) {
        setState(Authenticating);
        if (!authenticate()) {
            setState(Error);
            return;
        }
    }
    
    setState(Ready);
    emit connected();
}

void SmtpEmailSender::onSocketDisconnected()
{
    connectionTimer->stop();
    queueTimer->stop();
    setState(Disconnected);
    emit disconnected();
    emit statusChanged("Disconnected from SMTP server");
}

void SmtpEmailSender::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorString = socket->errorString();
    logError(QString("Socket error: %1").arg(errorString));
    setState(Error);
    emit errorOccurred(errorString);
}

void SmtpEmailSender::onSslErrors(const QList<QSslError> &errors)
{
    QStringList errorStrings;
    for (const auto &error : errors) {
        errorStrings << error.errorString();
    }
    QString errorString = errorStrings.join(", ");
    logError(QString("SSL errors: %1").arg(errorString));
    
    // For production use, you might want to be more strict about SSL errors
    socket->ignoreSslErrors();
}

void SmtpEmailSender::onSocketReadyRead()
{
    while (socket->canReadLine()) {
        QString line = QString::fromLatin1(socket->readLine()).trimmed();
        lastResponse = line;
        qDebug() << "SMTP Response:" << line;
    }
}

void SmtpEmailSender::onConnectionTimeout()
{
    logError("Connection timeout");
    setState(Error);
    disconnectFromServer();
}

void SmtpEmailSender::processNextEmail()
{
    if (!queueRunning || queuePaused || emailQueue.isEmpty()) {
        return;
    }
    
    if (currentQueueIndex >= emailQueue.size()) {
        // Queue complete
        stopQueue();
        return;
    }
    
    currentMessage = emailQueue.at(currentQueueIndex);
    emit queueProgress(currentQueueIndex + 1, emailQueue.size());
    emit statusChanged(QString("Sending email %1 of %2 to %3")
                      .arg(currentQueueIndex + 1)
                      .arg(emailQueue.size())
                      .arg(currentMessage.to));
    
    if (sendSingleEmail(currentMessage)) {
        sentCount++;
        emit emailSent(currentMessage.messageId, currentMessage.to);
    } else {
        failedCount++;
        emit emailFailed(currentMessage.messageId, currentMessage.to, lastError);
    }
    
    currentQueueIndex++;
    
    if (currentQueueIndex >= emailQueue.size()) {
        // All emails processed
        emit queueCompleted(sentCount, failedCount);
        stopQueue();
    } else {
        // Schedule next email
        queueTimer->start();
    }
}

bool SmtpEmailSender::sendCommand(const QString &command)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        logError("Socket not connected");
        return false;
    }
    
    QString fullCommand = command + "\r\n";
    qDebug() << "SMTP Command:" << command;
    
    qint64 bytesWritten = socket->write(fullCommand.toLatin1());
    if (bytesWritten == -1) {
        logError("Failed to write command to socket");
        return false;
    }
    
    return socket->waitForBytesWritten(5000);
}

bool SmtpEmailSender::waitForResponse(int expectedCode, int timeoutMs)
{
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(timeoutMs);
    
    QEventLoop loop;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(socket, &QSslSocket::readyRead, &loop, &QEventLoop::quit);
    
    timer.start();
    
    while (timer.isActive() && socket->canReadLine() == false) {
        loop.processEvents(QEventLoop::WaitForMoreEvents, 100);
    }
    
    if (!socket->canReadLine()) {
        logError("Timeout waiting for server response");
        return false;
    }
    
    onSocketReadyRead();
    
    // Parse response code
    if (lastResponse.length() < 3) {
        logError("Invalid server response format");
        return false;
    }
    
    bool ok;
    int responseCode = lastResponse.left(3).toInt(&ok);
    if (!ok) {
        logError("Failed to parse response code");
        return false;
    }
    
    if (responseCode != expectedCode && expectedCode != 0) {
        logError(QString("Unexpected response code: %1 (expected %2)")
                .arg(responseCode).arg(expectedCode));
        return false;
    }
    
    return true;
}

bool SmtpEmailSender::authenticate()
{
    if (smtpConfig.authMethod == SmtpAuthMethod::Plain) {
        QString auth = QString("\0%1\0%2").arg(smtpConfig.username, smtpConfig.password);
        QString encodedAuth = auth.toLatin1().toBase64();
        
        if (!sendCommand("AUTH PLAIN " + encodedAuth)) {
            logError("Failed to send AUTH PLAIN command");
            return false;
        }
        
        return waitForResponse(235);
    } else if (smtpConfig.authMethod == SmtpAuthMethod::Login) {
        if (!sendCommand("AUTH LOGIN")) {
            logError("Failed to send AUTH LOGIN command");
            return false;
        }
        
        if (!waitForResponse(334)) {
            logError("Server rejected AUTH LOGIN");
            return false;
        }
        
        QString encodedUsername = smtpConfig.username.toLatin1().toBase64();
        if (!sendCommand(encodedUsername)) {
            logError("Failed to send username");
            return false;
        }
        
        if (!waitForResponse(334)) {
            logError("Server rejected username");
            return false;
        }
        
        QString encodedPassword = smtpConfig.password.toLatin1().toBase64();
        if (!sendCommand(encodedPassword)) {
            logError("Failed to send password");
            return false;
        }
        
        return waitForResponse(235);
    }
    
    logError("Unsupported authentication method");
    return false;
}

bool SmtpEmailSender::sendSingleEmail(const EmailMessage &message)
{
    if (!isConnected()) {
        logError("Not connected to SMTP server");
        return false;
    }
    
    setState(Sending);
    
    // Send MAIL FROM
    if (!sendCommand(QString("MAIL FROM:<%1>").arg(message.from))) {
        logError("Failed to send MAIL FROM command");
        return false;
    }
    
    if (!waitForResponse(250)) {
        logError("Server rejected MAIL FROM command");
        return false;
    }
    
    // Send RCPT TO
    if (!sendCommand(QString("RCPT TO:<%1>").arg(message.to))) {
        logError("Failed to send RCPT TO command");
        return false;
    }
    
    if (!waitForResponse(250)) {
        logError("Server rejected RCPT TO command");
        return false;
    }
    
    // Send DATA
    if (!sendCommand("DATA")) {
        logError("Failed to send DATA command");
        return false;
    }
    
    if (!waitForResponse(354)) {
        logError("Server rejected DATA command");
        return false;
    }
    
    // Send message content
    QString messageData = formatEmailMessage(message);
    if (!sendCommand(messageData)) {
        logError("Failed to send message data");
        return false;
    }
    
    // End data with single dot
    if (!sendCommand(".")) {
        logError("Failed to send end-of-data marker");
        return false;
    }
    
    if (!waitForResponse(250)) {
        logError("Server rejected message data");
        return false;
    }
    
    setState(Ready);
    return true;
}

QString SmtpEmailSender::formatEmailMessage(const EmailMessage &message)
{
    QStringList lines;
    
    // Headers
    lines << QString("From: %1").arg(formatAddress(message.from, message.fromName));
    lines << QString("To: %1").arg(formatAddress(message.to, message.toName));
    lines << QString("Subject: %1").arg(message.subject);
    lines << QString("Date: %1").arg(formatDateTime(message.timestamp.isValid() ? 
                                                   message.timestamp : QDateTime::currentDateTime()));
    lines << QString("Message-ID: %1").arg(message.messageId.isEmpty() ? 
                                          generateMessageId() : message.messageId);
    lines << "MIME-Version: 1.0";
    
    // Custom headers
    for (auto it = message.headers.begin(); it != message.headers.end(); ++it) {
        lines << QString("%1: %2").arg(it.key(), it.value());
    }
    
    // Content type
    if (!message.htmlBody.isEmpty() && !message.textBody.isEmpty()) {
        // Multipart message
        QString boundary = "----=_Part_" + QUuid::createUuid().toString().remove('{').remove('}').remove('-');
        lines << QString("Content-Type: multipart/alternative; boundary=\"%1\"").arg(boundary);
        lines << "";
        lines << QString("This is a multi-part message in MIME format.");
        lines << "";
        
        // Text part
        lines << QString("--%1").arg(boundary);
        lines << "Content-Type: text/plain; charset=UTF-8";
        lines << "Content-Transfer-Encoding: quoted-printable";
        lines << "";
        lines << message.textBody;
        lines << "";
        
        // HTML part
        lines << QString("--%1").arg(boundary);
        lines << "Content-Type: text/html; charset=UTF-8";
        lines << "Content-Transfer-Encoding: quoted-printable";
        lines << "";
        lines << message.htmlBody;
        lines << "";
        lines << QString("--%1--").arg(boundary);
    } else if (!message.htmlBody.isEmpty()) {
        // HTML only
        lines << "Content-Type: text/html; charset=UTF-8";
        lines << "Content-Transfer-Encoding: quoted-printable";
        lines << "";
        lines << message.htmlBody;
    } else {
        // Text only
        lines << "Content-Type: text/plain; charset=UTF-8";
        lines << "Content-Transfer-Encoding: quoted-printable";
        lines << "";
        lines << message.textBody;
    }
    
    return lines.join("\r\n");
}

QString SmtpEmailSender::encodeBase64(const QString &text)
{
    return text.toLatin1().toBase64();
}

QString SmtpEmailSender::formatAddress(const QString &email, const QString &name)
{
    if (name.isEmpty()) {
        return email;
    }
    return QString("\"%1\" <%2>").arg(name, email);
}

QString SmtpEmailSender::generateMessageId()
{
    QString uuid = QUuid::createUuid().toString().remove('{').remove('}');
    QString domain = smtpConfig.server.isEmpty() ? "localhost" : smtpConfig.server;
    return QString("<%1@%2>").arg(uuid, domain);
}

QString SmtpEmailSender::formatDateTime(const QDateTime &dateTime)
{
    return dateTime.toString("ddd, dd MMM yyyy hh:mm:ss +0000");
}

void SmtpEmailSender::setState(SmtpState state)
{
    if (currentState != state) {
        currentState = state;
        QString statusText;
        switch (state) {
            case Disconnected: statusText = "Disconnected"; break;
            case Connecting: statusText = "Connecting"; break;
            case Connected: statusText = "Connected"; break;
            case Authenticating: statusText = "Authenticating"; break;
            case Ready: statusText = "Ready"; break;
            case Sending: statusText = "Sending"; break;
            case Error: statusText = "Error"; break;
        }
        emit statusChanged(statusText);
    }
}

void SmtpEmailSender::logError(const QString &error)
{
    lastError = error;
    errorLog.append(QString("[%1] %2").arg(QDateTime::currentDateTime().toString(), error));
    if (errorLog.size() > 100) {
        errorLog.removeFirst();
    }
    emit errorOccurred(error);
    qDebug() << "SMTP Error:" << error;
}

void SmtpEmailSender::resetConnection()
{
    disconnectFromServer();
    QTimer::singleShot(1000, this, &SmtpEmailSender::connectToServer);
}

int SmtpEmailSender::getExpectedResponseCode() const
{
    switch (currentState) {
        case Connected: return 220;
        case Authenticating: return 235;
        case Sending: return 250;
        default: return 0;
    }
}
