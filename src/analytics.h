#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QJsonObject>
#include <QJsonDocument>

struct EmailDeliveryStats {
    int totalSent = 0;
    int successful = 0;
    int failed = 0;
    int bounced = 0;
    int pending = 0;
    double deliveryRate = 0.0;
    double bounceRate = 0.0;
    QDateTime campaignStartTime;
    QDateTime campaignEndTime;
    qint64 averageDeliveryTime = 0; // in milliseconds
};

struct CampaignAnalytics {
    QString campaignId;
    QString campaignName;
    EmailDeliveryStats deliveryStats;
    QMap<QString, int> errorTypes; // Error type -> count
    QMap<QString, int> providerStats; // Email provider -> count
    QVector<QDateTime> deliveryTimestamps;
    QJsonObject metadata; // Additional campaign data
};

class Analytics : public QObject
{
    Q_OBJECT

public:
    explicit Analytics(QObject *parent = nullptr);
    ~Analytics();

    // Campaign tracking
    QString startCampaign(const QString &campaignName, int totalRecipients);
    void endCampaign(const QString &campaignId);
    void recordEmailSent(const QString &campaignId, const QString &recipient, bool success, const QString &errorMessage = "");
    void recordEmailBounce(const QString &campaignId, const QString &recipient, const QString &bounceType);
    
    // Analytics retrieval
    CampaignAnalytics getCampaignAnalytics(const QString &campaignId) const;
    QVector<CampaignAnalytics> getAllCampaignAnalytics() const;
    QVector<CampaignAnalytics> getCampaignAnalyticsByDateRange(const QDateTime &start, const QDateTime &end) const;
    
    // Statistics
    EmailDeliveryStats getOverallStats() const;
    QMap<QString, int> getTopErrors(int limit = 10) const;
    QMap<QString, double> getProviderDeliveryRates() const;
    
    // Export
    QString generateAnalyticsReport(const QString &campaignId = "") const;
    bool exportAnalyticsToJson(const QString &filePath, const QString &campaignId = "") const;
    bool exportAnalyticsToCSV(const QString &filePath, const QString &campaignId = "") const;
    
    // Settings
    void setRetentionPeriod(int days); // How long to keep analytics data
    void cleanupOldData();

private slots:
    void performCleanup();

private:
    void initializeDatabase();
    void calculateDeliveryStats(CampaignAnalytics &analytics) const;
    QString extractEmailProvider(const QString &email) const;
    void scheduleCleanup();
    
    QMap<QString, CampaignAnalytics> campaigns;
    int retentionDays;
    QTimer *cleanupTimer;
};

#endif // ANALYTICS_H
