#include "analytics.h"
#include <QDir>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonArray>
#include <QTextStream>
#include <QFileInfo>
#include <QTimer>
#include <QUuid>
#include <QRegularExpression>
#include <QtMath>

Analytics::Analytics(QObject *parent)
    : QObject(parent)
    , retentionDays(365) // Default 1 year retention
    , cleanupTimer(new QTimer(this))
{
    initializeDatabase();
    scheduleCleanup();
}

Analytics::~Analytics()
{
}

void Analytics::initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "analytics");
    
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    
    QString dbPath = QDir(dataPath).filePath("analytics.db");
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        qWarning() << "Failed to open analytics database:" << db.lastError().text();
        return;
    }
    
    QSqlQuery query(db);
    
    // Create campaigns table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS campaigns (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            start_time DATETIME NOT NULL,
            end_time DATETIME,
            total_recipients INTEGER NOT NULL DEFAULT 0,
            metadata TEXT
        )
    )");
    
    // Create email_deliveries table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS email_deliveries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            campaign_id TEXT NOT NULL,
            recipient TEXT NOT NULL,
            sent_time DATETIME NOT NULL,
            success BOOLEAN NOT NULL,
            error_message TEXT,
            bounce_type TEXT,
            provider TEXT,
            delivery_time_ms INTEGER,
            FOREIGN KEY (campaign_id) REFERENCES campaigns(id)
        )
    )");
    
    // Create indexes for performance
    query.exec("CREATE INDEX IF NOT EXISTS idx_campaign_id ON email_deliveries(campaign_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_sent_time ON email_deliveries(sent_time)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_provider ON email_deliveries(provider)");
}

QString Analytics::startCampaign(const QString &campaignName, int totalRecipients)
{
    QString campaignId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    query.prepare(R"(
        INSERT INTO campaigns (id, name, start_time, total_recipients)
        VALUES (?, ?, ?, ?)
    )");
    
    query.addBindValue(campaignId);
    query.addBindValue(campaignName);
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(totalRecipients);
    
    if (!query.exec()) {
        qWarning() << "Failed to start campaign:" << query.lastError().text();
        return QString();
    }
    
    // Initialize in-memory tracking
    CampaignAnalytics analytics;
    analytics.campaignId = campaignId;
    analytics.campaignName = campaignName;
    analytics.deliveryStats.campaignStartTime = QDateTime::currentDateTime();
    campaigns[campaignId] = analytics;
    
    return campaignId;
}

void Analytics::endCampaign(const QString &campaignId)
{
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    query.prepare("UPDATE campaigns SET end_time = ? WHERE id = ?");
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(campaignId);
    
    if (!query.exec()) {
        qWarning() << "Failed to end campaign:" << query.lastError().text();
    }
    
    if (campaigns.contains(campaignId)) {
        campaigns[campaignId].deliveryStats.campaignEndTime = QDateTime::currentDateTime();
    }
}

