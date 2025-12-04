#include "mainwindow.h"
#include "templateeditor.h"
#include <QHeaderView>
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QDateTime>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , emailManager(new EmailManager(this))
    , database(new Database(this))
    , templateManager(new TemplateManager(this))
    , csvReader(new CsvReader(this))
    , smtpSender(new SmtpEmailSender(this))
    , statusTimer(new QTimer(this))
{
    setupUI();
    setupConnections();
    updateTemplateCombo();
    loadCampaignHistory();
    
    // Center the window on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Set window title and icon
    setWindowTitle("Bulk Email Manager - Professional Edition");
    setWindowIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    
    // Initialize status timer
    statusTimer->setSingleShot(true);
    connect(statusTimer, &QTimer::timeout, [this]() {
        statusLabel->clear();
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Create tab widget
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    
    // Email Management Tab
    setupEmailTab();
    
    // Campaign History Tab
    setupHistoryTab();
    
    // Data Cleaning Tab
    setupCleaningTab();
    
    // Status bar
    statusLabel = new QLabel("Ready");
    statusBar()->addWidget(statusLabel);
    
    // Set window size
    resize(1200, 800);
}

void MainWindow::setupEmailTab()
{
    emailTab = new QWidget();
    QVBoxLayout *emailLayout = new QVBoxLayout(emailTab);
    
    // Email configuration group
    QGroupBox *configGroup = new QGroupBox("Email Configuration");
    QGridLayout *configLayout = new QGridLayout(configGroup);
    
    configLayout->addWidget(new QLabel("Sender Name:"), 0, 0);
    senderName = new QLineEdit();
    configLayout->addWidget(senderName, 0, 1);
    
    configLayout->addWidget(new QLabel("Sender Email:"), 0, 2);
    senderEmail = new QLineEdit();
    configLayout->addWidget(senderEmail, 0, 3);
    
    configLayout->addWidget(new QLabel("SMTP Server:"), 1, 0);
    smtpServer = new QLineEdit();
    smtpServer->setText("smtp.gmail.com");
    configLayout->addWidget(smtpServer, 1, 1);
    
    configLayout->addWidget(new QLabel("SMTP Port:"), 1, 2);
    smtpPort = new QLineEdit();
    smtpPort->setText("587");
    configLayout->addWidget(smtpPort, 1, 3);
    
    configLayout->addWidget(new QLabel("Username:"), 2, 0);
    smtpUsername = new QLineEdit();
    configLayout->addWidget(smtpUsername, 2, 1);
    
    configLayout->addWidget(new QLabel("Password:"), 2, 2);
    smtpPassword = new QLineEdit();
    smtpPassword->setEchoMode(QLineEdit::Password);
    configLayout->addWidget(smtpPassword, 2, 3);
    
    emailLayout->addWidget(configGroup);
    
    // Template group
    QGroupBox *templateGroup = new QGroupBox("Email Templates");
    QHBoxLayout *templateLayout = new QHBoxLayout(templateGroup);
    
    templateLayout->addWidget(new QLabel("Template:"));
    templateCombo = new QComboBox();
    templateCombo->setMinimumWidth(200);
    templateLayout->addWidget(templateCombo);
    
    newTemplateButton = new QPushButton("New Template");
    editTemplateButton = new QPushButton("Edit Template");
    deleteTemplateButton = new QPushButton("Delete Template");
    
    templateLayout->addWidget(newTemplateButton);
    templateLayout->addWidget(editTemplateButton);
    templateLayout->addWidget(deleteTemplateButton);
    templateLayout->addStretch();
    
    emailLayout->addWidget(templateGroup);
    
    // Email content group
    QGroupBox *contentGroup = new QGroupBox("Email Content");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentGroup);
    
    contentLayout->addWidget(new QLabel("Subject:"));
    subjectLine = new QLineEdit();
    contentLayout->addWidget(subjectLine);
    
    contentLayout->addWidget(new QLabel("Message:"));
    emailContent = new QTextEdit();
    emailContent->setPlaceholderText("Enter your email message here...\nYou can use HTML tags for formatting.");
    emailContent->setAcceptRichText(true);
    emailContent->setAutoFormatting(QTextEdit::AutoAll);
    
    // Add HTML/Text mode toggle
    QHBoxLayout *editorControls = new QHBoxLayout();
    QPushButton *htmlModeButton = new QPushButton("HTML Mode");
    QPushButton *textModeButton = new QPushButton("Rich Text Mode");
    QPushButton *previewButton = new QPushButton("Preview");
    
    htmlModeButton->setCheckable(true);
    textModeButton->setCheckable(true);
    textModeButton->setChecked(true);
    
    // Store the current editing mode
    bool isHtmlMode = false;
    
    connect(htmlModeButton, &QPushButton::clicked, [this, htmlModeButton, textModeButton, &isHtmlMode]() {
        htmlModeButton->setChecked(true);
        textModeButton->setChecked(false);
        isHtmlMode = true;
        
        // Switch to HTML source editing mode
        QString currentHtml = emailContent->toHtml();
        emailContent->clear();
        emailContent->setPlainText(currentHtml);
        emailContent->setPlaceholderText("Enter HTML code here...\n\nExample:\n<h1>Welcome!</h1>\n<p>Hello {{name}},</p>\n<p>Thank you for joining us!</p>");
    });
    
    connect(textModeButton, &QPushButton::clicked, [this, htmlModeButton, textModeButton, &isHtmlMode]() {
        htmlModeButton->setChecked(false);
        textModeButton->setChecked(true);
        isHtmlMode = false;
        
        // Switch back to rich text editing mode
        if (!emailContent->toPlainText().isEmpty()) {
            QString htmlContent = emailContent->toPlainText();
            emailContent->clear();
            emailContent->setHtml(htmlContent);
        }
        emailContent->setPlaceholderText("Enter your email message here...\nYou can use formatting buttons or type HTML tags.");
    });
    
    connect(previewButton, &QPushButton::clicked, [this]() {
        // Show HTML preview in a dialog
        QDialog *previewDialog = new QDialog(this);
        previewDialog->setWindowTitle("Email Preview");
        previewDialog->resize(800, 600);
        
        QVBoxLayout *layout = new QVBoxLayout(previewDialog);
        
        // Add subject preview
        QLabel *subjectLabel = new QLabel(QString("<b>Subject:</b> %1").arg(subjectLine->text()));
        layout->addWidget(subjectLabel);
        
        // Create preview area with proper HTML rendering
        QTextEdit *previewEdit = new QTextEdit();
        previewEdit->setReadOnly(true);
        
        // Get the content - check if it's HTML or plain text
        QString content = emailContent->toPlainText();
        
        // Check if we have a template selected
        QString templateName = templateCombo->currentText();
        if (templateName != "None" && templateManager) {
            EmailTemplate* tmpl = templateManager->getTemplate(templateName);
            if (tmpl && !tmpl->getName().isEmpty()) {
                content = tmpl->getHtmlContent();
                
                // Apply sample variable substitution for preview
                content.replace("{{name}}", "John Doe", Qt::CaseInsensitive);
                content.replace("{{email}}", "john.doe@example.com", Qt::CaseInsensitive);
                content.replace("{{company}}", "Example Company", Qt::CaseInsensitive);
                content.replace("{{subject}}", subjectLine->text(), Qt::CaseInsensitive);
            }
        }
        
        // If content looks like HTML (contains HTML tags), render it as HTML
        if (content.contains("<html>", Qt::CaseInsensitive) || 
            content.contains("<body>", Qt::CaseInsensitive) || 
            content.contains("<div>", Qt::CaseInsensitive) ||
            content.contains("<p>", Qt::CaseInsensitive) ||
            content.contains("<br>", Qt::CaseInsensitive)) {
            previewEdit->setHtml(content);
        } else {
            // Plain text content - convert newlines to HTML
            QString htmlContent = content.replace("\n", "<br>");
            previewEdit->setHtml(QString("<html><body><p>%1</p></body></html>").arg(htmlContent));
        }
        
        layout->addWidget(previewEdit);
        
        // Add control buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *closeButton = new QPushButton("Close");
        QPushButton *sendTestButton = new QPushButton("Send Test Email");
        
        connect(closeButton, &QPushButton::clicked, previewDialog, &QDialog::accept);
        connect(sendTestButton, &QPushButton::clicked, [this, previewDialog]() {
            previewDialog->accept();
            // TODO: Implement test email sending
            showSuccess("Test Email", "Test email functionality not yet implemented.");
        });
        
        buttonLayout->addWidget(sendTestButton);
        buttonLayout->addStretch();
        buttonLayout->addWidget(closeButton);
        layout->addLayout(buttonLayout);
        
        previewDialog->exec();
        previewDialog->deleteLater();
    });
    
    editorControls->addWidget(htmlModeButton);
    editorControls->addWidget(textModeButton);
    editorControls->addWidget(previewButton);
    editorControls->addStretch();
    
    contentLayout->addLayout(editorControls);
    contentLayout->addWidget(emailContent);
    
    emailLayout->addWidget(contentGroup);
    
    // Email list group
    QGroupBox *listGroup = new QGroupBox("Email List");
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);
    
    // Email table
    emailTable = new QTableWidget();
    emailTable->setColumnCount(4);
    emailTable->setHorizontalHeaderLabels({"Email", "Name", "Status", "Date Sent"});
    emailTable->horizontalHeader()->setStretchLastSection(true);
    emailTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    listLayout->addWidget(emailTable);
    
    // Email list buttons
    QHBoxLayout *listButtonLayout = new QHBoxLayout();
    addButton = new QPushButton("Add Email");
    removeButton = new QPushButton("Remove Selected");
    clearButton = new QPushButton("Clear All");
    importButton = new QPushButton("Import CSV");
    validateButton = new QPushButton("Validate Emails");
    
    listButtonLayout->addWidget(addButton);
    listButtonLayout->addWidget(removeButton);
    listButtonLayout->addWidget(clearButton);
    listButtonLayout->addWidget(importButton);
    listButtonLayout->addWidget(validateButton);
    listButtonLayout->addStretch();
    
    listLayout->addLayout(listButtonLayout);
    emailLayout->addWidget(listGroup);
    
    // Action buttons
    QHBoxLayout *actionLayout = new QHBoxLayout();
    loadButton = new QPushButton("Load List");
    saveButton = new QPushButton("Save List");
    sendButton = new QPushButton("Send Bulk Emails");
    sendButton->setStyleSheet("QPushButton { background-color: #FFD700; color: #1a2332; font-weight: bold; padding: 10px; }");
    
    actionLayout->addWidget(loadButton);
    actionLayout->addWidget(saveButton);
    actionLayout->addStretch();
    actionLayout->addWidget(sendButton);
    
    emailLayout->addLayout(actionLayout);
    
    // Progress bar
    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    emailLayout->addWidget(progressBar);
    
    tabWidget->addTab(emailTab, "Email Management");
}

