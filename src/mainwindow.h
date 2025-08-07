#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include "emailmanager.h"
#include "database.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMenuBar;
class QStatusBar;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadEmailList();
    void sendBulkEmails();
    void importEmails();
    void exportResults();
    void clearData();
    void updateStatus();
    void showCampaignHistory();
    void validateEmails();
    void cleanData();

private:
    void setupUI();
    void setupConnections();
    void setupStyles();
    void loadCampaignHistory();
    void updateEmailTable();
    void updateCampaignTable();

    // UI Components
    QTabWidget *tabWidget;
    
    // Email Management Tab
    QWidget *emailTab;
    QTableWidget *emailTable;
    QTextEdit *emailContent;
    QLineEdit *subjectLine;
    QLineEdit *senderEmail;
    QLineEdit *senderName;
    QPushButton *loadButton;
    QPushButton *sendButton;
    QPushButton *importButton;
    QPushButton *exportButton;
    QPushButton *validateButton;
    QPushButton *cleanButton;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    
    // Campaign History Tab
    QWidget *historyTab;
    QTableWidget *campaignTable;
    QDateEdit *startDate;
    QDateEdit *endDate;
    QPushButton *filterButton;
    QPushButton *refreshButton;
    
    // Settings Tab
    QWidget *settingsTab;
    QLineEdit *smtpServer;
    QLineEdit *smtpPort;
    QLineEdit *smtpUsername;
    QLineEdit *smtpPassword;
    QPushButton *testConnectionButton;
    QPushButton *saveSettingsButton;
    
    // Data members
    EmailManager *emailManager;
    Database *database;
    QTimer *statusTimer;
    
    // Email data
    QList<EmailData> emailList;
    QList<CampaignData> campaignHistory;
};

#endif // MAINWINDOW_H