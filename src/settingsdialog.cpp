#include "settingsdialog.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , settings(AppSettings::instance())
    , settingsModified(false)
{
    setupUI();
    loadSettings();
    
    setWindowTitle("Settings - Bulk Email Manager");
    setModal(true);
    resize(600, 500);
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    tabWidget = new QTabWidget(this);
    
    setupGeneralTab();
    setupEmailTab();
    setupUITab();
    setupTemplateTab();
    setupAnalyticsTab();
    setupLoggingTab();
    setupDatabaseTab();
    setupAdvancedTab();
    
    mainLayout->addWidget(tabWidget);
    
    // Dialog buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    importButton = new QPushButton("Import...");
    exportButton = new QPushButton("Export...");
    resetButton = new QPushButton("Reset to Defaults");
    
    buttonLayout->addWidget(importButton);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    
    applyButton = new QPushButton("Apply");
    cancelButton = new QPushButton("Cancel");
    okButton = new QPushButton("OK");
    
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    
    okButton->setDefault(true);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect buttons
    connect(okButton, &QPushButton::clicked, [this]() {
        if (validateSettings()) {
            saveSettings();
            accept();
        }
    });
    
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(applyButton, &QPushButton::clicked, [this]() {
        if (validateSettings()) {
            saveSettings();
            settingsModified = false;
            applyButton->setEnabled(false);
        }
    });
    
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);
    connect(importButton, &QPushButton::clicked, this, &SettingsDialog::importSettings);
    connect(exportButton, &QPushButton::clicked, this, &SettingsDialog::exportSettings);
    
    applyButton->setEnabled(false);
}