void MainWindow::setupHistoryTab()
{
    historyTab = new QWidget();
    QVBoxLayout *historyLayout = new QVBoxLayout(historyTab);
    
    QGroupBox *historyGroup = new QGroupBox("Campaign History");
    QVBoxLayout *historyGroupLayout = new QVBoxLayout(historyGroup);
    
    campaignTable = new QTableWidget();
    campaignTable->setColumnCount(5);
    campaignTable->setHorizontalHeaderLabels({"Campaign ID", "Subject", "Date Sent", "Total Emails", "Status"});
    campaignTable->horizontalHeader()->setStretchLastSection(true);
    historyGroupLayout->addWidget(campaignTable);
    
    QHBoxLayout *historyButtonLayout = new QHBoxLayout();
    refreshButton = new QPushButton("Refresh");
    exportButton = new QPushButton("Export Results");
    
    historyButtonLayout->addWidget(refreshButton);
    historyButtonLayout->addWidget(exportButton);
    historyButtonLayout->addStretch();
    
    historyGroupLayout->addLayout(historyButtonLayout);
    historyLayout->addWidget(historyGroup);
    
    tabWidget->addTab(historyTab, "Campaign History");
}

void MainWindow::setupCleaningTab()
{
    cleaningTab = new QWidget();
    QVBoxLayout *cleaningLayout = new QVBoxLayout(cleaningTab);
    
    QGroupBox *cleaningGroup = new QGroupBox("Data Cleaning");
    QVBoxLayout *cleaningGroupLayout = new QVBoxLayout(cleaningGroup);
    
    cleaningTable = new QTableWidget();
    cleaningTable->setColumnCount(4);
    cleaningTable->setHorizontalHeaderLabels({"Email", "Status", "Issues", "Action"});
    cleaningTable->horizontalHeader()->setStretchLastSection(true);
    cleaningGroupLayout->addWidget(cleaningTable);
    
    QHBoxLayout *cleaningButtonLayout = new QHBoxLayout();
    cleanButton = new QPushButton("Clean Data");
    exportCleanedButton = new QPushButton("Export Cleaned Data");
    
    cleaningButtonLayout->addWidget(cleanButton);
    cleaningButtonLayout->addWidget(exportCleanedButton);
    cleaningButtonLayout->addStretch();
    
    cleaningGroupLayout->addLayout(cleaningButtonLayout);
    cleaningLayout->addWidget(cleaningGroup);
    
    tabWidget->addTab(cleaningTab, "Data Cleaning");
}

