#ifndef CAMPAIGNMANAGER_H
#define CAMPAIGNMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QUuid>

struct Campaign {
    QString id;
    QString name;
    QString subject;
    QString message;
    QDateTime date;
    int totalRecipients;
    int sentCount;
    int deliveredCount;
    int bouncedCount;
    int openedCount;
    int clickedCount;
    QString status; // "draft", "sending", "completed", "failed"
};

class CampaignManager : public QObject
{
    Q_OBJECT

public:
    explicit CampaignManager(QObject *parent = nullptr);
    
    // Campaign management
    QString createCampaign(const QString& subject, const QString& message, int totalRecipients);
    bool updateCampaign(const QString& campaignId, const Campaign& campaign);
    bool deleteCampaign(const QString& campaignId);
    Campaign getCampaign(const QString& campaignId) const;
    QList<Campaign> getAllCampaigns() const;
    
    // Campaign statistics
    void updateCampaignStats(const QString& campaignId, int sent, int delivered, int bounced);
    void incrementSentCount(const QString& campaignId);
    void incrementDeliveredCount(const QString& campaignId);
    void incrementBouncedCount(const QString& campaignId);
    void incrementOpenedCount(const QString& campaignId);
    void incrementClickedCount(const QString& campaignId);
    
    // Campaign status
    void setCampaignStatus(const QString& campaignId, const QString& status);
    
    // Analytics
    int getTotalCampaigns() const;
    int getActiveCampaigns() const;
    int getCompletedCampaigns() const;
    double getAverageDeliveryRate() const;
    double getAverageOpenRate() const;
    
    // Export/Import
    bool exportCampaignReport(const QString& campaignId, const QString& fileName);
    bool exportAllCampaigns(const QString& fileName);

signals:
    void campaignCreated(const Campaign& campaign);
    void campaignUpdated(const Campaign& campaign);
    void campaignDeleted(const QString& campaignId);
    void campaignStatsUpdated(const QString& campaignId);

private:
    void loadCampaignsFromDatabase();
    void saveCampaignToDatabase(const Campaign& campaign);
    void removeCampaignFromDatabase(const QString& campaignId);
    Campaign* findCampaign(const QString& campaignId);
    
    QList<Campaign> m_campaigns;
};

#endif // CAMPAIGNMANAGER_H