void SettingsDialog::setupGeneralTab()
{
    generalTab = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(generalTab);
    
    // Auto-save group
    QGroupBox *autoSaveGroup = new QGroupBox("Auto-Save");
    QFormLayout *autoSaveLayout = new QFormLayout(autoSaveGroup);
    
    autoSaveCheck = new QCheckBox("Enable auto-save");
    autoSaveLayout->addRow(autoSaveCheck);
    
    autoSaveIntervalSpin = new QSpinBox();
    autoSaveIntervalSpin->setRange(1, 60);
    autoSaveIntervalSpin->setSuffix(" minutes");
    autoSaveLayout->addRow("Auto-save interval:", autoSaveIntervalSpin);
    
    layout->addWidget(autoSaveGroup);
    
    // Default sender group
    QGroupBox *senderGroup = new QGroupBox("Default Sender Information");
    QFormLayout *senderLayout = new QFormLayout(senderGroup);
    
    defaultSenderNameEdit = new QLineEdit();
    senderLayout->addRow("Name:", defaultSenderNameEdit);
    
    defaultSenderEmailEdit = new QLineEdit();
    senderLayout->addRow("Email:", defaultSenderEmailEdit);
    
    defaultSignatureEdit = new QTextEdit();
    defaultSignatureEdit->setMaximumHeight(100);
    senderLayout->addRow("Signature:", defaultSignatureEdit);
    
    layout->addWidget(senderGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(generalTab, "General");
    
    // Connect signals for change detection
    connect(autoSaveCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(autoSaveIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(defaultSenderNameEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(defaultSenderEmailEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(defaultSignatureEdit, &QTextEdit::textChanged, this, &SettingsDialog::onSettingChanged);
}

void SettingsDialog::setupEmailTab()
{
    emailTab = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(emailTab);
    
    // SMTP settings group
    QGroupBox *smtpGroup = new QGroupBox("SMTP Server Settings");
    QFormLayout *smtpLayout = new QFormLayout(smtpGroup);
    
    smtpServerEdit = new QLineEdit();
    smtpLayout->addRow("SMTP Server:", smtpServerEdit);
    
    smtpPortSpin = new QSpinBox();
    smtpPortSpin->setRange(1, 65535);
    smtpLayout->addRow("Port:", smtpPortSpin);
    
    encryptionCombo = new QComboBox();
    encryptionCombo->addItems({"None", "SSL/TLS", "STARTTLS"});
    smtpLayout->addRow("Encryption:", encryptionCombo);
    
    usernameEdit = new QLineEdit();
    smtpLayout->addRow("Username:", usernameEdit);
    
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    smtpLayout->addRow("Password:", passwordEdit);
    
    rememberCredentialsCheck = new QCheckBox("Remember credentials");
    smtpLayout->addRow(rememberCredentialsCheck);
    
    testConnectionButton = new QPushButton("Test Connection");
    smtpLayout->addRow(testConnectionButton);
    
    layout->addWidget(smtpGroup);
    
    // Email sending settings group
    QGroupBox *sendingGroup = new QGroupBox("Email Sending Settings");
    QFormLayout *sendingLayout = new QFormLayout(sendingGroup);
    
    sendDelaySpinBox = new QSpinBox();
    sendDelaySpinBox->setRange(0, 60000);
    sendDelaySpinBox->setSuffix(" ms");
    sendingLayout->addRow("Send delay:", sendDelaySpinBox);
    
    maxRetriesSpin = new QSpinBox();
    maxRetriesSpin->setRange(0, 10);
    sendingLayout->addRow("Max retries:", maxRetriesSpin);
    
    connectionTimeoutSpin = new QSpinBox();
    connectionTimeoutSpin->setRange(1, 300);
    connectionTimeoutSpin->setSuffix(" seconds");
    sendingLayout->addRow("Connection timeout:", connectionTimeoutSpin);
    
    confirmBeforeSendingCheck = new QCheckBox("Confirm before sending");
    sendingLayout->addRow(confirmBeforeSendingCheck);
    
    previewEmailsCheck = new QCheckBox("Preview emails before sending");
    sendingLayout->addRow(previewEmailsCheck);
    
    layout->addWidget(sendingGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(emailTab, "Email");
    
    // Connect signals
    connect(smtpServerEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(smtpPortSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(encryptionCombo, &QComboBox::currentTextChanged, this, &SettingsDialog::onSettingChanged);
    connect(usernameEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(passwordEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(rememberCredentialsCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(sendDelaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(maxRetriesSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(connectionTimeoutSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(confirmBeforeSendingCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(previewEmailsCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(testConnectionButton, &QPushButton::clicked, this, &SettingsDialog::testEmailConnection);
}

void SettingsDialog::setupUITab()
{
    uiTab = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(uiTab);
    
    // Appearance group
    QGroupBox *appearanceGroup = new QGroupBox("Appearance");
    QFormLayout *appearanceLayout = new QFormLayout(appearanceGroup);
    
    themeCombo = new QComboBox();
    themeCombo->addItems({"Dark", "Light", "System"});
    appearanceLayout->addRow("Theme:", themeCombo);
    
    fontCombo = new QFontComboBox();
    appearanceLayout->addRow("Font:", fontCombo);
    
    fontSizeSpin = new QSpinBox();
    fontSizeSpin->setRange(8, 72);
    fontSizeSpin->setSuffix(" pt");
    appearanceLayout->addRow("Font size:", fontSizeSpin);
    
    layout->addWidget(appearanceGroup);
    
    // Editor group
    QGroupBox *editorGroup = new QGroupBox("Editor");
    QFormLayout *editorLayout = new QFormLayout(editorGroup);
    
    showLineNumbersCheck = new QCheckBox("Show line numbers");
    editorLayout->addRow(showLineNumbersCheck);
    
    wordWrapCheck = new QCheckBox("Word wrap");
    editorLayout->addRow(wordWrapCheck);
    
    layout->addWidget(editorGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(uiTab, "UI");
    
    // Connect signals
    connect(themeCombo, &QComboBox::currentTextChanged, this, &SettingsDialog::onSettingChanged);
    connect(fontCombo, &QFontComboBox::currentTextChanged, this, &SettingsDialog::onSettingChanged);
    connect(fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(showLineNumbersCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(wordWrapCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
}

void SettingsDialog::setupTemplateTab()
{
    templateTab = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(templateTab);
    
    // Template settings group
    QGroupBox *templateGroup = new QGroupBox("Template Settings");
    QFormLayout *templateLayout = new QFormLayout(templateGroup);
    
    defaultCategoryCombo = new QComboBox();
    defaultCategoryCombo->addItems({"General", "Business", "Newsletter", "Promotional", "Personal"});
    templateLayout->addRow("Default category:", defaultCategoryCombo);
    
    templateDirectoryEdit = new QLineEdit();
    browseTemplateDirectoryButton = new QPushButton("Browse...");
    
    QHBoxLayout *templateDirLayout = new QHBoxLayout();
    templateDirLayout->addWidget(templateDirectoryEdit);
    templateDirLayout->addWidget(browseTemplateDirectoryButton);
    
    templateLayout->addRow("Template directory:", templateDirLayout);
    
    layout->addWidget(templateGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(templateTab, "Templates");
    
    // Connect signals
    connect(defaultCategoryCombo, &QComboBox::currentTextChanged, this, &SettingsDialog::onSettingChanged);
    connect(templateDirectoryEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(browseTemplateDirectoryButton, &QPushButton::clicked, [this]() {
        browseDirectory(templateDirectoryEdit, "Select Template Directory");
    });
}

void SettingsDialog::setupAnalyticsTab()
{
    analyticsTab = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(analyticsTab);
    
    // Analytics settings group
    QGroupBox *analyticsGroup = new QGroupBox("Analytics Settings");
    QFormLayout *analyticsLayout = new QFormLayout(analyticsGroup);
    
    enableAnalyticsCheck = new QCheckBox("Enable analytics tracking");
    analyticsLayout->addRow(enableAnalyticsCheck);
    
    retentionDaysSpin = new QSpinBox();
    retentionDaysSpin->setRange(1, 3650);
    retentionDaysSpin->setSuffix(" days");
    analyticsLayout->addRow("Data retention:", retentionDaysSpin);
    
    trackEmailOpensCheck = new QCheckBox("Track email opens (requires HTML emails)");
    analyticsLayout->addRow(trackEmailOpensCheck);
    
    trackLinkClicksCheck = new QCheckBox("Track link clicks (requires HTML emails)");
    analyticsLayout->addRow(trackLinkClicksCheck);
    
    layout->addWidget(analyticsGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(analyticsTab, "Analytics");
    
    // Connect signals
    connect(enableAnalyticsCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(retentionDaysSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(trackEmailOpensCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(trackLinkClicksCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
}

void SettingsDialog::setupLoggingTab()
{
    loggingTab = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(loggingTab);
    
    // Logging settings group
    QGroupBox *loggingGroup = new QGroupBox("Logging Settings");
    QFormLayout *loggingLayout = new QFormLayout(loggingGroup);
    
    logLevelCombo = new QComboBox();
    logLevelCombo->addItems({"Debug", "Info", "Warning", "Error", "Critical"});
    loggingLayout->addRow("Log level:", logLevelCombo);
    
    logToFileCheck = new QCheckBox("Log to file");
    loggingLayout->addRow(logToFileCheck);
    
    logToConsoleCheck = new QCheckBox("Log to console");
    loggingLayout->addRow(logToConsoleCheck);
    
    maxLogFilesSpin = new QSpinBox();
    maxLogFilesSpin->setRange(1, 100);
    loggingLayout->addRow("Max log files:", maxLogFilesSpin);
    
    maxLogFileSizeSpin = new QSpinBox();
    maxLogFileSizeSpin->setRange(1, 1000);
    maxLogFileSizeSpin->setSuffix(" MB");
    loggingLayout->addRow("Max file size:", maxLogFileSizeSpin);
    
    logDirectoryEdit = new QLineEdit();
    browseLogDirectoryButton = new QPushButton("Browse...");
    
    QHBoxLayout *logDirLayout = new QHBoxLayout();
    logDirLayout->addWidget(logDirectoryEdit);
    logDirLayout->addWidget(browseLogDirectoryButton);
    
    loggingLayout->addRow("Log directory:", logDirLayout);
    
    layout->addWidget(loggingGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(loggingTab, "Logging");
    
    // Connect signals
    connect(logLevelCombo, &QComboBox::currentTextChanged, this, &SettingsDialog::onSettingChanged);
    connect(logToFileCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(logToConsoleCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(maxLogFilesSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(maxLogFileSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(logDirectoryEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(browseLogDirectoryButton, &QPushButton::clicked, [this]() {
        browseDirectory(logDirectoryEdit, "Select Log Directory");
    });
}

void SettingsDialog::setupDatabaseTab()
{
    databaseTab = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(databaseTab);
    
    // Database settings group
    QGroupBox *databaseGroup = new QGroupBox("Database Settings");
    QFormLayout *databaseLayout = new QFormLayout(databaseGroup);
    
    databasePathEdit = new QLineEdit();
    browseDatabasePathButton = new QPushButton("Browse...");
    
    QHBoxLayout *dbPathLayout = new QHBoxLayout();
    dbPathLayout->addWidget(databasePathEdit);
    dbPathLayout->addWidget(browseDatabasePathButton);
    
    databaseLayout->addRow("Database path:", dbPathLayout);
    
    autoBackupCheck = new QCheckBox("Auto backup database");
    databaseLayout->addRow(autoBackupCheck);
    
    backupRetentionSpin = new QSpinBox();
    backupRetentionSpin->setRange(1, 365);
    backupRetentionSpin->setSuffix(" days");
    databaseLayout->addRow("Backup retention:", backupRetentionSpin);
    
    backupDirectoryEdit = new QLineEdit();
    browseBackupDirectoryButton = new QPushButton("Browse...");
    
    QHBoxLayout *backupDirLayout = new QHBoxLayout();
    backupDirLayout->addWidget(backupDirectoryEdit);
    backupDirLayout->addWidget(browseBackupDirectoryButton);
    
    databaseLayout->addRow("Backup directory:", backupDirLayout);
    
    layout->addWidget(databaseGroup);
    
    // Validation settings group
    QGroupBox *validationGroup = new QGroupBox("Import Validation");
    QFormLayout *validationLayout = new QFormLayout(validationGroup);
    
    validateEmailsOnImportCheck = new QCheckBox("Validate emails on import");
    validationLayout->addRow(validateEmailsOnImportCheck);
    
    removeDuplicatesOnImportCheck = new QCheckBox("Remove duplicates on import");
    validationLayout->addRow(removeDuplicatesOnImportCheck);
    
    skipBlacklistedOnImportCheck = new QCheckBox("Skip blacklisted emails on import");
    validationLayout->addRow(skipBlacklistedOnImportCheck);
    
    layout->addWidget(validationGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(databaseTab, "Database");
    
    // Connect signals
    connect(databasePathEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(autoBackupCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(backupRetentionSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(backupDirectoryEdit, &QLineEdit::textChanged, this, &SettingsDialog::onSettingChanged);
    connect(validateEmailsOnImportCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(removeDuplicatesOnImportCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(skipBlacklistedOnImportCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    
    connect(browseDatabasePathButton, &QPushButton::clicked, [this]() {
        browseDirectory(databasePathEdit, "Select Database Directory");
    });
    connect(browseBackupDirectoryButton, &QPushButton::clicked, [this]() {
        browseDirectory(backupDirectoryEdit, "Select Backup Directory");
    });
}

void SettingsDialog::setupAdvancedTab()
{
    advancedTab = new QWidget();
    
    QVBoxLayout *layout = new QVBoxLayout(advancedTab);
    
    // Network settings group
    QGroupBox *networkGroup = new QGroupBox("Network Settings");
    QFormLayout *networkLayout = new QFormLayout(networkGroup);
    
    maxConcurrentConnectionsSpin = new QSpinBox();
    maxConcurrentConnectionsSpin->setRange(1, 20);
    networkLayout->addRow("Max concurrent connections:", maxConcurrentConnectionsSpin);
    
    useDnsCacheCheck = new QCheckBox("Use DNS cache");
    networkLayout->addRow(useDnsCacheCheck);
    
    dnsCacheTimeoutSpin = new QSpinBox();
    dnsCacheTimeoutSpin->setRange(1, 1440);
    dnsCacheTimeoutSpin->setSuffix(" minutes");
    networkLayout->addRow("DNS cache timeout:", dnsCacheTimeoutSpin);
    
    layout->addWidget(networkGroup);
    
    // Debug settings group
    QGroupBox *debugGroup = new QGroupBox("Debug Settings");
    QFormLayout *debugLayout = new QFormLayout(debugGroup);
    
    showDebugInfoCheck = new QCheckBox("Show debug information");
    debugLayout->addRow(showDebugInfoCheck);
    
    layout->addWidget(debugGroup);
    
    layout->addStretch();
    
    tabWidget->addTab(advancedTab, "Advanced");
    
    // Connect signals
    connect(maxConcurrentConnectionsSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(useDnsCacheCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
    connect(dnsCacheTimeoutSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onSettingChanged);
    connect(showDebugInfoCheck, &QCheckBox::toggled, this, &SettingsDialog::onSettingChanged);
}

void SettingsDialog::loadSettings()
{
    // General settings
    autoSaveCheck->setChecked(settings->getAutoSave());
    autoSaveIntervalSpin->setValue(settings->getAutoSaveInterval());
    defaultSenderNameEdit->setText(settings->getDefaultSenderName());
    defaultSenderEmailEdit->setText(settings->getDefaultSenderEmail());
    defaultSignatureEdit->setPlainText(settings->getDefaultSignature());
    
    // Email settings
    smtpServerEdit->setText(settings->getDefaultSmtpServer());
    smtpPortSpin->setValue(settings->getDefaultSmtpPort());
    encryptionCombo->setCurrentText(settings->getDefaultEncryption());
    usernameEdit->setText(settings->getDefaultUsername());
    rememberCredentialsCheck->setChecked(settings->getRememberCredentials());
    if (settings->getRememberCredentials()) {
        passwordEdit->setText(settings->getDefaultPassword());
    }
    sendDelaySpinBox->setValue(settings->getEmailSendDelay());
    maxRetriesSpin->setValue(settings->getMaxRetries());
    connectionTimeoutSpin->setValue(settings->getConnectionTimeout());
    confirmBeforeSendingCheck->setChecked(settings->getConfirmBeforeSending());
    previewEmailsCheck->setChecked(settings->getPreviewEmails());
    
    // UI settings
    themeCombo->setCurrentText(settings->getTheme());
    QFont appFont = settings->getApplicationFont();
    fontCombo->setCurrentFont(appFont);
    fontSizeSpin->setValue(appFont.pointSize());
    showLineNumbersCheck->setChecked(settings->getShowLineNumbers());
    wordWrapCheck->setChecked(settings->getWordWrap());
    
    // Template settings
    defaultCategoryCombo->setCurrentText(settings->getDefaultTemplateCategory());
    templateDirectoryEdit->setText(settings->getTemplateDirectory());
    
    // Analytics settings
    enableAnalyticsCheck->setChecked(settings->getEnableAnalytics());
    retentionDaysSpin->setValue(settings->getAnalyticsRetentionDays());
    trackEmailOpensCheck->setChecked(settings->getTrackEmailOpens());
    trackLinkClicksCheck->setChecked(settings->getTrackLinkClicks());
    
    // Logging settings
    logLevelCombo->setCurrentText(settings->getLogLevel());
    logToFileCheck->setChecked(settings->getLogToFile());
    logToConsoleCheck->setChecked(settings->getLogToConsole());
    maxLogFilesSpin->setValue(settings->getMaxLogFiles());
    maxLogFileSizeSpin->setValue(settings->getMaxLogFileSize());
    logDirectoryEdit->setText(settings->getLogDirectory());
    
    // Database settings
    databasePathEdit->setText(settings->getDatabasePath());
    autoBackupCheck->setChecked(settings->getAutoBackupDatabase());
    backupRetentionSpin->setValue(settings->getBackupRetentionDays());
    backupDirectoryEdit->setText(settings->getBackupDirectory());
    validateEmailsOnImportCheck->setChecked(settings->getValidateEmailsOnImport());
    removeDuplicatesOnImportCheck->setChecked(settings->getRemoveDuplicatesOnImport());
    skipBlacklistedOnImportCheck->setChecked(settings->getSkipBlacklistedOnImport());
    
    // Advanced settings
    maxConcurrentConnectionsSpin->setValue(settings->getMaxConcurrentConnections());
    useDnsCacheCheck->setChecked(settings->getUseDnsCache());
    dnsCacheTimeoutSpin->setValue(settings->getDnsCacheTimeout());
    showDebugInfoCheck->setChecked(settings->getShowDebugInfo());
    
    settingsModified = false;
    applyButton->setEnabled(false);
}

void SettingsDialog::saveSettings()
{
    // General settings
    settings->setAutoSave(autoSaveCheck->isChecked());
    settings->setAutoSaveInterval(autoSaveIntervalSpin->value());
    settings->setDefaultSenderName(defaultSenderNameEdit->text());
    settings->setDefaultSenderEmail(defaultSenderEmailEdit->text());
    settings->setDefaultSignature(defaultSignatureEdit->toPlainText());
    
    // Email settings
    settings->setDefaultSmtpServer(smtpServerEdit->text());
    settings->setDefaultSmtpPort(smtpPortSpin->value());
    settings->setDefaultEncryption(encryptionCombo->currentText());
    settings->setDefaultUsername(usernameEdit->text());
    settings->setRememberCredentials(rememberCredentialsCheck->isChecked());
    if (rememberCredentialsCheck->isChecked()) {
        settings->setDefaultPassword(passwordEdit->text());
    } else {
        settings->setDefaultPassword("");
    }
    settings->setEmailSendDelay(sendDelaySpinBox->value());
    settings->setMaxRetries(maxRetriesSpin->value());
    settings->setConnectionTimeout(connectionTimeoutSpin->value());
    settings->setConfirmBeforeSending(confirmBeforeSendingCheck->isChecked());
    settings->setPreviewEmails(previewEmailsCheck->isChecked());
    
    // UI settings
    settings->setTheme(themeCombo->currentText());
    QFont appFont = fontCombo->currentFont();
    appFont.setPointSize(fontSizeSpin->value());
    settings->setApplicationFont(appFont);
    settings->setShowLineNumbers(showLineNumbersCheck->isChecked());
    settings->setWordWrap(wordWrapCheck->isChecked());
    
    // Template settings
    settings->setDefaultTemplateCategory(defaultCategoryCombo->currentText());
    settings->setTemplateDirectory(templateDirectoryEdit->text());
    
    // Analytics settings
    settings->setEnableAnalytics(enableAnalyticsCheck->isChecked());
    settings->setAnalyticsRetentionDays(retentionDaysSpin->value());
    settings->setTrackEmailOpens(trackEmailOpensCheck->isChecked());
    settings->setTrackLinkClicks(trackLinkClicksCheck->isChecked());
    
    // Logging settings
    settings->setLogLevel(logLevelCombo->currentText());
    settings->setLogToFile(logToFileCheck->isChecked());
    settings->setLogToConsole(logToConsoleCheck->isChecked());
    settings->setMaxLogFiles(maxLogFilesSpin->value());
    settings->setMaxLogFileSize(maxLogFileSizeSpin->value());
    settings->setLogDirectory(logDirectoryEdit->text());
    
    // Database settings
    settings->setDatabasePath(databasePathEdit->text());
    settings->setAutoBackupDatabase(autoBackupCheck->isChecked());
    settings->setBackupRetentionDays(backupRetentionSpin->value());
    settings->setBackupDirectory(backupDirectoryEdit->text());
    settings->setValidateEmailsOnImport(validateEmailsOnImportCheck->isChecked());
    settings->setRemoveDuplicatesOnImport(removeDuplicatesOnImportCheck->isChecked());
    settings->setSkipBlacklistedOnImport(skipBlacklistedOnImportCheck->isChecked());
    
    // Advanced settings
    settings->setMaxConcurrentConnections(maxConcurrentConnectionsSpin->value());
    settings->setUseDnsCache(useDnsCacheCheck->isChecked());
    settings->setDnsCacheTimeout(dnsCacheTimeoutSpin->value());
    settings->setShowDebugInfo(showDebugInfoCheck->isChecked());
    
    settings->save();
}

bool SettingsDialog::validateSettings()
{
    QStringList errors = settings->getValidationErrors();
    
    if (!errors.isEmpty()) {
        QString message = "The following validation errors were found:\n\n";
        message += errors.join("\n");
        
        QMessageBox::warning(this, "Validation Errors", message);
        return false;
    }
    
    return true;
}

void SettingsDialog::resetToDefaults()
{
    int result = QMessageBox::question(this, "Reset Settings", 
                                     "Are you sure you want to reset all settings to their default values?",
                                     QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        settings->resetToDefaults();
        loadSettings();
        settingsModified = true;
        applyButton->setEnabled(true);
    }
}

void SettingsDialog::importSettings()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
                                                   "Import Settings", 
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                   "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        if (settings->importSettings(fileName)) {
            loadSettings();
            settingsModified = true;
            applyButton->setEnabled(true);
            QMessageBox::information(this, "Import Successful", "Settings have been imported successfully.");
        } else {
            QMessageBox::warning(this, "Import Failed", "Failed to import settings from the selected file.");
        }
    }
}

void SettingsDialog::exportSettings()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
                                                   "Export Settings", 
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/bulk_email_settings.json",
                                                   "JSON Files (*.json)");
    
    if (!fileName.isEmpty()) {
        if (settings->exportSettings(fileName)) {
            QMessageBox::information(this, "Export Successful", "Settings have been exported successfully.");
        } else {
            QMessageBox::warning(this, "Export Failed", "Failed to export settings to the selected file.");
        }
    }
}

void SettingsDialog::browseDirectory(QLineEdit *lineEdit, const QString &title)
{
    QString currentDir = lineEdit->text();
    if (currentDir.isEmpty()) {
        currentDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    
    QString selectedDir = QFileDialog::getExistingDirectory(this, title, currentDir);
    
    if (!selectedDir.isEmpty()) {
        lineEdit->setText(selectedDir);
    }
}

void SettingsDialog::testEmailConnection()
{
    // This would test the SMTP connection with current settings
    // For now, just show a message
    QMessageBox::information(this, "Test Connection", 
                            "Connection test functionality will be implemented based on the SMTP settings.");
}

void SettingsDialog::onSettingChanged()
{
    settingsModified = true;
    applyButton->setEnabled(true);
}