void MainWindow::setupConnections()
{
    // Email management connections
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendBulkEmails);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadEmailList);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveEmailList);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addEmail);
    connect(removeButton, &QPushButton::clicked, this, &MainWindow::removeEmail);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearEmails);
    connect(validateButton, &QPushButton::clicked, this, &MainWindow::validateEmails);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importEmails);
    
    // History connections
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadCampaignHistory);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportResults);
    
    // Cleaning connections
    connect(cleanButton, &QPushButton::clicked, this, &MainWindow::cleanData);
    connect(exportCleanedButton, &QPushButton::clicked, this, &MainWindow::exportResults);
    
    // Template management connections
    connect(newTemplateButton, &QPushButton::clicked, this, &MainWindow::newTemplate);
    connect(editTemplateButton, &QPushButton::clicked, this, &MainWindow::editTemplate);
    connect(deleteTemplateButton, &QPushButton::clicked, this, &MainWindow::deleteTemplate);
    connect(templateCombo, &QComboBox::currentIndexChanged, this, &MainWindow::loadTemplate);
}

void MainWindow::sendBulkEmails()
{
    if (emailTable->rowCount() == 0) {
        showError("Error", "No emails to send. Please add emails first.");
        return;
    }
    
    if (subjectLine->text().isEmpty()) {
        showError("Error", "Please enter a subject line.");
        return;
    }
    
    if (emailContent->toPlainText().isEmpty()) {
        showError("Error", "Please enter email content.");
        return;
    }
    
    // Validate SMTP settings
    if (smtpServer->text().isEmpty()) {
        showError("Error", "Please enter SMTP server.");
        return;
    }
    
    if (smtpPort->text().isEmpty()) {
        showError("Error", "Please enter SMTP port.");
        return;
    }
    
    if (senderEmail->text().isEmpty()) {
        showError("Error", "Please enter sender email.");
        return;
    }
    
    // Configure SMTP
    SmtpConfiguration config;
    config.server = smtpServer->text();
    config.port = smtpPort->text().toInt();
    config.username = smtpUsername->text();
    config.password = smtpPassword->text();
    config.encryption = SmtpEncryption::TLS; // Default to TLS
    config.authMethod = config.username.isEmpty() ? SmtpAuthMethod::None : SmtpAuthMethod::Login;
    config.timeout = 30;
    
    smtpSender->setConfiguration(config);
    
    // Test connection first
    if (!smtpSender->testConnection()) {
        showError("SMTP Error", QString("Failed to connect to SMTP server:\n%1").arg(smtpSender->getLastError()));
        return;
    }
    
    // Prepare emails
    QList<EmailMessage> emailMessages;
    QString senderEmailAddr = senderEmail->text();
    QString senderNameText = senderName->text();
    QString subject = subjectLine->text();
    
    // Get template content if selected
    QString htmlContent;
    QString textContent = emailContent->toPlainText();
    
    // Check if we have an HTML template selected
    QString templateName = templateCombo->currentText();
    if (templateName != "None" && templateManager) {
        EmailTemplate* tmpl = templateManager->getTemplate(templateName);
        if (!tmpl->getName().isEmpty()) {
            // Use template content
            htmlContent = tmpl->getHtmlContent();
            textContent = tmpl->getTextContent();
            
            // Apply basic variable substitution for each recipient
            for (int row = 0; row < emailTable->rowCount(); ++row) {
                QTableWidgetItem *emailItem = emailTable->item(row, 0);
                QTableWidgetItem *nameItem = emailTable->item(row, 1);
                
                if (emailItem && !emailItem->text().isEmpty()) {
                    QString recipientEmail = emailItem->text().trimmed();
                    QString recipientName = nameItem ? nameItem->text().trimmed() : recipientEmail;
                    
                    // Create personalized content
                    QString personalizedHtml = htmlContent;
                    QString personalizedText = textContent;
                    
                    // Replace common variables
                    personalizedHtml.replace("{{name}}", recipientName, Qt::CaseInsensitive);
                    personalizedHtml.replace("{{email}}", recipientEmail, Qt::CaseInsensitive);
                    personalizedText.replace("{{name}}", recipientName, Qt::CaseInsensitive);
                    personalizedText.replace("{{email}}", recipientEmail, Qt::CaseInsensitive);
                    
                    EmailMessage message;
                    message.from = senderEmailAddr;
                    message.fromName = senderNameText;
                    message.to = recipientEmail;
                    message.toName = recipientName;
                    message.subject = subject;
                    message.htmlBody = personalizedHtml;
                    message.textBody = personalizedText;
                    message.timestamp = QDateTime::currentDateTime();
                    message.messageId = QString::number(row);
                    
                    emailMessages.append(message);
                }
            }
        }
    }
    
    // If no template, use plain text
    if (emailMessages.isEmpty()) {
        for (int row = 0; row < emailTable->rowCount(); ++row) {
            QTableWidgetItem *emailItem = emailTable->item(row, 0);
            QTableWidgetItem *nameItem = emailTable->item(row, 1);
            
            if (emailItem && !emailItem->text().isEmpty()) {
                QString recipientEmail = emailItem->text().trimmed();
                QString recipientName = nameItem ? nameItem->text().trimmed() : recipientEmail;
                
                EmailMessage message;
                message.from = senderEmailAddr;
                message.fromName = senderNameText;
                message.to = recipientEmail;
                message.toName = recipientName;
                message.subject = subject;
                message.textBody = textContent;
                message.timestamp = QDateTime::currentDateTime();
                message.messageId = QString::number(row);
                
                emailMessages.append(message);
            }
        }
    }
    
    // Setup progress tracking
    progressBar->setVisible(true);
    progressBar->setRange(0, emailMessages.size());
    progressBar->setValue(0);
    
    // Connect signals for progress tracking
    connect(smtpSender, &SmtpEmailSender::emailSent, this, [this](const QString &messageId, const QString &recipient) {
        int row = messageId.toInt();
        if (row >= 0 && row < emailTable->rowCount()) {
            QTableWidgetItem *statusItem = emailTable->item(row, 2);
            if (statusItem) {
                statusItem->setText("Sent");
                statusItem->setBackground(QColor(144, 238, 144)); // Light green
            }
        }
        progressBar->setValue(progressBar->value() + 1);
        updateStatusBar(QString("Sent email to %1").arg(recipient));
    });
    
    connect(smtpSender, &SmtpEmailSender::emailFailed, this, [this](const QString &messageId, const QString &recipient, const QString &error) {
        int row = messageId.toInt();
        if (row >= 0 && row < emailTable->rowCount()) {
            QTableWidgetItem *statusItem = emailTable->item(row, 2);
            QTableWidgetItem *notesItem = emailTable->item(row, 3);
            if (statusItem) {
                statusItem->setText("Failed");
                statusItem->setBackground(QColor(255, 182, 193)); // Light red
            }
            if (notesItem) {
                notesItem->setText(error);
            }
        }
        progressBar->setValue(progressBar->value() + 1);
        updateStatusBar(QString("Failed to send email to %1: %2").arg(recipient, error));
    });
    
    connect(smtpSender, &SmtpEmailSender::queueCompleted, this, [this](int sent, int failed) {
        progressBar->setVisible(false);
        updateStatusBar(QString("Bulk email sending completed: %1 sent, %2 failed").arg(sent).arg(failed));
        showSuccess("Email Campaign Complete", 
                   QString("Email campaign completed:\n• Emails sent: %1\n• Failed: %2").arg(sent).arg(failed));
        
        // Disconnect temporary signals
        disconnect(smtpSender, &SmtpEmailSender::emailSent, this, nullptr);
        disconnect(smtpSender, &SmtpEmailSender::emailFailed, this, nullptr);
        disconnect(smtpSender, &SmtpEmailSender::queueCompleted, this, nullptr);
    });
    
    // Start sending
    smtpSender->sendEmails(emailMessages);
    updateStatusBar("Starting bulk email sending...");
}

