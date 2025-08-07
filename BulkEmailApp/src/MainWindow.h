#pragma once

#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QDateTime>
#include "EmailSender.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSendClicked();
    void onImportClicked();

private:
    void setupUi();
    void loadCampaigns();
    void addCampaign(const QString &name, const QDateTime &date);

    QTableView *m_campaignView;
    QStandardItemModel *m_campaignModel;

    QTableView *m_recipientView;
    QStandardItemModel *m_recipientModel;

    QPushButton *m_sendButton;
    QPushButton *m_importButton;

    EmailSender m_sender;
};