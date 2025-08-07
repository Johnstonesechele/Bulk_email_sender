#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_campaignView(new QTableView(this)),
      m_campaignModel(new QStandardItemModel(this)),
      m_recipientView(new QTableView(this)),
      m_recipientModel(new QStandardItemModel(this)),
      m_sendButton(new QPushButton("Send Emails", this)),
      m_importButton(new QPushButton("Import Recipients", this))
{
    setupUi();
    loadCampaigns();

    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(m_importButton, &QPushButton::clicked, this, &MainWindow::onImportClicked);
}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    m_campaignModel->setHorizontalHeaderLabels({"Campaign", "Date"});
    m_campaignView->setModel(m_campaignModel);
    m_campaignView->horizontalHeader()->setStretchLastSection(true);

    m_recipientModel->setHorizontalHeaderLabels({"Email", "Status"});
    m_recipientView->setModel(m_recipientModel);
    m_recipientView->horizontalHeader()->setStretchLastSection(true);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(m_campaignView);
    leftLayout->addWidget(m_importButton);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_recipientView);
    rightLayout->addWidget(m_sendButton);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 2);

    central->setLayout(mainLayout);

    setWindowTitle("Bulk Email Sender");
    resize(900, 600);
}

void MainWindow::loadCampaigns()
{
    // Placeholder: In a real application load from persistent storage
    addCampaign("Welcome Campaign", QDateTime::currentDateTime());
}

void MainWindow::addCampaign(const QString &name, const QDateTime &date)
{
    QList<QStandardItem*> row;
    row << new QStandardItem(name);
    row << new QStandardItem(date.toString(Qt::ISODate));
    m_campaignModel->appendRow(row);
}

void MainWindow::onSendClicked()
{
    // Iterate through recipient list and send emails via EmailSender
    for (int row = 0; row < m_recipientModel->rowCount(); ++row)
    {
        QString email = m_recipientModel->item(row, 0)->text();
        bool success = m_sender.sendEmail(email, "Subject", "Body");
        m_recipientModel->setItem(row, 1, new QStandardItem(success ? "Sent" : "Failed"));
    }

    QMessageBox::information(this, "Send Complete", "Email sending process completed.");
}

void MainWindow::onImportClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select CSV", QString(), "CSV Files (*.csv)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    m_recipientModel->removeRows(0, m_recipientModel->rowCount());

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QString email = line.trimmed();
        if (!email.isEmpty())
        {
            QList<QStandardItem*> items;
            items << new QStandardItem(email) << new QStandardItem("Pending");
            m_recipientModel->appendRow(items);
        }
    }
}