void MainWindow::loadEmailList()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Email List", "", "CSV Files (*.csv);;Excel Files (*.xlsx);;All Files (*)");
    if (!fileName.isEmpty()) {
        try {
            bool success = false;
            
            if (fileName.endsWith(".xlsx", Qt::CaseInsensitive)) {
                success = csvReader->loadFromExcel(fileName);
            } else {
                success = csvReader->loadFromCsv(fileName);
            }
            
            if (success) {
                // Clear existing emails
                emailTable->setRowCount(0);
                
                // Add imported data to table
                QList<ContactData> contacts = csvReader->getContacts();
                for (const auto &contact : contacts) {
                    if (!contact.email.isEmpty()) {
                        int tableRow = emailTable->rowCount();
                        emailTable->insertRow(tableRow);
                        emailTable->setItem(tableRow, 0, new QTableWidgetItem(contact.email));
                        emailTable->setItem(tableRow, 1, new QTableWidgetItem(contact.getDisplayName()));
                        emailTable->setItem(tableRow, 2, new QTableWidgetItem("Pending"));
                        emailTable->setItem(tableRow, 3, new QTableWidgetItem(""));
                    }
                }
                
                updateStatusBar(QString("Successfully loaded %1 emails from %2")
                               .arg(contacts.size()).arg(QFileInfo(fileName).fileName()));
                showSuccess("Import Successful", 
                           QString("Imported %1 emails from %2").arg(contacts.size()).arg(QFileInfo(fileName).fileName()));
            } else {
                showError("Import Failed", QString("Failed to import emails from %1")
                         .arg(QFileInfo(fileName).fileName()));
            }
        } catch (const std::exception &e) {
            showError("Import Error", QString("An error occurred while importing: %1").arg(e.what()));
        }
    }
}

