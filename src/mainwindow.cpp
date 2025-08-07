#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , emailManager(new EmailManager(this))
    , database(new Database(this))
    , statusTimer(new QTimer(this))
{
    setupUI();
    setupConnections();
    setupStyles();
    
    if (!database->initialize()) {
        QMessageBox::critical(this, "Database Error", "Failed to initialize database!");
    }
    
    statusTimer->setInterval(1000);
    statusTimer->start();
    
    loadCampaignHistory();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Bulk Email Manager - Professional Edition");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
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
    
    // Settings Tab
    setupSettingsTab();
    
    // Status bar
    statusLabel = new QLabel("Ready");
    statusBar()->addWidget(statusLabel);
}

void MainWindow::setupEmailTab()
{
    emailTab = new QWidget();
    QVBoxLayout *emailLayout = new QVBoxLayout(emailTab);
    
    // Top controls
    QHBoxLayout *topControls = new QHBoxLayout();
    
    QGroupBox *senderGroup = new QGroupBox("Sender Information");
    QGridLayout *senderLayout = new QGridLayout(senderGroup);
    
    senderName = new QLineEdit();
    senderName->setPlaceholderText("Sender Name");
    senderEmail = new QLineEdit();
    senderEmail->setPlaceholderText("Sender Email");
    
    senderLayout->addWidget(new QLabel("Name:"), 0, 0);
    senderLayout->addWidget(senderName, 0, 1);
    senderLayout->addWidget(new QLabel("Email:"), 1, 0);
    senderLayout->addWidget(senderEmail, 1, 1);
    
    QGroupBox *emailGroup = new QGroupBox("Email Content");
    QGridLayout *emailContentLayout = new QGridLayout(emailGroup);
    
    subjectLine = new QLineEdit();
    subjectLine->setPlaceholderText("Email Subject");
    emailContent = new QTextEdit();
    emailContent->setPlaceholderText("Enter your email content here...");
    emailContent->setMinimumHeight(200);
    
    emailContentLayout->addWidget(new QLabel("Subject:"), 0, 0);
    emailContentLayout->addWidget(subjectLine, 0, 1);
    emailContentLayout->addWidget(new QLabel("Content:"), 1, 0);
    emailContentLayout->addWidget(emailContent, 1, 1);
    
    topControls->addWidget(senderGroup);
    topControls->addWidget(emailGroup);
    
    emailLayout->addLayout(topControls);
    
    // Email table
    QGroupBox *emailTableGroup = new QGroupBox("Email List");
    QVBoxLayout *emailTableLayout = new QVBoxLayout(emailTableGroup);
    
    emailTable = new QTableWidget();
    emailTable->setColumnCount(5);
    emailTable->setHorizontalHeaderLabels({"Email", "Name", "Status", "Sent Date", "Received Date"});
    emailTable->horizontalHeader()->setStretchLastSection(true);
    emailTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    emailTable->setAlternatingRowColors(true);
    
    emailTableLayout->addWidget(emailTable);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    loadButton = new QPushButton("Load Emails");
    importButton = new QPushButton("Import CSV");
    validateButton = new QPushButton("Validate Emails");
    cleanButton = new QPushButton("Clean Data");
    sendButton = new QPushButton("Send Bulk Emails");
    exportButton = new QPushButton("Export Results");
    
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(importButton);
    buttonLayout->addWidget(validateButton);
    buttonLayout->addWidget(cleanButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(sendButton);
    buttonLayout->addWidget(exportButton);
    
    emailTableLayout->addLayout(buttonLayout);
    
    // Progress bar
    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    emailTableLayout->addWidget(progressBar);
    
    emailLayout->addWidget(emailTableGroup);
    
    tabWidget->addTab(emailTab, "Email Management");
}

void MainWindow::setupHistoryTab()
{
    historyTab = new QWidget();
    QVBoxLayout *historyLayout = new QVBoxLayout(historyTab);
    
    // Date filters
    QHBoxLayout *filterLayout = new QHBoxLayout();
    
    filterLayout->addWidget(new QLabel("Start Date:"));
    startDate = new QDateEdit();
    startDate->setDate(QDate::currentDate().addDays(-30));
    filterLayout->addWidget(startDate);
    
    filterLayout->addWidget(new QLabel("End Date:"));
    endDate = new QDateEdit();
    endDate->setDate(QDate::currentDate());
    filterLayout->addWidget(endDate);
    
    filterButton = new QPushButton("Filter");
    refreshButton = new QPushButton("Refresh");
    
    filterLayout->addWidget(filterButton);
    filterLayout->addWidget(refreshButton);
    filterLayout->addStretch();
    
    historyLayout->addLayout(filterLayout);
    
    // Campaign table
    campaignTable = new QTableWidget();
    campaignTable->setColumnCount(7);
    campaignTable->setHorizontalHeaderLabels({"Campaign ID", "Name", "Subject", "Sent Date", "Total", "Sent", "Received"});
    campaignTable->horizontalHeader()->setStretchLastSection(true);
    campaignTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    campaignTable->setAlternatingRowColors(true);
    
    historyLayout->addWidget(campaignTable);
    
    tabWidget->addTab(historyTab, "Campaign History");
}

void MainWindow::setupSettingsTab()
{
    settingsTab = new QWidget();
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsTab);
    
    QGroupBox *smtpGroup = new QGroupBox("SMTP Settings");
    QGridLayout *smtpLayout = new QGridLayout(smtpGroup);
    
    smtpServer = new QLineEdit();
    smtpServer->setPlaceholderText("smtp.gmail.com");
    smtpPort = new QLineEdit();
    smtpPort->setPlaceholderText("587");
    smtpUsername = new QLineEdit();
    smtpUsername->setPlaceholderText("your-email@gmail.com");
    smtpPassword = new QLineEdit();
    smtpPassword->setEchoMode(QLineEdit::Password);
    smtpPassword->setPlaceholderText("Your password or app password");
    
    smtpLayout->addWidget(new QLabel("SMTP Server:"), 0, 0);
    smtpLayout->addWidget(smtpServer, 0, 1);
    smtpLayout->addWidget(new QLabel("Port:"), 1, 0);
    smtpLayout->addWidget(smtpPort, 1, 1);
    smtpLayout->addWidget(new QLabel("Username:"), 2, 0);
    smtpLayout->addWidget(smtpUsername, 2, 1);
    smtpLayout->addWidget(new QLabel("Password:"), 3, 0);
    smtpLayout->addWidget(smtpPassword, 3, 1);
    
    testConnectionButton = new QPushButton("Test Connection");
    saveSettingsButton = new QPushButton("Save Settings");
    
    QHBoxLayout *settingsButtonLayout = new QHBoxLayout();
    settingsButtonLayout->addWidget(testConnectionButton);
    settingsButtonLayout->addWidget(saveSettingsButton);
    settingsButtonLayout->addStretch();
    
    smtpLayout->addLayout(settingsButtonLayout, 4, 0, 1, 2);
    
    settingsLayout->addWidget(smtpGroup);
    settingsLayout->addStretch();
    
    tabWidget->addTab(settingsTab, "Settings");
}

