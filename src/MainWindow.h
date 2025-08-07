#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QTableWidget>
#include <QProgressBar>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QHeaderView>

#include "EmailManager.h"
#include "CampaignManager.h"
#include "SMTPClient.h"
#include "DatabaseManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onImportContacts();
    void onExportContacts();
    void onAddContact();
    void onRemoveContact();
    void onSendBulkEmails();
    void onNewCampaign();
    void onViewCampaignHistory();
    void onEmailStatusUpdate(const QString& email, const QString& status);
    void onCampaignProgress(int sent, int total);
    void updateStatusBar();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupContactsTab();
    void setupCampaignTab();
    void setupHistoryTab();
    void setupSettingsTab();
    void applyCustomStyles();
    void loadContacts();
    void saveContacts();
    
    // UI Components
    QTabWidget *m_tabWidget;
    
    // Contacts Tab
    QWidget *m_contactsTab;
    QTableWidget *m_contactsTable;
    QPushButton *m_importBtn;
    QPushButton *m_exportBtn;
    QPushButton *m_addContactBtn;
    QPushButton *m_removeContactBtn;
    QLineEdit *m_nameEdit;
    QLineEdit *m_emailEdit;
    
    // Campaign Tab
    QWidget *m_campaignTab;
    QLineEdit *m_subjectEdit;
    QTextEdit *m_messageEdit;
    QPushButton *m_sendBtn;
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    QComboBox *m_templateCombo;
    
    // History Tab
    QWidget *m_historyTab;
    QTableWidget *m_historyTable;
    QPushButton *m_viewDetailsBtn;
    QPushButton *m_exportReportBtn;
    
    // Settings Tab
    QWidget *m_settingsTab;
    QLineEdit *m_smtpServerEdit;
    QSpinBox *m_smtpPortSpin;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QSpinBox *m_delaySpin;
    QPushButton *m_testConnectionBtn;
    QPushButton *m_saveSettingsBtn;
    
    // Core components
    EmailManager *m_emailManager;
    CampaignManager *m_campaignManager;
    SMTPClient *m_smtpClient;
    DatabaseManager *m_dbManager;
    
    // Status tracking
    QTimer *m_statusTimer;
    int m_totalEmails;
    int m_sentEmails;
    int m_deliveredEmails;
    int m_bouncedEmails;
};

#endif // MAINWINDOW_H