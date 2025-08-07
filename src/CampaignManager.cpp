#include "CampaignManager.h"
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CampaignManager::CampaignManager(QObject *parent)
    : QObject(parent)
{
    loadCampaignsFromDatabase();
}

QString CampaignManager::createCampaign(const QString& subject, const QString& message, int totalRecipients)
{
    Campaign campaign;
    campaign.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    campaign.name = QString("Campaign %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
    campaign.subject = subject;
    campaign.message = message;
    campaign.date = QDateTime::currentDateTime();
    campaign.totalRecipients = totalRecipients;
    campaign.sentCount = 0;
    campaign.deliveredCount = 0;
    campaign.bouncedCount = 0;
    campaign.openedCount = 0;
    campaign.clickedCount = 0;
    campaign.status = "draft";
    
    m_campaigns.append(campaign);
    saveCampaignToDatabase(campaign);
    
    emit campaignCreated(campaign);
    
    qDebug() << "Created campaign:" << campaign.id << "with subject:" << subject;
    return campaign.id;
}

bool CampaignManager::updateCampaign(const QString& campaignId, const Campaign& updatedCampaign)
{
    Campaign* campaign = findCampaign(campaignId);
    if (!campaign) {
        return false;
    }
    
    *campaign = updatedCampaign;
    campaign->id = campaignId; // Ensure ID doesn't change
    
    saveCampaignToDatabase(*campaign);
    emit campaignUpdated(*campaign);
    
    return true;
}

bool CampaignManager::deleteCampaign(const QString& campaignId)
{
    for (int i = 0; i < m_campaigns.size(); ++i) {
        if (m_campaigns[i].id == campaignId) {
            m_campaigns.removeAt(i);
            removeCampaignFromDatabase(campaignId);
            emit campaignDeleted(campaignId);
            return true;
        }
    }
    return false;
}

Campaign CampaignManager::getCampaign(const QString& campaignId) const
{
    for (const auto& campaign : m_campaigns) {
        if (campaign.id == campaignId) {
            return campaign;
        }
    }
    return Campaign(); // Return empty campaign if not found
}

QList<Campaign> CampaignManager::getAllCampaigns() const
{
    return m_campaigns;
}

void CampaignManager::updateCampaignStats(const QString& campaignId, int sent, int delivered, int bounced)
{
    Campaign* campaign = findCampaign(campaignId);
    if (!campaign) {
        return;
    }
    
    campaign->sentCount = sent;
    campaign->deliveredCount = delivered;
    campaign->bouncedCount = bounced;
    
    saveCampaignToDatabase(*campaign);
    emit campaignStatsUpdated(campaignId);
}

void CampaignManager::incrementSentCount(const QString& campaignId)
{
    Campaign* campaign = findCampaign(campaignId);
    if (campaign) {
        campaign->sentCount++;
        saveCampaignToDatabase(*campaign);
        emit campaignStatsUpdated(campaignId);
    }
}

void CampaignManager::incrementDeliveredCount(const QString& campaignId)
{
    Campaign* campaign = findCampaign(campaignId);
    if (campaign) {
        campaign->deliveredCount++;
        saveCampaignToDatabase(*campaign);
        emit campaignStatsUpdated(campaignId);
    }
}

void CampaignManager::incrementBouncedCount(const QString& campaignId)
{
    Campaign* campaign = findCampaign(campaignId);
    if (campaign) {
        campaign->bouncedCount++;
        saveCampaignToDatabase(*campaign);
        emit campaignStatsUpdated(campaignId);
    }
}

void CampaignManager::incrementOpenedCount(const QString& campaignId)
{
    Campaign* campaign = findCampaign(campaignId);
    if (campaign) {
        campaign->openedCount++;
        saveCampaignToDatabase(*campaign);
        emit campaignStatsUpdated(campaignId);
    }
}

void CampaignManager::incrementClickedCount(const QString& campaignId)
{
    Campaign* campaign = findCampaign(campaignId);
    if (campaign) {
        campaign->clickedCount++;
        saveCampaignToDatabase(*campaign);
        emit campaignStatsUpdated(campaignId);
    }
}

void CampaignManager::setCampaignStatus(const QString& campaignId, const QString& status)
{
    Campaign* campaign = findCampaign(campaignId);
    if (campaign) {
        campaign->status = status;
        saveCampaignToDatabase(*campaign);
        emit campaignUpdated(*campaign);
    }
}

int CampaignManager::getTotalCampaigns() const
{
    return m_campaigns.size();
}

int CampaignManager::getActiveCampaigns() const
{
    int count = 0;
    for (const auto& campaign : m_campaigns) {
        if (campaign.status == "sending" || campaign.status == "draft") {
            count++;
        }
    }
    return count;
}

int CampaignManager::getCompletedCampaigns() const
{
    int count = 0;
    for (const auto& campaign : m_campaigns) {
        if (campaign.status == "completed") {
            count++;
        }
    }
    return count;
}

double CampaignManager::getAverageDeliveryRate() const
{
    if (m_campaigns.isEmpty()) {
        return 0.0;
    }
    
    int totalSent = 0;
    int totalDelivered = 0;
    
    for (const auto& campaign : m_campaigns) {
        if (campaign.status == "completed") {
            totalSent += campaign.sentCount;
            totalDelivered += campaign.deliveredCount;
        }
    }
    
    return totalSent > 0 ? (double)totalDelivered / totalSent * 100.0 : 0.0;
}

double CampaignManager::getAverageOpenRate() const
{
    if (m_campaigns.isEmpty()) {
        return 0.0;
    }
    
    int totalDelivered = 0;
    int totalOpened = 0;
    
    for (const auto& campaign : m_campaigns) {
        if (campaign.status == "completed") {
            totalDelivered += campaign.deliveredCount;
            totalOpened += campaign.openedCount;
        }
    }
    
    return totalDelivered > 0 ? (double)totalOpened / totalDelivered * 100.0 : 0.0;
}

bool CampaignManager::exportCampaignReport(const QString& campaignId, const QString& fileName)
{
    Campaign campaign = getCampaign(campaignId);
    if (campaign.id.isEmpty()) {
        return false;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Write campaign report
    out << "Campaign Report\n";
    out << "===============\n\n";
    out << "Campaign ID: " << campaign.id << "\n";
    out << "Campaign Name: " << campaign.name << "\n";
    out << "Subject: " << campaign.subject << "\n";
    out << "Date: " << campaign.date.toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << "Status: " << campaign.status << "\n\n";
    
    out << "Statistics:\n";
    out << "-----------\n";
    out << "Total Recipients: " << campaign.totalRecipients << "\n";
    out << "Sent: " << campaign.sentCount << "\n";
    out << "Delivered: " << campaign.deliveredCount << "\n";
    out << "Bounced: " << campaign.bouncedCount << "\n";
    out << "Opened: " << campaign.openedCount << "\n";
    out << "Clicked: " << campaign.clickedCount << "\n\n";
    
    // Calculate rates
    double deliveryRate = campaign.sentCount > 0 ? 
        (double)campaign.deliveredCount / campaign.sentCount * 100.0 : 0.0;
    double openRate = campaign.deliveredCount > 0 ? 
        (double)campaign.openedCount / campaign.deliveredCount * 100.0 : 0.0;
    double clickRate = campaign.openedCount > 0 ? 
        (double)campaign.clickedCount / campaign.openedCount * 100.0 : 0.0;
    
    out << "Rates:\n";
    out << "------\n";
    out << QString("Delivery Rate: %1%\n").arg(deliveryRate, 0, 'f', 2);
    out << QString("Open Rate: %1%\n").arg(openRate, 0, 'f', 2);
    out << QString("Click Rate: %1%\n").arg(clickRate, 0, 'f', 2);
    
    file.close();
    return true;
}

bool CampaignManager::exportAllCampaigns(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Write CSV header
    out << "Campaign ID,Name,Subject,Date,Status,Total Recipients,Sent,Delivered,Bounced,Opened,Clicked,Delivery Rate,Open Rate,Click Rate\n";
    
    // Write campaign data
    for (const auto& campaign : m_campaigns) {
        double deliveryRate = campaign.sentCount > 0 ? 
            (double)campaign.deliveredCount / campaign.sentCount * 100.0 : 0.0;
        double openRate = campaign.deliveredCount > 0 ? 
            (double)campaign.openedCount / campaign.deliveredCount * 100.0 : 0.0;
        double clickRate = campaign.openedCount > 0 ? 
            (double)campaign.clickedCount / campaign.openedCount * 100.0 : 0.0;
        
        out << "\"" << campaign.id << "\","
            << "\"" << campaign.name << "\","
            << "\"" << campaign.subject << "\","
            << "\"" << campaign.date.toString("yyyy-MM-dd hh:mm:ss") << "\","
            << "\"" << campaign.status << "\","
            << campaign.totalRecipients << ","
            << campaign.sentCount << ","
            << campaign.deliveredCount << ","
            << campaign.bouncedCount << ","
            << campaign.openedCount << ","
            << campaign.clickedCount << ","
            << QString::number(deliveryRate, 'f', 2) << ","
            << QString::number(openRate, 'f', 2) << ","
            << QString::number(clickRate, 'f', 2) << "\n";
    }
    
    file.close();
    return true;
}

void CampaignManager::loadCampaignsFromDatabase()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    QString dbFile = dataDir + "/campaigns.json";
    
    QFile file(dbFile);
    if (!file.exists()) {
        // Create sample campaigns for demonstration
        createCampaign("Welcome to our service!", "Thank you for joining us...", 100);
        createCampaign("Monthly Newsletter", "Here's what's new this month...", 250);
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonArray campaignArray = doc.array();
    
    for (const auto& value : campaignArray) {
        QJsonObject obj = value.toObject();
        
        Campaign campaign;
        campaign.id = obj["id"].toString();
        campaign.name = obj["name"].toString();
        campaign.subject = obj["subject"].toString();
        campaign.message = obj["message"].toString();
        campaign.date = QDateTime::fromString(obj["date"].toString(), Qt::ISODate);
        campaign.totalRecipients = obj["totalRecipients"].toInt();
        campaign.sentCount = obj["sentCount"].toInt();
        campaign.deliveredCount = obj["deliveredCount"].toInt();
        campaign.bouncedCount = obj["bouncedCount"].toInt();
        campaign.openedCount = obj["openedCount"].toInt();
        campaign.clickedCount = obj["clickedCount"].toInt();
        campaign.status = obj["status"].toString();
        
        m_campaigns.append(campaign);
    }
}

void CampaignManager::saveCampaignToDatabase(const Campaign& campaign)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    QString dbFile = dataDir + "/campaigns.json";
    
    QJsonArray campaignArray;
    
    for (const auto& c : m_campaigns) {
        QJsonObject obj;
        obj["id"] = c.id;
        obj["name"] = c.name;
        obj["subject"] = c.subject;
        obj["message"] = c.message;
        obj["date"] = c.date.toString(Qt::ISODate);
        obj["totalRecipients"] = c.totalRecipients;
        obj["sentCount"] = c.sentCount;
        obj["deliveredCount"] = c.deliveredCount;
        obj["bouncedCount"] = c.bouncedCount;
        obj["openedCount"] = c.openedCount;
        obj["clickedCount"] = c.clickedCount;
        obj["status"] = c.status;
        
        campaignArray.append(obj);
    }
    
    QJsonDocument doc(campaignArray);
    
    QFile file(dbFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson());
        file.close();
    }
}

void CampaignManager::removeCampaignFromDatabase(const QString& campaignId)
{
    // The campaign has already been removed from m_campaigns,
    // so we just need to save the updated list
    Campaign dummy; // We need a campaign object for the save function
    saveCampaignToDatabase(dummy);
}

Campaign* CampaignManager::findCampaign(const QString& campaignId)
{
    for (auto& campaign : m_campaigns) {
        if (campaign.id == campaignId) {
            return &campaign;
        }
    }
    return nullptr;
}

#include "CampaignManager.moc"