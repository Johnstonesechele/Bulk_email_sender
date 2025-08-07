#include "MainWindow.h"
#include <QApplication>
#include <QSplitter>
#include <QFrame>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_totalEmails(0)
    , m_sentEmails(0)
    , m_deliveredEmails(0)
    , m_bouncedEmails(0)
{
    setWindowTitle("Bulk Email Manager - Professional Edition");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // Initialize core components
    m_dbManager = new DatabaseManager(this);
    m_emailManager = new EmailManager(this);
    m_campaignManager = new CampaignManager(this);
    m_smtpClient = new SMTPClient(this);
    
    // Setup timer for status updates
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    m_statusTimer->start(1000); // Update every second
    
    // Connect signals
    connect(m_smtpClient, &SMTPClient::emailStatusChanged, 
            this, &MainWindow::onEmailStatusUpdate);
    connect(m_smtpClient, &SMTPClient::campaignProgress,
            this, &MainWindow::onCampaignProgress);
    
    setupUI();
    setupMenuBar();
    setupStatusBar();
    applyCustomStyles();
    loadContacts();
}

MainWindow::~MainWindow()
{
    saveContacts();
}

void MainWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);
    
    setupContactsTab();
    setupCampaignTab();
    setupHistoryTab();
    setupSettingsTab();
}

void MainWindow::setupContactsTab()
{
    m_contactsTab = new QWidget();
    m_tabWidget->addTab(m_contactsTab, "📧 Contacts");
    
    auto *mainLayout = new QHBoxLayout(m_contactsTab);
    
    // Left side - Contact list
    auto *leftWidget = new QWidget();
    auto *leftLayout = new QVBoxLayout(leftWidget);
    
    // Contact management buttons
    auto *buttonLayout = new QHBoxLayout();
    m_importBtn = new QPushButton("📥 Import CSV");
    m_exportBtn = new QPushButton("📤 Export CSV");
    m_addContactBtn = new QPushButton("➕ Add Contact");
    m_removeContactBtn = new QPushButton("🗑️ Remove Selected");
    
    buttonLayout->addWidget(m_importBtn);
    buttonLayout->addWidget(m_exportBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_addContactBtn);
    buttonLayout->addWidget(m_removeContactBtn);
    
    // Contacts table
    m_contactsTable = new QTableWidget();
    m_contactsTable->setColumnCount(4);
    QStringList headers = {"Name", "Email", "Status", "Last Campaign"};
    m_contactsTable->setHorizontalHeaderLabels(headers);
    m_contactsTable->horizontalHeader()->setStretchLastSection(true);
    m_contactsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_contactsTable->setAlternatingRowColors(true);
    
    leftLayout->addLayout(buttonLayout);
    leftLayout->addWidget(m_contactsTable);
    
    // Right side - Add contact form
    auto *rightWidget = new QGroupBox("Add New Contact");
    auto *rightLayout = new QVBoxLayout(rightWidget);
    
    rightLayout->addWidget(new QLabel("Name:"));
    m_nameEdit = new QLineEdit();
    rightLayout->addWidget(m_nameEdit);
    
    rightLayout->addWidget(new QLabel("Email:"));
    m_emailEdit = new QLineEdit();
    rightLayout->addWidget(m_emailEdit);
    
    auto *addBtn = new QPushButton("Add Contact");
    rightLayout->addWidget(addBtn);
    rightLayout->addStretch();
    
    // Add to main layout
    auto *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes({800, 300});
    
    mainLayout->addWidget(splitter);
    
    // Connect signals
    connect(m_importBtn, &QPushButton::clicked, this, &MainWindow::onImportContacts);
    connect(m_exportBtn, &QPushButton::clicked, this, &MainWindow::onExportContacts);
    connect(m_addContactBtn, &QPushButton::clicked, this, &MainWindow::onAddContact);
    connect(m_removeContactBtn, &QPushButton::clicked, this, &MainWindow::onRemoveContact);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddContact);
}