void MainWindow::setupConnections()
{
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadEmailList);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendBulkEmails);
    connect(importButton, &QPushButton::clicked, this, &MainWindow::importEmails);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportResults);
    connect(validateButton, &QPushButton::clicked, this, &MainWindow::validateEmails);
    connect(cleanButton, &QPushButton::clicked, this, &MainWindow::cleanData);
    
    connect(filterButton, &QPushButton::clicked, this, &MainWindow::showCampaignHistory);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::loadCampaignHistory);
    
    connect(testConnectionButton, &QPushButton::clicked, [this]() {
        // Test SMTP connection
        SmtpSettings settings;
        settings.server = smtpServer->text();
        settings.port = smtpPort->text().toInt();
        settings.username = smtpUsername->text();
        settings.password = smtpPassword->text();
        
        emailManager->setSmtpSettings(settings);
        emailManager->testConnection();
    });
    
    connect(saveSettingsButton, &QPushButton::clicked, [this]() {
        SmtpSettings settings;
        settings.server = smtpServer->text();
        settings.port = smtpPort->text().toInt();
        settings.username = smtpUsername->text();
        settings.password = smtpPassword->text();
        
        emailManager->setSmtpSettings(settings);
        QMessageBox::information(this, "Settings", "Settings saved successfully!");
    });
    
    connect(statusTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    
    connect(emailManager, &EmailManager::emailSent, [this](const EmailData &email) {
        database->updateEmailStatus(email.email, "Sent");
        updateEmailTable();
    });
    
    connect(emailManager, &EmailManager::emailFailed, [this](const EmailData &email, const QString &error) {
        database->updateEmailStatus(email.email, "Failed", error);
        updateEmailTable();
    });
    
    connect(emailManager, &EmailManager::progressUpdated, [this](int current, int total) {
        progressBar->setVisible(true);
        progressBar->setMaximum(total);
        progressBar->setValue(current);
        
        if (current >= total) {
            progressBar->setVisible(false);
        }
    });
}

