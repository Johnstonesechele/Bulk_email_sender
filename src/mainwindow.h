#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
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
class QToolBar;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void sendBulkEmails();
    void loadEmailList();
    void saveEmailList();
    void addEmail();
    void removeEmail();
    void clearEmails();
    void updateEmailStatus();
    void showCampaignHistory();
    void exportResults();
    void importEmails();
    void validateEmails();
    void cleanData();

private:
    void setupUI();
    void setupConnections();
    void loadCampaignHistory();
    void updateStatusBar(const QString &message);
    void showError(const QString &title, const QString &message);
    void showSuccess(const QString &title, const QString &message);

    // UI Components
    QTabWidget *tabWidget;
    
    // Email Management Tab
    QWidget *emailTab;
    QTableWidget *emailTable;
    QTextEdit *emailContent;
    QLineEdit *subjectLine;
    QLineEdit *senderEmail;
    QLineEdit *senderName;
    QLineEdit *smtpServer;
    QLineEdit *smtpPort;
    QLineEdit *smtpUsername;
    QLineEdit *smtpPassword;
    QPushButton *sendButton;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *clearButton;
    QPushButton *validateButton;
    QPushButton *importButton;
    QProgressBar *progressBar;
    
    // Campaign History Tab
    QWidget *historyTab;
    QTableWidget *campaignTable;
    QPushButton *exportButton;
    QPushButton *refreshButton;
    
    // Data Cleaning Tab
    QWidget *cleaningTab;
    QTableWidget *cleaningTable;
    QPushButton *cleanButton;
    QPushButton *exportCleanedButton;
    
    // Status
    QLabel *statusLabel;
    QTimer *statusTimer;
    
    // Core components
    EmailManager *emailManager;
    Database *database;
};

#endif // MAINWINDOW_H