void MainWindow::setupCampaignTab()
{
    m_campaignTab = new QWidget();
    m_tabWidget->addTab(m_campaignTab, "📨 Campaign");
    
    auto *mainLayout = new QVBoxLayout(m_campaignTab);
    
    // Campaign composition area
    auto *composerGroup = new QGroupBox("Email Composition");
    auto *composerLayout = new QGridLayout(composerGroup);
    
    // Subject line
    composerLayout->addWidget(new QLabel("Subject:"), 0, 0);
    m_subjectEdit = new QLineEdit();
    composerLayout->addWidget(m_subjectEdit, 0, 1, 1, 2);
    
    // Template selection
    composerLayout->addWidget(new QLabel("Template:"), 1, 0);
    m_templateCombo = new QComboBox();
    m_templateCombo->addItems({"Custom", "Newsletter", "Promotional", "Welcome", "Follow-up"});
    composerLayout->addWidget(m_templateCombo, 1, 1);
    
    // Message body
    composerLayout->addWidget(new QLabel("Message:"), 2, 0, Qt::AlignTop);
    m_messageEdit = new QTextEdit();
    m_messageEdit->setMinimumHeight(300);
    composerLayout->addWidget(m_messageEdit, 2, 1, 1, 2);
    
    mainLayout->addWidget(composerGroup);
    
    // Campaign controls
    auto *controlsGroup = new QGroupBox("Campaign Controls");
    auto *controlsLayout = new QHBoxLayout(controlsGroup);
    
    m_sendBtn = new QPushButton("🚀 Send Bulk Email");
    m_sendBtn->setMinimumHeight(40);
    
    m_progressBar = new QProgressBar();
    m_progressLabel = new QLabel("Ready to send");
    
    controlsLayout->addWidget(m_sendBtn);
    controlsLayout->addWidget(m_progressBar);
    controlsLayout->addWidget(m_progressLabel);
    
    mainLayout->addWidget(controlsGroup);
    
    // Connect signals
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendBulkEmails);
}

void MainWindow::setupHistoryTab()
{
    m_historyTab = new QWidget();
    m_tabWidget->addTab(m_historyTab, "📊 History");
    
    auto *layout = new QVBoxLayout(m_historyTab);
    
    // Controls
    auto *controlsLayout = new QHBoxLayout();
    m_viewDetailsBtn = new QPushButton("👁️ View Details");
    m_exportReportBtn = new QPushButton("📋 Export Report");
    auto *refreshBtn = new QPushButton("🔄 Refresh");
    
    controlsLayout->addWidget(m_viewDetailsBtn);
    controlsLayout->addWidget(m_exportReportBtn);
    controlsLayout->addStretch();
    controlsLayout->addWidget(refreshBtn);
    
    layout->addLayout(controlsLayout);
    
    // History table
    m_historyTable = new QTableWidget();
    m_historyTable->setColumnCount(7);
    QStringList historyHeaders = {"Date", "Campaign Name", "Subject", "Recipients", "Sent", "Delivered", "Bounced"};
    m_historyTable->setHorizontalHeaderLabels(historyHeaders);
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setAlternatingRowColors(true);
    
    layout->addWidget(m_historyTable);
    
    connect(m_viewDetailsBtn, &QPushButton::clicked, this, &MainWindow::onViewCampaignHistory);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onViewCampaignHistory);
}

void MainWindow::setupSettingsTab()
{
    m_settingsTab = new QWidget();
    m_tabWidget->addTab(m_settingsTab, "⚙️ Settings");
    
    auto *layout = new QVBoxLayout(m_settingsTab);
    
    // SMTP Settings
    auto *smtpGroup = new QGroupBox("SMTP Configuration");
    auto *smtpLayout = new QGridLayout(smtpGroup);
    
    smtpLayout->addWidget(new QLabel("SMTP Server:"), 0, 0);
    m_smtpServerEdit = new QLineEdit("smtp.gmail.com");
    smtpLayout->addWidget(m_smtpServerEdit, 0, 1);
    
    smtpLayout->addWidget(new QLabel("Port:"), 1, 0);
    m_smtpPortSpin = new QSpinBox();
    m_smtpPortSpin->setRange(1, 65535);
    m_smtpPortSpin->setValue(587);
    smtpLayout->addWidget(m_smtpPortSpin, 1, 1);
    
    smtpLayout->addWidget(new QLabel("Username:"), 2, 0);
    m_usernameEdit = new QLineEdit();
    smtpLayout->addWidget(m_usernameEdit, 2, 1);
    
    smtpLayout->addWidget(new QLabel("Password:"), 3, 0);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    smtpLayout->addWidget(m_passwordEdit, 3, 1);
    
    layout->addWidget(smtpGroup);
    
    // Sending Settings
    auto *sendGroup = new QGroupBox("Sending Configuration");
    auto *sendLayout = new QGridLayout(sendGroup);
    
    sendLayout->addWidget(new QLabel("Delay between emails (seconds):"), 0, 0);
    m_delaySpin = new QSpinBox();
    m_delaySpin->setRange(1, 300);
    m_delaySpin->setValue(5);
    sendLayout->addWidget(m_delaySpin, 0, 1);
    
    layout->addWidget(sendGroup);
    
    // Control buttons
    auto *buttonLayout = new QHBoxLayout();
    m_testConnectionBtn = new QPushButton("🔌 Test Connection");
    m_saveSettingsBtn = new QPushButton("💾 Save Settings");
    
    buttonLayout->addWidget(m_testConnectionBtn);
    buttonLayout->addWidget(m_saveSettingsBtn);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    layout->addStretch();
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Import Contacts", this, &MainWindow::onImportContacts);
    fileMenu->addAction("&Export Contacts", this, &MainWindow::onExportContacts);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &QWidget::close);
    
    auto *campaignMenu = menuBar()->addMenu("&Campaign");
    campaignMenu->addAction("&New Campaign", this, &MainWindow::onNewCampaign);
    campaignMenu->addAction("&View History", this, &MainWindow::onViewCampaignHistory);
    
    auto *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", [this]() {
        QMessageBox::about(this, "About Bulk Email Manager",
                          "Bulk Email Manager v1.0\n\n"
                          "Professional email campaign management tool\n"
                          "Built with Qt6 and C++");
    });
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
    statusBar()->addPermanentWidget(new QLabel("Contacts: 0"));
}