void MainWindow::saveEmailList()
{
    if (emailTable->rowCount() == 0) {
        showError("Export Error", "No emails to export. Please add some emails first.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "Save Email List", "", "CSV Files (*.csv);;Excel Files (*.xlsx);;All Files (*)");
    if (!fileName.isEmpty()) {
        try {
            // Create ContactData list from table
            QList<ContactData> contacts;
            
            for (int row = 0; row < emailTable->rowCount(); ++row) {
                QTableWidgetItem *emailItem = emailTable->item(row, 0);
                QTableWidgetItem *nameItem = emailTable->item(row, 1);
                
                if (emailItem && !emailItem->text().isEmpty()) {
                    ContactData contact;
                    contact.email = emailItem->text();
                    contact.fullName = nameItem ? nameItem->text() : "";
                    contacts.append(contact);
                }
            }
            
            bool success = false;
            if (fileName.endsWith(".xlsx", Qt::CaseInsensitive)) {
                success = csvReader->exportToExcel(fileName, contacts);
            } else {
                success = csvReader->exportToCsv(fileName, contacts);
            }
            
            if (success) {
                updateStatusBar(QString("Successfully saved %1 emails to %2")
                               .arg(emailTable->rowCount()).arg(QFileInfo(fileName).fileName()));
                showSuccess("Export Successful", 
                           QString("Saved %1 emails to %2").arg(emailTable->rowCount()).arg(QFileInfo(fileName).fileName()));
            } else {
                showError("Export Failed", QString("Failed to save emails to %1")
                         .arg(QFileInfo(fileName).fileName()));
            }
        } catch (const std::exception &e) {
            showError("Export Error", QString("An error occurred while exporting: %1").arg(e.what()));
        }
    }
}

void MainWindow::addEmail()
{
    int row = emailTable->rowCount();
    emailTable->insertRow(row);
    emailTable->setItem(row, 0, new QTableWidgetItem(""));
    emailTable->setItem(row, 1, new QTableWidgetItem(""));
    emailTable->setItem(row, 2, new QTableWidgetItem("Pending"));
    emailTable->setItem(row, 3, new QTableWidgetItem(""));
}

void MainWindow::removeEmail()
{
    QList<QTableWidgetItem*> selectedItems = emailTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        int row = selectedItems.first()->row();
        emailTable->removeRow(row);
    }
}

void MainWindow::clearEmails()
{
    emailTable->setRowCount(0);
}

void MainWindow::updateEmailStatus()
{
    if (emailTable->rowCount() == 0) {
        return;
    }
    
    // Update status based on current time and random simulation
    int pendingCount = 0;
    int sentCount = 0;
    int failedCount = 0;
    
    for (int row = 0; row < emailTable->rowCount(); ++row) {
        QTableWidgetItem *statusItem = emailTable->item(row, 2);
        if (statusItem) {
            QString status = statusItem->text();
            if (status == "Pending") pendingCount++;
            else if (status == "Sent") sentCount++;
            else if (status == "Failed") failedCount++;
        }
    }
    
    updateStatusBar(QString("Email Status - Pending: %1, Sent: %2, Failed: %3")
                   .arg(pendingCount).arg(sentCount).arg(failedCount));
}

void MainWindow::showCampaignHistory()
{
    tabWidget->setCurrentIndex(1);
    loadCampaignHistory();
}

void MainWindow::exportResults()
{
    if (emailTable->rowCount() == 0) {
        showError("Export Error", "No results to export. Please add some emails first.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "Export Results", "", "CSV Files (*.csv);;Excel Files (*.xlsx);;All Files (*)");
    if (!fileName.isEmpty()) {
        try {
            // Create ContactData list from table with status
            QList<ContactData> contacts;
            
            for (int row = 0; row < emailTable->rowCount(); ++row) {
                QTableWidgetItem *emailItem = emailTable->item(row, 0);
                QTableWidgetItem *nameItem = emailTable->item(row, 1);
                QTableWidgetItem *statusItem = emailTable->item(row, 2);
                QTableWidgetItem *notesItem = emailTable->item(row, 3);
                
                if (emailItem && !emailItem->text().isEmpty()) {
                    ContactData contact;
                    contact.email = emailItem->text();
                    contact.fullName = nameItem ? nameItem->text() : "";
                    // Add status and notes as custom fields
                    contact.customFields["Status"] = statusItem ? statusItem->text() : "";
                    contact.customFields["Notes"] = notesItem ? notesItem->text() : "";
                    contacts.append(contact);
                }
            }
            
            bool success = false;
            if (fileName.endsWith(".xlsx", Qt::CaseInsensitive)) {
                success = csvReader->exportToExcel(fileName, contacts);
            } else {
                success = csvReader->exportToCsv(fileName, contacts);
            }
            
            if (success) {
                updateStatusBar(QString("Successfully exported results to %1")
                               .arg(QFileInfo(fileName).fileName()));
                showSuccess("Export Successful", 
                           QString("Exported results to %1").arg(QFileInfo(fileName).fileName()));
            } else {
                showError("Export Failed", QString("Failed to export results to %1")
                         .arg(QFileInfo(fileName).fileName()));
            }
        } catch (const std::exception &e) {
            showError("Export Error", QString("An error occurred while exporting: %1").arg(e.what()));
        }
    }
}

void MainWindow::importEmails()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Emails", "", "CSV Files (*.csv);;Excel Files (*.xlsx);;All Files (*)");
    if (!fileName.isEmpty()) {
        try {
            bool success = false;
            
            if (fileName.endsWith(".xlsx", Qt::CaseInsensitive)) {
                success = csvReader->loadFromExcel(fileName);
            } else {
                success = csvReader->loadFromCsv(fileName);
            }
            
            if (success) {
                // Add imported data to existing table (append mode)
                QList<ContactData> contacts = csvReader->getContacts();
                int importedCount = 0;
                
                for (const auto &contact : contacts) {
                    if (!contact.email.isEmpty()) {
                        int tableRow = emailTable->rowCount();
                        emailTable->insertRow(tableRow);
                        emailTable->setItem(tableRow, 0, new QTableWidgetItem(contact.email));
                        emailTable->setItem(tableRow, 1, new QTableWidgetItem(contact.getDisplayName()));
                        emailTable->setItem(tableRow, 2, new QTableWidgetItem("Pending"));
                        emailTable->setItem(tableRow, 3, new QTableWidgetItem(""));
                        importedCount++;
                    }
                }
                
                updateStatusBar(QString("Successfully imported %1 emails from %2")
                               .arg(importedCount).arg(QFileInfo(fileName).fileName()));
                showSuccess("Import Successful", 
                           QString("Imported %1 emails from %2").arg(importedCount).arg(QFileInfo(fileName).fileName()));
            } else {
                showError("Import Failed", QString("Failed to import emails from %1")
                         .arg(QFileInfo(fileName).fileName()));
            }
        } catch (const std::exception &e) {
            showError("Import Error", QString("An error occurred while importing: %1").arg(e.what()));
        }
    }
}

void MainWindow::validateEmails()
{
    if (emailTable->rowCount() == 0) {
        showError("Validation Error", "No emails to validate. Please add some emails first.");
        return;
    }
    
    int validCount = 0;
    int invalidCount = 0;
    
    // Simple email validation using regex
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    
    for (int row = 0; row < emailTable->rowCount(); ++row) {
        QTableWidgetItem *emailItem = emailTable->item(row, 0);
        if (emailItem) {
            QString email = emailItem->text().trimmed();
            QTableWidgetItem *statusItem = emailTable->item(row, 2);
            
            if (emailRegex.match(email).hasMatch()) {
                validCount++;
                if (statusItem) {
                    statusItem->setText("Valid");
                    statusItem->setBackground(QColor(144, 238, 144)); // Light green
                }
            } else {
                invalidCount++;
                if (statusItem) {
                    statusItem->setText("Invalid");
                    statusItem->setBackground(QColor(255, 182, 193)); // Light red
                }
                
                QTableWidgetItem *notesItem = emailTable->item(row, 3);
                if (notesItem) {
                    notesItem->setText("Invalid email format");
                }
            }
        }
    }
    
    updateStatusBar(QString("Email validation completed: %1 valid, %2 invalid")
                   .arg(validCount).arg(invalidCount));
    showSuccess("Validation Complete", 
               QString("Validation completed:\n• Valid emails: %1\n• Invalid emails: %2")
               .arg(validCount).arg(invalidCount));
}

void MainWindow::cleanData()
{
    if (emailTable->rowCount() == 0) {
        showError("Cleaning Error", "No data to clean. Please add some emails first.");
        return;
    }
    
    int originalCount = emailTable->rowCount();
    int removedCount = 0;
    int cleanedCount = 0;
    
    // Remove rows from bottom to top to avoid index issues
    for (int row = emailTable->rowCount() - 1; row >= 0; --row) {
        QTableWidgetItem *emailItem = emailTable->item(row, 0);
        QTableWidgetItem *nameItem = emailTable->item(row, 1);
        
        if (emailItem) {
            QString email = emailItem->text().trimmed();
            
            // Remove empty emails
            if (email.isEmpty()) {
                emailTable->removeRow(row);
                removedCount++;
                continue;
            }
            
            // Clean and normalize email
            email = email.toLower();
            emailItem->setText(email);
            cleanedCount++;
            
            // Clean name field
            if (nameItem) {
                QString name = nameItem->text().trimmed();
                // Capitalize first letter of each word
                QStringList nameParts = name.split(' ', Qt::SkipEmptyParts);
                for (QString &part : nameParts) {
                    if (!part.isEmpty()) {
                        part[0] = part[0].toUpper();
                    }
                }
                nameItem->setText(nameParts.join(' '));
            }
        } else {
            emailTable->removeRow(row);
            removedCount++;
        }
    }
    
    // Remove duplicates
    QSet<QString> seenEmails;
    int duplicatesRemoved = 0;
    
    for (int row = emailTable->rowCount() - 1; row >= 0; --row) {
        QTableWidgetItem *emailItem = emailTable->item(row, 0);
        if (emailItem) {
            QString email = emailItem->text().trimmed().toLower();
            if (seenEmails.contains(email)) {
                emailTable->removeRow(row);
                duplicatesRemoved++;
            } else {
                seenEmails.insert(email);
            }
        }
    }
    
    updateStatusBar(QString("Data cleaning completed: %1 cleaned, %2 removed, %3 duplicates removed")
                   .arg(cleanedCount).arg(removedCount).arg(duplicatesRemoved));
    showSuccess("Data Cleaning Complete", 
               QString("Data cleaning completed:\n• Original count: %1\n• Cleaned emails: %2\n• Empty rows removed: %3\n• Duplicates removed: %4\n• Final count: %5")
               .arg(originalCount).arg(cleanedCount).arg(removedCount).arg(duplicatesRemoved).arg(emailTable->rowCount()));
}

void MainWindow::loadCampaignHistory()
{
    campaignTable->setRowCount(0);
    
    // Load from database if available, otherwise show sample data
    if (database && database->isConnected()) {
        // TODO: Load real campaign data from database
        // For now, add sample data
    }
    
    // Add sample campaign history
    struct CampaignInfo {
        QString id;
        QString name;
        QString date;
        QString count;
        QString status;
    };
    
    QList<CampaignInfo> sampleCampaigns = {
        {"CAMP001", "Welcome Email", "2024-12-01", "150", "Completed"},
        {"CAMP002", "Product Launch", "2024-12-02", "200", "Completed"},
        {"CAMP003", "Newsletter #1", "2024-12-03", "180", "Completed"},
        {"CAMP004", "Holiday Promo", "2024-12-04", "250", "In Progress"}
    };
    
    for (const auto &campaign : sampleCampaigns) {
        int row = campaignTable->rowCount();
        campaignTable->insertRow(row);
        campaignTable->setItem(row, 0, new QTableWidgetItem(campaign.id));
        campaignTable->setItem(row, 1, new QTableWidgetItem(campaign.name));
        campaignTable->setItem(row, 2, new QTableWidgetItem(campaign.date));
        campaignTable->setItem(row, 3, new QTableWidgetItem(campaign.count));
        campaignTable->setItem(row, 4, new QTableWidgetItem(campaign.status));
        
        // Color code status
        QTableWidgetItem *statusItem = campaignTable->item(row, 4);
        if (campaign.status == "Completed") {
            statusItem->setBackground(QColor(144, 238, 144)); // Light green
        } else if (campaign.status == "In Progress") {
            statusItem->setBackground(QColor(255, 255, 224)); // Light yellow
        } else if (campaign.status == "Failed") {
            statusItem->setBackground(QColor(255, 182, 193)); // Light red
        }
    }
}

void MainWindow::updateStatusBar(const QString &message)
{
    statusLabel->setText(message);
    statusTimer->start(3000); // Clear after 3 seconds
}

void MainWindow::showError(const QString &title, const QString &message)
{
    QMessageBox::critical(this, title, message);
}

void MainWindow::showSuccess(const QString &title, const QString &message)
{
    QMessageBox::information(this, title, message);
}

// Template management slots
void MainWindow::newTemplate()
{
    TemplateEditor *editor = new TemplateEditor(templateManager, this);
    if (editor->exec() == QDialog::Accepted) {
        updateTemplateCombo();
        updateStatusBar("New template created successfully");
    }
    editor->deleteLater();
}

void MainWindow::editTemplate()
{
    QString templateId = templateCombo->currentData().toString();
    if (templateId.isEmpty()) {
        showError("No Template", "Please select a template to edit.");
        return;
    }
    
    EmailTemplate* emailTemplate = templateManager->getTemplate(templateId);
    if (!emailTemplate) {
        showError("Template Not Found", "The selected template could not be found.");
        return;
    }
    
    TemplateEditor *editor = new TemplateEditor(templateManager, emailTemplate, this);
    if (editor->exec() == QDialog::Accepted) {
        updateTemplateCombo();
        updateStatusBar("Template updated successfully");
    }
    editor->deleteLater();
}

void MainWindow::deleteTemplate()
{
    QString templateId = templateCombo->currentData().toString();
    if (templateId.isEmpty()) {
        showError("No Template", "Please select a template to delete.");
        return;
    }
    
    EmailTemplate* emailTemplate = templateManager->getTemplate(templateId);
    if (!emailTemplate) {
        showError("Template Not Found", "The selected template could not be found.");
        return;
    }
    
    if (templateManager->isBuiltInTemplate(templateId)) {
        showError("Cannot Delete", "Built-in templates cannot be deleted.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
        "Delete Template", 
        QString("Are you sure you want to delete the template '%1'?").arg(emailTemplate->getName()),
        QMessageBox::Yes | QMessageBox::No, 
        QMessageBox::No);
        
    if (reply == QMessageBox::Yes) {
        if (templateManager->deleteTemplate(templateId)) {
            updateTemplateCombo();
            updateStatusBar("Template deleted successfully");
        } else {
            showError("Delete Failed", "Failed to delete the template.");
        }
    }
}

void MainWindow::loadTemplate()
{
    QString templateId = templateCombo->currentData().toString();
    if (templateId.isEmpty()) {
        return;
    }
    
    EmailTemplate* emailTemplate = templateManager->getTemplate(templateId);
    if (!emailTemplate) {
        showError("Template Not Found", "The selected template could not be found.");
        return;
    }
    
    // Get sample variables for preview
    QMap<QString, QString> sampleVars;
    sampleVars["first_name"] = "John";
    sampleVars["last_name"] = "Doe";
    sampleVars["email"] = "john.doe@example.com";
    sampleVars["company_name"] = "Your Company";
    sampleVars["company_address"] = "123 Main St, City, State";
    
    // Load template content with sample variables
    subjectLine->setText(emailTemplate->processSubject(sampleVars));
    emailContent->setHtml(emailTemplate->processTemplate(sampleVars));
    
    updateStatusBar(QString("Template '%1' loaded").arg(emailTemplate->getName()));
}

void MainWindow::saveAsTemplate()
{
    if (subjectLine->text().isEmpty() && emailContent->toPlainText().isEmpty()) {
        showError("No Content", "Please enter email subject and content before saving as template.");
        return;
    }
    
    EmailTemplate* emailTemplate = new EmailTemplate();
    emailTemplate->setName("New Template");
    emailTemplate->setSubject(subjectLine->text());
    emailTemplate->setHtmlContent(emailContent->toHtml());
    emailTemplate->setTextContent(emailContent->toPlainText());
    emailTemplate->setType(TemplateType::Custom);
    
    TemplateEditor *editor = new TemplateEditor(templateManager, emailTemplate, this);
    if (editor->exec() == QDialog::Accepted) {
        updateTemplateCombo();
        updateStatusBar("Template saved successfully");
    } else {
        delete emailTemplate;
    }
    editor->deleteLater();
}

void MainWindow::updateTemplateCombo()
{
    templateCombo->clear();
    templateCombo->addItem("-- Select Template --", "");
    
    QList<EmailTemplate*> templates = templateManager->getAllTemplates();
    for (EmailTemplate* emailTemplate : templates) {
        QString displayName = emailTemplate->getName();
        if (templateManager->isBuiltInTemplate(emailTemplate->getId())) {
            displayName += " (Built-in)";
        }
        templateCombo->addItem(displayName, emailTemplate->getId());
    }
}