void MainWindow::setupStyles()
{
    // Apply custom styles for dark blue and gold theme
    QString styleSheet = R"(
        QMainWindow {
            background-color: #1a2332;
            color: #ffffff;
        }
        
        QTabWidget::pane {
            border: 1px solid #2d3748;
            background-color: #1a2332;
        }
        
        QTabBar::tab {
            background-color: #2d3748;
            color: #ffffff;
            padding: 8px 16px;
            margin-right: 2px;
        }
        
        QTabBar::tab:selected {
            background-color: #4a5568;
            border-bottom: 2px solid #ffd700;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #2d3748;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
            color: #ffffff;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            color: #ffd700;
        }
        
        QPushButton {
            background-color: #2d3748;
            border: 1px solid #4a5568;
            border-radius: 4px;
            padding: 8px 16px;
            color: #ffffff;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #4a5568;
            border-color: #ffd700;
        }
        
        QPushButton:pressed {
            background-color: #1a202c;
        }
        
        QPushButton:disabled {
            background-color: #1a202c;
            color: #718096;
        }
        
        QLineEdit, QTextEdit {
            background-color: #2d3748;
            border: 1px solid #4a5568;
            border-radius: 4px;
            padding: 8px;
            color: #ffffff;
        }
        
        QLineEdit:focus, QTextEdit:focus {
            border-color: #ffd700;
        }
        
        QTableWidget {
            background-color: #2d3748;
            alternate-background-color: #1a202c;
            gridline-color: #4a5568;
            color: #ffffff;
        }
        
        QTableWidget::item {
            padding: 8px;
        }
        
        QTableWidget::item:selected {
            background-color: #4a5568;
        }
        
        QHeaderView::section {
            background-color: #1a202c;
            color: #ffd700;
            padding: 8px;
            border: 1px solid #4a5568;
            font-weight: bold;
        }
        
        QProgressBar {
            border: 1px solid #4a5568;
            border-radius: 4px;
            text-align: center;
            background-color: #2d3748;
            color: #ffffff;
        }
        
        QProgressBar::chunk {
            background-color: #ffd700;
            border-radius: 3px;
        }
        
        QLabel {
            color: #ffffff;
        }
        
        QDateEdit {
            background-color: #2d3748;
            border: 1px solid #4a5568;
            border-radius: 4px;
            padding: 8px;
            color: #ffffff;
        }
    )";
    
    setStyleSheet(styleSheet);
}

void MainWindow::loadEmailList()
{
    emailList = database->getAllEmails();
    updateEmailTable();
    statusLabel->setText(QString("Loaded %1 emails").arg(emailList.size()));
}

void MainWindow::sendBulkEmails()
{
    if (emailList.isEmpty()) {
        QMessageBox::warning(this, "No Emails", "Please load or import emails first.");
        return;
    }
    
    if (subjectLine->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Subject", "Please enter an email subject.");
        return;
    }
    
    if (emailContent->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "Missing Content", "Please enter email content.");
        return;
    }
    
    int result = QMessageBox::question(this, "Confirm Send", 
        QString("Are you sure you want to send %1 emails?").arg(emailList.size()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        progressBar->setVisible(true);
        progressBar->setMaximum(emailList.size());
        progressBar->setValue(0);
        
        emailManager->sendBulkEmails(emailList, subjectLine->text(), emailContent->toPlainText());
    }
}