void MainWindow::applyCustomStyles()
{
    QString style = R"(
        QTabWidget::pane {
            border: 2px solid #B8860B;
            background-color: #192329;
        }
        
        QTabBar::tab {
            background-color: #354555;
            color: white;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        
        QTabBar::tab:selected {
            background-color: #B8860B;
            color: black;
            font-weight: bold;
        }
        
        QTabBar::tab:hover {
            background-color: #4A5A6A;
        }
        
        QPushButton {
            background-color: #354555;
            color: white;
            border: 2px solid #B8860B;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #B8860B;
            color: black;
        }
        
        QPushButton:pressed {
            background-color: #9A7209;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #B8860B;
            border-radius: 5px;
            margin-top: 10px;
            color: #B8860B;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
        
        QTableWidget {
            gridline-color: #B8860B;
            selection-background-color: #B8860B;
            selection-color: black;
        }
        
        QTableWidget::item:alternate {
            background-color: #2D3D4D;
        }
        
        QHeaderView::section {
            background-color: #354555;
            color: #B8860B;
            padding: 4px;
            border: 1px solid #B8860B;
            font-weight: bold;
        }
        
        QProgressBar {
            border: 2px solid #B8860B;
            border-radius: 5px;
            text-align: center;
        }
        
        QProgressBar::chunk {
            background-color: #B8860B;
            border-radius: 3px;
        }
        
        QLineEdit, QTextEdit, QSpinBox {
            border: 2px solid #B8860B;
            border-radius: 4px;
            padding: 4px;
            background-color: #232D37;
            color: white;
        }
        
        QLineEdit:focus, QTextEdit:focus, QSpinBox:focus {
            border-color: #FFD700;
        }
    )";
    
    setStyleSheet(style);
}

// Slot implementations
void MainWindow::onImportContacts()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Import Contacts", "", "CSV Files (*.csv)");
    
    if (!fileName.isEmpty()) {
        m_emailManager->importContacts(fileName);
        loadContacts();
        QMessageBox::information(this, "Success", "Contacts imported successfully!");
    }
}

void MainWindow::onExportContacts()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export Contacts", "", "CSV Files (*.csv)");
    
    if (!fileName.isEmpty()) {
        m_emailManager->exportContacts(fileName);
        QMessageBox::information(this, "Success", "Contacts exported successfully!");
    }
}

void MainWindow::onAddContact()
{
    QString name = m_nameEdit->text().trimmed();
    QString email = m_emailEdit->text().trimmed();
    
    if (name.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please fill in both name and email fields.");
        return;
    }
    
    if (m_emailManager->addContact(name, email)) {
        m_nameEdit->clear();
        m_emailEdit->clear();
        loadContacts();
        QMessageBox::information(this, "Success", "Contact added successfully!");
    } else {
        QMessageBox::warning(this, "Warning", "Email already exists!");
    }
}

void MainWindow::onRemoveContact()
{
    int currentRow = m_contactsTable->currentRow();
    if (currentRow >= 0) {
        QString email = m_contactsTable->item(currentRow, 1)->text();
        m_emailManager->removeContact(email);
        loadContacts();
    }
}

