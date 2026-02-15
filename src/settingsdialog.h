#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QFontComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include "appsettings.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void applySettings();
    void resetToDefaults();
    void importSettings();
    void exportSettings();
    void browseDirectory(QLineEdit *lineEdit, const QString &title);
    void testEmailConnection();
    void onSettingChanged();

private:
    void setupUI();
    void setupGeneralTab();
    void setupEmailTab();
    void setupUITab();
    void setupTemplateTab();
    void setupAnalyticsTab();
    void setupLoggingTab();
    void setupDatabaseTab();
    void setupAdvancedTab();
    void loadSettings();
    void saveSettings();
    bool validateSettings();

    AppSettings *settings;
    
    QTabWidget *tabWidget;
    
    // General tab
    QWidget *generalTab;
    QCheckBox *autoSaveCheck;
    QSpinBox *autoSaveIntervalSpin;
    QLineEdit *defaultSenderNameEdit;
    QLineEdit *defaultSenderEmailEdit;
    QTextEdit *defaultSignatureEdit;
    
    // Email tab
    QWidget *emailTab;
    QLineEdit *smtpServerEdit;
    QSpinBox *smtpPortSpin;
    QComboBox *encryptionCombo;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QCheckBox *rememberCredentialsCheck;
    QSpinBox *sendDelaySpinBox;
    QSpinBox *maxRetriesSpin;
    QSpinBox *connectionTimeoutSpin;
    QCheckBox *confirmBeforeSendingCheck;
    QCheckBox *previewEmailsCheck;
    QPushButton *testConnectionButton;
    
    // UI tab
    QWidget *uiTab;
    QComboBox *themeCombo;
    QFontComboBox *fontCombo;
    QSpinBox *fontSizeSpin;
    QCheckBox *showLineNumbersCheck;
    QCheckBox *wordWrapCheck;
    
    // Template tab
    QWidget *templateTab;
    QComboBox *defaultCategoryCombo;
    QLineEdit *templateDirectoryEdit;
    QPushButton *browseTemplateDirectoryButton;
    
    // Analytics tab
    QWidget *analyticsTab;
    QCheckBox *enableAnalyticsCheck;
    QSpinBox *retentionDaysSpin;
    QCheckBox *trackEmailOpensCheck;
    QCheckBox *trackLinkClicksCheck;
    
    // Logging tab
    QWidget *loggingTab;
    QComboBox *logLevelCombo;
    QCheckBox *logToFileCheck;
    QCheckBox *logToConsoleCheck;
    QSpinBox *maxLogFilesSpin;
    QSpinBox *maxLogFileSizeSpin;
    QLineEdit *logDirectoryEdit;
    QPushButton *browseLogDirectoryButton;
    
    // Database tab
    QWidget *databaseTab;
    QLineEdit *databasePathEdit;
    QPushButton *browseDatabasePathButton;
    QCheckBox *autoBackupCheck;
    QSpinBox *backupRetentionSpin;
    QLineEdit *backupDirectoryEdit;
    QPushButton *browseBackupDirectoryButton;
    QCheckBox *validateEmailsOnImportCheck;
    QCheckBox *removeDuplicatesOnImportCheck;
    QCheckBox *skipBlacklistedOnImportCheck;
    
    // Advanced tab
    QWidget *advancedTab;
    QSpinBox *maxConcurrentConnectionsSpin;
    QCheckBox *useDnsCacheCheck;
    QSpinBox *dnsCacheTimeoutSpin;
    QCheckBox *showDebugInfoCheck;
    
    // Dialog buttons
    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *applyButton;
    QPushButton *resetButton;
    QPushButton *importButton;
    QPushButton *exportButton;
    
    bool settingsModified;
};

#endif // SETTINGSDIALOG_H