void MainWindow::importEmails()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Emails", "", "CSV Files (*.csv);;All Files (*)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file!");
        return;
    }
    
    emailList.clear();
    QTextStream in(&file);
    QString line = in.readLine(); // Skip header
    
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList fields = line.split(",");
        if (fields.size() >= 1) {
            EmailData email;
            email.email = fields[0].trimmed();
            if (fields.size() >= 2) {
                email.name = fields[1].trimmed();
            }
            emailList.append(email);
        }
    }
    
    file.close();
    updateEmailTable();
    statusLabel->setText(QString("Imported %1 emails").arg(emailList.size()));
}

void MainWindow::exportResults()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Results", "", "CSV Files (*.csv);;All Files (*)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not create file!");
        return;
    }
    
    QTextStream out(&file);
    out << "Email,Name,Status,Sent Date,Received Date,Error Message\n";
    
    for (const EmailData &email : emailList) {
        out << email.email << ","
            << email.name << ","
            << email.status << ","
            << email.sentDate.toString("yyyy-MM-dd hh:mm:ss") << ","
            << email.receivedDate.toString("yyyy-MM-dd hh:mm:ss") << ","
            << email.errorMessage << "\n";
    }
    
    file.close();
    QMessageBox::information(this, "Export", "Results exported successfully!");
}

void MainWindow::validateEmails()
{
    if (emailList.isEmpty()) {
        QMessageBox::warning(this, "No Emails", "Please load or import emails first.");
        return;
    }
    
    emailManager->validateEmails(emailList);
    updateEmailTable();
    statusLabel->setText("Email validation completed");
}

void MainWindow::cleanData()
{
    if (emailList.isEmpty()) {
        QMessageBox::warning(this, "No Emails", "Please load or import emails first.");
        return;
    }
    
    emailManager->cleanEmailData(emailList);
    updateEmailTable();
    statusLabel->setText("Data cleaning completed");
}

void MainWindow::updateStatus()
{
    int total = emailList.size();
    int sent = 0, received = 0, failed = 0;
    
    for (const EmailData &email : emailList) {
        if (email.status == "Sent") sent++;
        else if (email.status == "Received") received++;
        else if (email.status == "Failed") failed++;
    }
    
    QString status = QString("Total: %1 | Sent: %2 | Received: %3 | Failed: %4")
                    .arg(total).arg(sent).arg(received).arg(failed);
    statusLabel->setText(status);
}

void MainWindow::showCampaignHistory()
{
    loadCampaignHistory();
}

void MainWindow::loadCampaignHistory()
{
    campaignHistory = database->getCampaigns(startDate->dateTime(), endDate->dateTime());
    updateCampaignTable();
}

void MainWindow::updateEmailTable()
{
    emailTable->setRowCount(emailList.size());
    
    for (int i = 0; i < emailList.size(); ++i) {
        const EmailData &email = emailList[i];
        
        emailTable->setItem(i, 0, new QTableWidgetItem(email.email));
        emailTable->setItem(i, 1, new QTableWidgetItem(email.name));
        emailTable->setItem(i, 2, new QTableWidgetItem(email.status));
        emailTable->setItem(i, 3, new QTableWidgetItem(email.sentDate.toString("yyyy-MM-dd hh:mm:ss")));
        emailTable->setItem(i, 4, new QTableWidgetItem(email.receivedDate.toString("yyyy-MM-dd hh:mm:ss")));
    }
}

void MainWindow::updateCampaignTable()
{
    campaignTable->setRowCount(campaignHistory.size());
    
    for (int i = 0; i < campaignHistory.size(); ++i) {
        const CampaignData &campaign = campaignHistory[i];
        
        campaignTable->setItem(i, 0, new QTableWidgetItem(campaign.id));
        campaignTable->setItem(i, 1, new QTableWidgetItem(campaign.name));
        campaignTable->setItem(i, 2, new QTableWidgetItem(campaign.subject));
        campaignTable->setItem(i, 3, new QTableWidgetItem(campaign.sentDate.toString("yyyy-MM-dd hh:mm:ss")));
        campaignTable->setItem(i, 4, new QTableWidgetItem(QString::number(campaign.totalEmails)));
        campaignTable->setItem(i, 5, new QTableWidgetItem(QString::number(campaign.sentEmails)));
        campaignTable->setItem(i, 6, new QTableWidgetItem(QString::number(campaign.receivedEmails)));
    }
}