void Analytics::recordEmailSent(const QString &campaignId, const QString &recipient, bool success, const QString &errorMessage)
{
    QDateTime sentTime = QDateTime::currentDateTime();
    QString provider = extractEmailProvider(recipient);
    
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    query.prepare(R"(
        INSERT INTO email_deliveries 
        (campaign_id, recipient, sent_time, success, error_message, provider)
        VALUES (?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(campaignId);
    query.addBindValue(recipient);
    query.addBindValue(sentTime);
    query.addBindValue(success);
    query.addBindValue(errorMessage);
    query.addBindValue(provider);
    
    if (!query.exec()) {
        qWarning() << "Failed to record email delivery:" << query.lastError().text();
        return;
    }
    
    // Update in-memory tracking
    if (campaigns.contains(campaignId)) {
        CampaignAnalytics &analytics = campaigns[campaignId];
        analytics.deliveryStats.totalSent++;
        
        if (success) {
            analytics.deliveryStats.successful++;
        } else {
            analytics.deliveryStats.failed++;
            if (!errorMessage.isEmpty()) {
                analytics.errorTypes[errorMessage]++;
            }
        }
        
        analytics.providerStats[provider]++;
        analytics.deliveryTimestamps.append(sentTime);
        
        calculateDeliveryStats(analytics);
    }
}

void Analytics::recordEmailBounce(const QString &campaignId, const QString &recipient, const QString &bounceType)
{
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    query.prepare("UPDATE email_deliveries SET bounce_type = ? WHERE campaign_id = ? AND recipient = ?");
    query.addBindValue(bounceType);
    query.addBindValue(campaignId);
    query.addBindValue(recipient);
    
    if (!query.exec()) {
        qWarning() << "Failed to record email bounce:" << query.lastError().text();
        return;
    }
    
    // Update in-memory tracking
    if (campaigns.contains(campaignId)) {
        CampaignAnalytics &analytics = campaigns[campaignId];
        analytics.deliveryStats.bounced++;
        analytics.deliveryStats.successful--; // Move from successful to bounced
        calculateDeliveryStats(analytics);
    }
}

CampaignAnalytics Analytics::getCampaignAnalytics(const QString &campaignId) const
{
    if (campaigns.contains(campaignId)) {
        return campaigns[campaignId];
    }
    
    // Load from database if not in memory
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery campaignQuery(db);
    
    campaignQuery.prepare("SELECT * FROM campaigns WHERE id = ?");
    campaignQuery.addBindValue(campaignId);
    
    if (!campaignQuery.exec() || !campaignQuery.next()) {
        return CampaignAnalytics(); // Return empty analytics
    }
    
    CampaignAnalytics analytics;
    analytics.campaignId = campaignId;
    analytics.campaignName = campaignQuery.value("name").toString();
    analytics.deliveryStats.campaignStartTime = campaignQuery.value("start_time").toDateTime();
    analytics.deliveryStats.campaignEndTime = campaignQuery.value("end_time").toDateTime();
    
    // Load delivery data
    QSqlQuery deliveryQuery(db);
    deliveryQuery.prepare("SELECT * FROM email_deliveries WHERE campaign_id = ?");
    deliveryQuery.addBindValue(campaignId);
    
    if (deliveryQuery.exec()) {
        while (deliveryQuery.next()) {
            analytics.deliveryStats.totalSent++;
            
            bool success = deliveryQuery.value("success").toBool();
            QString errorMessage = deliveryQuery.value("error_message").toString();
            QString bounceType = deliveryQuery.value("bounce_type").toString();
            QString provider = deliveryQuery.value("provider").toString();
            QDateTime sentTime = deliveryQuery.value("sent_time").toDateTime();
            
            if (!bounceType.isEmpty()) {
                analytics.deliveryStats.bounced++;
            } else if (success) {
                analytics.deliveryStats.successful++;
            } else {
                analytics.deliveryStats.failed++;
                if (!errorMessage.isEmpty()) {
                    analytics.errorTypes[errorMessage]++;
                }
            }
            
            analytics.providerStats[provider]++;
            analytics.deliveryTimestamps.append(sentTime);
        }
    }
    
    calculateDeliveryStats(analytics);
    return analytics;
}

QVector<CampaignAnalytics> Analytics::getAllCampaignAnalytics() const
{
    QVector<CampaignAnalytics> allAnalytics;
    
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    query.exec("SELECT id FROM campaigns ORDER BY start_time DESC");
    
    while (query.next()) {
        QString campaignId = query.value("id").toString();
        allAnalytics.append(getCampaignAnalytics(campaignId));
    }
    
    return allAnalytics;
}

QVector<CampaignAnalytics> Analytics::getCampaignAnalyticsByDateRange(const QDateTime &start, const QDateTime &end) const
{
    QVector<CampaignAnalytics> filteredAnalytics;
    
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    query.prepare("SELECT id FROM campaigns WHERE start_time BETWEEN ? AND ? ORDER BY start_time DESC");
    query.addBindValue(start);
    query.addBindValue(end);
    
    if (query.exec()) {
        while (query.next()) {
            QString campaignId = query.value("id").toString();
            filteredAnalytics.append(getCampaignAnalytics(campaignId));
        }
    }
    
    return filteredAnalytics;
}

EmailDeliveryStats Analytics::getOverallStats() const
{
    EmailDeliveryStats overallStats;
    
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    // Get overall counts
    query.exec(R"(
        SELECT 
            COUNT(*) as total,
            SUM(CASE WHEN success = 1 AND bounce_type IS NULL THEN 1 ELSE 0 END) as successful,
            SUM(CASE WHEN success = 0 THEN 1 ELSE 0 END) as failed,
            SUM(CASE WHEN bounce_type IS NOT NULL THEN 1 ELSE 0 END) as bounced
        FROM email_deliveries
    )");
    
    if (query.next()) {
        overallStats.totalSent = query.value("total").toInt();
        overallStats.successful = query.value("successful").toInt();
        overallStats.failed = query.value("failed").toInt();
        overallStats.bounced = query.value("bounced").toInt();
        
        if (overallStats.totalSent > 0) {
            overallStats.deliveryRate = (double)overallStats.successful / overallStats.totalSent * 100.0;
            overallStats.bounceRate = (double)overallStats.bounced / overallStats.totalSent * 100.0;
        }
    }
    
    return overallStats;
}

QMap<QString, int> Analytics::getTopErrors(int limit) const
{
    QMap<QString, int> topErrors;
    
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    query.prepare(R"(
        SELECT error_message, COUNT(*) as count
        FROM email_deliveries
        WHERE success = 0 AND error_message IS NOT NULL AND error_message != ''
        GROUP BY error_message
        ORDER BY count DESC
        LIMIT ?
    )");
    query.addBindValue(limit);
    
    if (query.exec()) {
        while (query.next()) {
            topErrors[query.value("error_message").toString()] = query.value("count").toInt();
        }
    }
    
    return topErrors;
}

QMap<QString, double> Analytics::getProviderDeliveryRates() const
{
    QMap<QString, double> providerRates;
    
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    query.exec(R"(
        SELECT 
            provider,
            COUNT(*) as total,
            SUM(CASE WHEN success = 1 AND bounce_type IS NULL THEN 1 ELSE 0 END) as successful
        FROM email_deliveries
        WHERE provider IS NOT NULL AND provider != ''
        GROUP BY provider
        HAVING total >= 10
        ORDER BY successful / CAST(total AS FLOAT) DESC
    )");
    
    while (query.next()) {
        QString provider = query.value("provider").toString();
        int total = query.value("total").toInt();
        int successful = query.value("successful").toInt();
        
        if (total > 0) {
            providerRates[provider] = (double)successful / total * 100.0;
        }
    }
    
    return providerRates;
}

QString Analytics::generateAnalyticsReport(const QString &campaignId) const
{
    QString report;
    QTextStream stream(&report);
    
    stream << "=== EMAIL CAMPAIGN ANALYTICS REPORT ===\n";
    stream << "Generated: " << QDateTime::currentDateTime().toString() << "\n\n";
    
    if (campaignId.isEmpty()) {
        // Overall report
        EmailDeliveryStats overallStats = getOverallStats();
        
        stream << "OVERALL STATISTICS\n";
        stream << "------------------\n";
        stream << "Total Emails Sent: " << overallStats.totalSent << "\n";
        stream << "Successful Deliveries: " << overallStats.successful << "\n";
        stream << "Failed Deliveries: " << overallStats.failed << "\n";
        stream << "Bounced Emails: " << overallStats.bounced << "\n";
        stream << QString("Delivery Rate: %1%\n").arg(overallStats.deliveryRate, 0, 'f', 2);
        stream << QString("Bounce Rate: %1%\n").arg(overallStats.bounceRate, 0, 'f', 2);
        stream << "\n";
        
        // Top errors
        auto topErrors = getTopErrors(5);
        if (!topErrors.isEmpty()) {
            stream << "TOP ERROR MESSAGES\n";
            stream << "------------------\n";
            for (auto it = topErrors.constBegin(); it != topErrors.constEnd(); ++it) {
                stream << it.key() << ": " << it.value() << " occurrences\n";
            }
            stream << "\n";
        }
        
        // Provider delivery rates
        auto providerRates = getProviderDeliveryRates();
        if (!providerRates.isEmpty()) {
            stream << "PROVIDER DELIVERY RATES\n";
            stream << "-----------------------\n";
            for (auto it = providerRates.constBegin(); it != providerRates.constEnd(); ++it) {
                stream << it.key() << ": " << QString::number(it.value(), 'f', 2) << "%\n";
            }
        }
    } else {
        // Campaign-specific report
        CampaignAnalytics analytics = getCampaignAnalytics(campaignId);
        
        stream << "CAMPAIGN: " << analytics.campaignName << "\n";
        stream << "Campaign ID: " << analytics.campaignId << "\n";
        stream << "Started: " << analytics.deliveryStats.campaignStartTime.toString() << "\n";
        if (analytics.deliveryStats.campaignEndTime.isValid()) {
            stream << "Ended: " << analytics.deliveryStats.campaignEndTime.toString() << "\n";
        }
        stream << "\n";
        
        stream << "DELIVERY STATISTICS\n";
        stream << "-------------------\n";
        stream << "Total Emails Sent: " << analytics.deliveryStats.totalSent << "\n";
        stream << "Successful Deliveries: " << analytics.deliveryStats.successful << "\n";
        stream << "Failed Deliveries: " << analytics.deliveryStats.failed << "\n";
        stream << "Bounced Emails: " << analytics.deliveryStats.bounced << "\n";
        stream << QString("Delivery Rate: %1%\n").arg(analytics.deliveryStats.deliveryRate, 0, 'f', 2);
        stream << QString("Bounce Rate: %1%\n").arg(analytics.deliveryStats.bounceRate, 0, 'f', 2);
        
        if (!analytics.errorTypes.isEmpty()) {
            stream << "\nERRORS BY TYPE\n";
            stream << "--------------\n";
            for (auto it = analytics.errorTypes.constBegin(); it != analytics.errorTypes.constEnd(); ++it) {
                stream << it.key() << ": " << it.value() << " occurrences\n";
            }
        }
        
        if (!analytics.providerStats.isEmpty()) {
            stream << "\nEMAIL PROVIDERS\n";
            stream << "---------------\n";
            for (auto it = analytics.providerStats.constBegin(); it != analytics.providerStats.constEnd(); ++it) {
                stream << it.key() << ": " << it.value() << " emails\n";
            }
        }
    }
    
    return report;
}

bool Analytics::exportAnalyticsToJson(const QString &filePath, const QString &campaignId) const
{
    QJsonObject jsonReport;
    jsonReport["generated"] = QDateTime::currentDateTime().toString();
    
    if (campaignId.isEmpty()) {
        // Export all campaigns
        QJsonArray campaignsArray;
        auto allAnalytics = getAllCampaignAnalytics();
        
        for (const auto &analytics : allAnalytics) {
            QJsonObject campaignObj;
            campaignObj["id"] = analytics.campaignId;
            campaignObj["name"] = analytics.campaignName;
            campaignObj["start_time"] = analytics.deliveryStats.campaignStartTime.toString();
            campaignObj["end_time"] = analytics.deliveryStats.campaignEndTime.toString();
            campaignObj["total_sent"] = analytics.deliveryStats.totalSent;
            campaignObj["successful"] = analytics.deliveryStats.successful;
            campaignObj["failed"] = analytics.deliveryStats.failed;
            campaignObj["bounced"] = analytics.deliveryStats.bounced;
            campaignObj["delivery_rate"] = analytics.deliveryStats.deliveryRate;
            campaignObj["bounce_rate"] = analytics.deliveryStats.bounceRate;
            
            campaignsArray.append(campaignObj);
        }
        
        jsonReport["campaigns"] = campaignsArray;
        jsonReport["overall_stats"] = QJsonObject::fromVariantMap(QVariantMap{
            {"total_sent", getOverallStats().totalSent},
            {"successful", getOverallStats().successful},
            {"failed", getOverallStats().failed},
            {"bounced", getOverallStats().bounced},
            {"delivery_rate", getOverallStats().deliveryRate},
            {"bounce_rate", getOverallStats().bounceRate}
        });
    } else {
        // Export specific campaign
        CampaignAnalytics analytics = getCampaignAnalytics(campaignId);
        jsonReport["campaign"] = QJsonObject{
            {"id", analytics.campaignId},
            {"name", analytics.campaignName},
            {"start_time", analytics.deliveryStats.campaignStartTime.toString()},
            {"end_time", analytics.deliveryStats.campaignEndTime.toString()},
            {"total_sent", analytics.deliveryStats.totalSent},
            {"successful", analytics.deliveryStats.successful},
            {"failed", analytics.deliveryStats.failed},
            {"bounced", analytics.deliveryStats.bounced},
            {"delivery_rate", analytics.deliveryStats.deliveryRate},
            {"bounce_rate", analytics.deliveryStats.bounceRate}
        };
    }
    
    QJsonDocument doc(jsonReport);
    
    QFileInfo fileInfo(filePath);
    QDir().mkpath(fileInfo.dir().path());
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

bool Analytics::exportAnalyticsToCSV(const QString &filePath, const QString &campaignId) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    
    if (campaignId.isEmpty()) {
        // Export all campaigns
        stream << "Campaign ID,Campaign Name,Start Time,End Time,Total Sent,Successful,Failed,Bounced,Delivery Rate %,Bounce Rate %\n";
        
        auto allAnalytics = getAllCampaignAnalytics();
        for (const auto &analytics : allAnalytics) {
            stream << analytics.campaignId << ","
                   << analytics.campaignName << ","
                   << analytics.deliveryStats.campaignStartTime.toString() << ","
                   << analytics.deliveryStats.campaignEndTime.toString() << ","
                   << analytics.deliveryStats.totalSent << ","
                   << analytics.deliveryStats.successful << ","
                   << analytics.deliveryStats.failed << ","
                   << analytics.deliveryStats.bounced << ","
                   << QString::number(analytics.deliveryStats.deliveryRate, 'f', 2) << ","
                   << QString::number(analytics.deliveryStats.bounceRate, 'f', 2) << "\n";
        }
    } else {
        // Export specific campaign details
        CampaignAnalytics analytics = getCampaignAnalytics(campaignId);
        
        stream << "Campaign: " << analytics.campaignName << "\n";
        stream << "Campaign ID: " << analytics.campaignId << "\n";
        stream << "Start Time: " << analytics.deliveryStats.campaignStartTime.toString() << "\n";
        stream << "End Time: " << analytics.deliveryStats.campaignEndTime.toString() << "\n\n";
        
        stream << "Metric,Value\n";
        stream << "Total Sent," << analytics.deliveryStats.totalSent << "\n";
        stream << "Successful," << analytics.deliveryStats.successful << "\n";
        stream << "Failed," << analytics.deliveryStats.failed << "\n";
        stream << "Bounced," << analytics.deliveryStats.bounced << "\n";
        stream << "Delivery Rate %," << QString::number(analytics.deliveryStats.deliveryRate, 'f', 2) << "\n";
        stream << "Bounce Rate %," << QString::number(analytics.deliveryStats.bounceRate, 'f', 2) << "\n";
    }
    
    return true;
}

void Analytics::setRetentionPeriod(int days)
{
    retentionDays = days;
}

void Analytics::cleanupOldData()
{
    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-retentionDays);
    
    QSqlDatabase db = QSqlDatabase::database("analytics");
    QSqlQuery query(db);
    
    // Delete old email deliveries
    query.prepare("DELETE FROM email_deliveries WHERE sent_time < ?");
    query.addBindValue(cutoffDate);
    
    if (!query.exec()) {
        qWarning() << "Failed to cleanup old email delivery data:" << query.lastError().text();
        return;
    }
    
    // Delete campaigns with no deliveries
    query.exec(R"(
        DELETE FROM campaigns 
        WHERE id NOT IN (SELECT DISTINCT campaign_id FROM email_deliveries)
        AND start_time < ?
    )");
    
    if (!query.exec()) {
        qWarning() << "Failed to cleanup old campaign data:" << query.lastError().text();
    }
    
    qDebug() << "Analytics cleanup completed. Removed data older than" << retentionDays << "days";
}

void Analytics::performCleanup()
{
    cleanupOldData();
}

void Analytics::calculateDeliveryStats(CampaignAnalytics &analytics) const
{
    const EmailDeliveryStats &stats = analytics.deliveryStats;
    
    if (stats.totalSent > 0) {
        analytics.deliveryStats.deliveryRate = (double)(stats.successful - stats.bounced) / stats.totalSent * 100.0;
        analytics.deliveryStats.bounceRate = (double)stats.bounced / stats.totalSent * 100.0;
    }
    
    // Calculate average delivery time
    if (!analytics.deliveryTimestamps.isEmpty() && analytics.deliveryStats.campaignStartTime.isValid()) {
        qint64 totalTime = 0;
        for (const QDateTime &timestamp : analytics.deliveryTimestamps) {
            totalTime += analytics.deliveryStats.campaignStartTime.msecsTo(timestamp);
        }
        analytics.deliveryStats.averageDeliveryTime = totalTime / analytics.deliveryTimestamps.size();
    }
}

QString Analytics::extractEmailProvider(const QString &email) const
{
    QRegularExpression regex("@([^.]+\\.[^.]+)$");
    QRegularExpressionMatch match = regex.match(email);
    
    if (match.hasMatch()) {
        return match.captured(1).toLower();
    }
    
    return "unknown";
}

void Analytics::scheduleCleanup()
{
    // Schedule cleanup to run daily
    cleanupTimer->setInterval(24 * 60 * 60 * 1000); // 24 hours
    connect(cleanupTimer, &QTimer::timeout, this, &Analytics::performCleanup);
    cleanupTimer->start();
    
    // Also run cleanup once at startup
    QTimer::singleShot(5000, this, &Analytics::performCleanup); // 5 seconds after startup
}
