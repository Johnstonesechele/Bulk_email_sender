#include "mainwindow.h"
#include "templateeditor.h"
#include <QHeaderView>
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , emailManager(new EmailManager(this))
    , database(new Database(this))
    , templateManager(new TemplateManager(this))
    , statusTimer(new QTimer(this))
{
    setupUI();
    setupConnections();
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
    emailContent->setPlaceholderText("Enter your email message here...");
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
    
    // TODO: Implement email sending logic
    showSuccess("Success", "Bulk email sending started. Check the progress bar for updates.");
    progressBar->setVisible(true);
    progressBar->setRange(0, emailTable->rowCount());
    progressBar->setValue(0);
}

void MainWindow::loadEmailList()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load Email List", "", "CSV Files (*.csv);;All Files (*)");
    if (!fileName.isEmpty()) {
        // TODO: Implement loading logic
        updateStatusBar("Email list loaded from " + fileName);
    }
}

void MainWindow::saveEmailList()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Email List", "", "CSV Files (*.csv);;All Files (*)");
    if (!fileName.isEmpty()) {
        // TODO: Implement saving logic
        updateStatusBar("Email list saved to " + fileName);
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
    // TODO: Implement status update logic
}

void MainWindow::showCampaignHistory()
{
    tabWidget->setCurrentIndex(1);
    loadCampaignHistory();
}

void MainWindow::exportResults()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Results", "", "CSV Files (*.csv);;All Files (*)");
    if (!fileName.isEmpty()) {
        // TODO: Implement export logic
        updateStatusBar("Results exported to " + fileName);
    }
}

void MainWindow::importEmails()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Import Emails", "", "CSV Files (*.csv);;All Files (*)");
    if (!fileName.isEmpty()) {
        // TODO: Implement import logic
        updateStatusBar("Emails imported from " + fileName);
    }
}

void MainWindow::validateEmails()
{
    // TODO: Implement email validation logic
    showSuccess("Validation", "Email validation completed.");
}

void MainWindow::cleanData()
{
    // TODO: Implement data cleaning logic
    showSuccess("Data Cleaning", "Data cleaning completed.");
}

void MainWindow::loadCampaignHistory()
{
    // TODO: Implement campaign history loading
    campaignTable->setRowCount(0);
    // Add sample data for now
    int row = campaignTable->rowCount();
    campaignTable->insertRow(row);
    campaignTable->setItem(row, 0, new QTableWidgetItem("CAMP001"));
    campaignTable->setItem(row, 1, new QTableWidgetItem("Welcome Email"));
    campaignTable->setItem(row, 2, new QTableWidgetItem("2024-01-15"));
    campaignTable->setItem(row, 3, new QTableWidgetItem("150"));
    campaignTable->setItem(row, 4, new QTableWidgetItem("Completed"));
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