void MainWindow::onSendBulkEmails()
{
    if (m_subjectEdit->text().isEmpty() || m_messageEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please fill in subject and message fields.");
        return;
    }
    
    auto contacts = m_emailManager->getAllContacts();
    if (contacts.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No contacts available for sending.");
        return;
    }
    
    m_totalEmails = contacts.size();
    m_sentEmails = 0;
    m_deliveredEmails = 0;
    m_bouncedEmails = 0;
    
    m_progressBar->setMaximum(m_totalEmails);
    m_progressBar->setValue(0);
    m_sendBtn->setEnabled(false);
    
    // Create campaign record
    QString campaignId = m_campaignManager->createCampaign(
        m_subjectEdit->text(), 
        m_messageEdit->toPlainText(),
        contacts.size()
    );
    
    // Start sending
    m_smtpClient->sendBulkEmails(contacts, m_subjectEdit->text(), 
                                m_messageEdit->toPlainText(), campaignId);
}

void MainWindow::onNewCampaign()
{
    m_tabWidget->setCurrentIndex(1); // Switch to campaign tab
    m_subjectEdit->clear();
    m_messageEdit->clear();
    m_progressBar->setValue(0);
    m_sendBtn->setEnabled(true);
}

void MainWindow::onViewCampaignHistory()
{
    m_historyTable->setRowCount(0);
    auto campaigns = m_campaignManager->getAllCampaigns();
    
    m_historyTable->setRowCount(campaigns.size());
    
    for (int i = 0; i < campaigns.size(); ++i) {
        const auto& campaign = campaigns[i];
        m_historyTable->setItem(i, 0, new QTableWidgetItem(campaign.date.toString()));
        m_historyTable->setItem(i, 1, new QTableWidgetItem(campaign.name));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(campaign.subject));
        m_historyTable->setItem(i, 3, new QTableWidgetItem(QString::number(campaign.totalRecipients)));
        m_historyTable->setItem(i, 4, new QTableWidgetItem(QString::number(campaign.sentCount)));
        m_historyTable->setItem(i, 5, new QTableWidgetItem(QString::number(campaign.deliveredCount)));
        m_historyTable->setItem(i, 6, new QTableWidgetItem(QString::number(campaign.bouncedCount)));
    }
}

void MainWindow::onEmailStatusUpdate(const QString& email, const QString& status)
{
    if (status == "delivered") {
        m_deliveredEmails++;
    } else if (status == "bounced") {
        m_bouncedEmails++;
    }
    
    // Update contact status in database
    m_emailManager->updateContactStatus(email, status);
}

void MainWindow::onCampaignProgress(int sent, int total)
{
    m_sentEmails = sent;
    m_progressBar->setValue(sent);
    m_progressLabel->setText(QString("Sent: %1/%2").arg(sent).arg(total));
    
    if (sent >= total) {
        m_sendBtn->setEnabled(true);
        QMessageBox::information(this, "Campaign Complete", 
                                QString("Campaign completed!\n"
                                       "Sent: %1\n"
                                       "Delivered: %2\n"
                                       "Bounced: %3")
                                .arg(m_sentEmails)
                                .arg(m_deliveredEmails)
                                .arg(m_bouncedEmails));
    }
}

void MainWindow::updateStatusBar()
{
    int contactCount = m_contactsTable->rowCount();
    statusBar()->showMessage(QString("Contacts: %1 | Sent: %2 | Delivered: %3 | Bounced: %4")
                            .arg(contactCount)
                            .arg(m_sentEmails)
                            .arg(m_deliveredEmails)
                            .arg(m_bouncedEmails));
}

void MainWindow::loadContacts()
{
    auto contacts = m_emailManager->getAllContacts();
    
    m_contactsTable->setRowCount(contacts.size());
    
    for (int i = 0; i < contacts.size(); ++i) {
        const auto& contact = contacts[i];
        m_contactsTable->setItem(i, 0, new QTableWidgetItem(contact.name));
        m_contactsTable->setItem(i, 1, new QTableWidgetItem(contact.email));
        m_contactsTable->setItem(i, 2, new QTableWidgetItem(contact.status));
        m_contactsTable->setItem(i, 3, new QTableWidgetItem(contact.lastCampaign.toString()));
    }
}

void MainWindow::saveContacts()
{
    // Auto-save functionality - contacts are saved automatically in EmailManager
}

#include "MainWindow.moc"