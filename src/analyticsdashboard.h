#ifndef ANALYTICSDASHBOARD_H
#define ANALYTICSDASHBOARD_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QProgressBar>
#include <QGroupBox>
#include <QPainter>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QPieSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QBarCategoryAxis>
#include <QTimer>
#include "analytics.h"

QT_CHARTS_USE_NAMESPACE

class MetricWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MetricWidget(const QString &title, QWidget *parent = nullptr);
    
    void setValue(const QString &value);
    void setSubtitle(const QString &subtitle = "");
    void setColor(const QColor &color);

private:
    QLabel *titleLabel;
    QLabel *valueLabel;
    QLabel *subtitleLabel;
    QColor accentColor;
};

class AnalyticsDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyticsDashboard(QWidget *parent = nullptr);
    ~AnalyticsDashboard();
    
    void setAnalytics(Analytics *analytics);
    void refreshData();
    
public slots:
    void onCampaignFilterChanged();
    void onDateRangeChanged();
    void exportReport();
    void scheduleAutoRefresh(bool enabled, int intervalMinutes = 5);

private slots:
    void autoRefresh();

private:
    void setupUI();
    void setupMetricsSection();
    void setupChartsSection();
    void setupTableSection();
    void setupControls();
    
    void updateMetrics();
    void updateCharts();
    void updateTable();
    
    void createDeliveryRateChart();
    void createCampaignPerformanceChart();
    void createProviderDistributionChart();
    void createTimelineChart();
    
    QWidget *createMetricCard(const QString &title, const QString &value, 
                             const QString &subtitle = "", const QColor &color = QColor("#4CAF50"));
    
    Analytics *analytics;
    
    // Layout
    QVBoxLayout *mainLayout;
    QHBoxLayout *controlsLayout;
    QHBoxLayout *metricsLayout;
    QGridLayout *chartsLayout;
    
    // Controls
    QComboBox *campaignFilterCombo;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QPushButton *refreshButton;
    QPushButton *exportButton;
    
    // Metrics widgets
    MetricWidget *totalSentMetric;
    MetricWidget *deliveryRateMetric;
    MetricWidget *bounceRateMetric;
    MetricWidget *activeCampaignsMetric;
    
    // Charts
    QChartView *deliveryRateChartView;
    QChartView *campaignPerformanceChartView;
    QChartView *providerDistributionChartView;
    QChartView *timelineChartView;
    
    // Table
    QTableWidget *campaignSummaryTable;
    
    // Auto-refresh
    QTimer *refreshTimer;
    bool autoRefreshEnabled;
    
    // Current data
    QVector<CampaignAnalytics> currentCampaigns;
    EmailDeliveryStats overallStats;
};

#endif // ANALYTICSDASHBOARD_H
