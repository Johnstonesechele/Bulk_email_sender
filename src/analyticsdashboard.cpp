#include "analyticsdashboard.h"
#include <QApplication>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDateTime>
#include <QScrollArea>

// MetricWidget Implementation
MetricWidget::MetricWidget(const QString &title, QWidget *parent)
    : QFrame(parent), accentColor("#4CAF50")
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setStyleSheet(R"(
        QFrame {
            background-color: #2b2b2b;
            border: 1px solid #555;
            border-radius: 8px;
            padding: 10px;
        }
    )");
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    
    titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 12px; color: #aaa; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    valueLabel = new QLabel("--");
    valueLabel->setStyleSheet("font-size: 24px; color: white; font-weight: bold;");
    valueLabel->setAlignment(Qt::AlignCenter);
    
    subtitleLabel = new QLabel("");
    subtitleLabel->setStyleSheet("font-size: 10px; color: #888;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    layout->addWidget(subtitleLabel);
    
    setMinimumSize(150, 100);
}

void MetricWidget::setValue(const QString &value)
{
    valueLabel->setText(value);
}

void MetricWidget::setSubtitle(const QString &subtitle)
{
    subtitleLabel->setText(subtitle);
    subtitleLabel->setVisible(!subtitle.isEmpty());
}

void MetricWidget::setColor(const QColor &color)
{
    accentColor = color;
    valueLabel->setStyleSheet(QString("font-size: 24px; color: %1; font-weight: bold;").arg(color.name()));
}

// AnalyticsDashboard Implementation
AnalyticsDashboard::AnalyticsDashboard(QWidget *parent)
    : QWidget(parent)
    , analytics(nullptr)
    , refreshTimer(new QTimer(this))
    , autoRefreshEnabled(false)
{
    setupUI();
    
    connect(refreshTimer, &QTimer::timeout, this, &AnalyticsDashboard::autoRefresh);
}

AnalyticsDashboard::~AnalyticsDashboard()
{
}

void AnalyticsDashboard::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    
    setupControls();
    setupMetricsSection();
    setupChartsSection();
    setupTableSection();
    
    setStyleSheet(R"(
        QWidget {
            background-color: #1a2332;
            color: white;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #555;
            border-radius: 8px;
            padding-top: 10px;
            margin-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 5px;
            color: #FFD700;
        }
    )");
}

void AnalyticsDashboard::setupControls()
{
    controlsLayout = new QHBoxLayout();
    
    // Campaign filter
    controlsLayout->addWidget(new QLabel("Campaign:"));
    campaignFilterCombo = new QComboBox();
    campaignFilterCombo->addItem("All Campaigns", "");
    controlsLayout->addWidget(campaignFilterCombo);
    
    controlsLayout->addSpacing(20);
    
    // Date range
    controlsLayout->addWidget(new QLabel("From:"));
    startDateEdit = new QDateEdit(QDate::currentDate().addDays(-30));
    startDateEdit->setCalendarPopup(true);
    controlsLayout->addWidget(startDateEdit);
    
    controlsLayout->addWidget(new QLabel("To:"));
    endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);
    controlsLayout->addWidget(endDateEdit);
    
    controlsLayout->addSpacing(20);
    
    // Action buttons
    refreshButton = new QPushButton("Refresh");
    refreshButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    controlsLayout->addWidget(refreshButton);
    
    exportButton = new QPushButton("Export Report");
    exportButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    controlsLayout->addWidget(exportButton);
    
    controlsLayout->addStretch();
    
    mainLayout->addLayout(controlsLayout);
    
    // Connect signals
    connect(campaignFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &AnalyticsDashboard::onCampaignFilterChanged);
    connect(startDateEdit, &QDateEdit::dateChanged, this, &AnalyticsDashboard::onDateRangeChanged);
    connect(endDateEdit, &QDateEdit::dateChanged, this, &AnalyticsDashboard::onDateRangeChanged);
    connect(refreshButton, &QPushButton::clicked, this, &AnalyticsDashboard::refreshData);
    connect(exportButton, &QPushButton::clicked, this, &AnalyticsDashboard::exportReport);
}

void AnalyticsDashboard::setupMetricsSection()
{
    QGroupBox *metricsGroup = new QGroupBox("Key Metrics");
    metricsLayout = new QHBoxLayout(metricsGroup);
    
    totalSentMetric = new MetricWidget("Total Emails Sent");
    deliveryRateMetric = new MetricWidget("Delivery Rate");
    deliveryRateMetric->setColor(QColor("#4CAF50"));
    
    bounceRateMetric = new MetricWidget("Bounce Rate");
    bounceRateMetric->setColor(QColor("#f44336"));
    
    activeCampaignsMetric = new MetricWidget("Active Campaigns");
    activeCampaignsMetric->setColor(QColor("#2196F3"));
    
    metricsLayout->addWidget(totalSentMetric);
    metricsLayout->addWidget(deliveryRateMetric);
    metricsLayout->addWidget(bounceRateMetric);
    metricsLayout->addWidget(activeCampaignsMetric);
    
    mainLayout->addWidget(metricsGroup);
}

void AnalyticsDashboard::setupChartsSection()
{
    QGroupBox *chartsGroup = new QGroupBox("Analytics Charts");
    chartsLayout = new QGridLayout(chartsGroup);
    
    // Create placeholder charts (will be populated when data is available)
    deliveryRateChartView = new QChartView();
    deliveryRateChartView->setMinimumHeight(300);
    
    campaignPerformanceChartView = new QChartView();
    campaignPerformanceChartView->setMinimumHeight(300);
    
    providerDistributionChartView = new QChartView();
    providerDistributionChartView->setMinimumHeight(300);
    
    timelineChartView = new QChartView();
    timelineChartView->setMinimumHeight(300);
    
    chartsLayout->addWidget(deliveryRateChartView, 0, 0);
    chartsLayout->addWidget(campaignPerformanceChartView, 0, 1);
    chartsLayout->addWidget(providerDistributionChartView, 1, 0);
    chartsLayout->addWidget(timelineChartView, 1, 1);
    
    mainLayout->addWidget(chartsGroup);
}

void AnalyticsDashboard::setupTableSection()
{
    QGroupBox *tableGroup = new QGroupBox("Campaign Summary");
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);
    
    campaignSummaryTable = new QTableWidget();
    campaignSummaryTable->setColumnCount(8);
    
    QStringList headers = {"Campaign", "Start Date", "Total Sent", "Delivered", 
                          "Failed", "Bounced", "Delivery Rate", "Bounce Rate"};
    campaignSummaryTable->setHorizontalHeaderLabels(headers);
    
    campaignSummaryTable->horizontalHeader()->setStretchLastSection(true);
    campaignSummaryTable->setAlternatingRowColors(true);
    campaignSummaryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    campaignSummaryTable->setMinimumHeight(200);
    
    tableLayout->addWidget(campaignSummaryTable);
    
    mainLayout->addWidget(tableGroup);
}

void AnalyticsDashboard::setAnalytics(Analytics *analyticsInstance)
{
    analytics = analyticsInstance;
    if (analytics) {
        refreshData();
    }
}

void AnalyticsDashboard::refreshData()
{
    if (!analytics) {
        return;
    }
    
    // Get date range
    QDateTime startDateTime = QDateTime(startDateEdit->date());
    QDateTime endDateTime = QDateTime(endDateEdit->date().addDays(1)); // Include the end date
    
    // Get campaign filter
    QString campaignFilter = campaignFilterCombo->currentData().toString();
    
    // Load data
    if (campaignFilter.isEmpty()) {
        currentCampaigns = analytics->getCampaignAnalyticsByDateRange(startDateTime, endDateTime);
        overallStats = analytics->getOverallStats();
    } else {
        CampaignAnalytics singleCampaign = analytics->getCampaignAnalytics(campaignFilter);
        currentCampaigns = {singleCampaign};
        overallStats = singleCampaign.deliveryStats;
    }
    
    // Update UI components
    updateMetrics();
    updateCharts();
    updateTable();
    
    // Update campaign filter combo if needed
    if (campaignFilterCombo->count() == 1) { // Only "All Campaigns"
        QVector<CampaignAnalytics> allCampaigns = analytics->getAllCampaignAnalytics();
        for (const CampaignAnalytics &campaign : allCampaigns) {
            campaignFilterCombo->addItem(campaign.campaignName, campaign.campaignId);
        }
    }
}

void AnalyticsDashboard::updateMetrics()
{
    totalSentMetric->setValue(QString::number(overallStats.totalSent));
    totalSentMetric->setSubtitle("emails sent");
    
    deliveryRateMetric->setValue(QString("%1%").arg(overallStats.deliveryRate, 0, 'f', 1));
    deliveryRateMetric->setSubtitle("delivery rate");
    
    bounceRateMetric->setValue(QString("%1%").arg(overallStats.bounceRate, 0, 'f', 1));
    bounceRateMetric->setSubtitle("bounce rate");
    
    activeCampaignsMetric->setValue(QString::number(currentCampaigns.size()));
    activeCampaignsMetric->setSubtitle("campaigns");
}

void AnalyticsDashboard::updateCharts()
{
    createDeliveryRateChart();
    createCampaignPerformanceChart();
    createProviderDistributionChart();
    createTimelineChart();
}

void AnalyticsDashboard::createDeliveryRateChart()
{
    QChart *chart = new QChart();
    chart->setTitle("Delivery Rate by Campaign");
    chart->setTheme(QChart::ChartThemeDark);
    
    QBarSeries *series = new QBarSeries();
    QBarSet *deliveredSet = new QBarSet("Delivered");
    QBarSet *failedSet = new QBarSet("Failed");
    QBarSet *bouncedSet = new QBarSet("Bounced");
    
    deliveredSet->setColor(QColor("#4CAF50"));
    failedSet->setColor(QColor("#FF9800"));
    bouncedSet->setColor(QColor("#f44336"));
    
    QStringList categories;
    
    for (const CampaignAnalytics &campaign : currentCampaigns) {
        if (currentCampaigns.size() <= 10) { // Limit to avoid crowding
            categories << campaign.campaignName;
            *deliveredSet << campaign.deliveryStats.successful;
            *failedSet << campaign.deliveryStats.failed;
            *bouncedSet << campaign.deliveryStats.bounced;
        }
    }
    
    series->append(deliveredSet);
    series->append(failedSet);
    series->append(bouncedSet);
    
    chart->addSeries(series);
    
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    deliveryRateChartView->setChart(chart);
}

void AnalyticsDashboard::createCampaignPerformanceChart()
{
    QChart *chart = new QChart();
    chart->setTitle("Campaign Performance Over Time");
    chart->setTheme(QChart::ChartThemeDark);
    
    QLineSeries *deliveryRateSeries = new QLineSeries();
    deliveryRateSeries->setName("Delivery Rate %");
    deliveryRateSeries->setColor(QColor("#4CAF50"));
    
    for (const CampaignAnalytics &campaign : currentCampaigns) {
        if (campaign.deliveryStats.campaignStartTime.isValid()) {
            qint64 timestamp = campaign.deliveryStats.campaignStartTime.toMSecsSinceEpoch();
            deliveryRateSeries->append(timestamp, campaign.deliveryStats.deliveryRate);
        }
    }
    
    chart->addSeries(deliveryRateSeries);
    
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("MMM dd");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);
    deliveryRateSeries->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Delivery Rate (%)");
    axisY->setRange(0, 100);
    chart->addAxis(axisY, Qt::AlignLeft);
    deliveryRateSeries->attachAxis(axisY);
    
    campaignPerformanceChartView->setChart(chart);
}

void AnalyticsDashboard::createProviderDistributionChart()
{
    QChart *chart = new QChart();
    chart->setTitle("Email Provider Distribution");
    chart->setTheme(QChart::ChartThemeDark);
    
    QPieSeries *series = new QPieSeries();
    
    if (analytics) {
        QMap<QString, double> providerRates = analytics->getProviderDeliveryRates();
        
        QColor colors[] = {
            QColor("#4CAF50"), QColor("#2196F3"), QColor("#FF9800"), 
            QColor("#9C27B0"), QColor("#F44336"), QColor("#795548")
        };
        int colorIndex = 0;
        
        for (auto it = providerRates.constBegin(); it != providerRates.constEnd(); ++it) {
            QPieSlice *slice = series->append(it.key(), it.value());
            slice->setColor(colors[colorIndex % 6]);
            slice->setLabelVisible(true);
            slice->setLabel(QString("%1 (%2%)").arg(it.key()).arg(it.value(), 0, 'f', 1));
            colorIndex++;
        }
    }
    
    chart->addSeries(series);
    providerDistributionChartView->setChart(chart);
}

void AnalyticsDashboard::createTimelineChart()
{
    QChart *chart = new QChart();
    chart->setTitle("Email Volume Timeline");
    chart->setTheme(QChart::ChartThemeDark);
    
    QLineSeries *volumeSeries = new QLineSeries();
    volumeSeries->setName("Emails Sent");
    volumeSeries->setColor(QColor("#2196F3"));
    
    // Group emails by day
    QMap<QDate, int> dailyVolume;
    for (const CampaignAnalytics &campaign : currentCampaigns) {
        for (const QDateTime &timestamp : campaign.deliveryTimestamps) {
            QDate date = timestamp.date();
            dailyVolume[date]++;
        }
    }
    
    for (auto it = dailyVolume.constBegin(); it != dailyVolume.constEnd(); ++it) {
        qint64 timestamp = QDateTime(it.key()).toMSecsSinceEpoch();
        volumeSeries->append(timestamp, it.value());
    }
    
    chart->addSeries(volumeSeries);
    
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("MMM dd");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);
    volumeSeries->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Number of Emails");
    chart->addAxis(axisY, Qt::AlignLeft);
    volumeSeries->attachAxis(axisY);
    
    timelineChartView->setChart(chart);
}

void AnalyticsDashboard::updateTable()
{
    campaignSummaryTable->setRowCount(currentCampaigns.size());
    
    for (int i = 0; i < currentCampaigns.size(); ++i) {
        const CampaignAnalytics &campaign = currentCampaigns[i];
        const EmailDeliveryStats &stats = campaign.deliveryStats;
        
        campaignSummaryTable->setItem(i, 0, new QTableWidgetItem(campaign.campaignName));
        campaignSummaryTable->setItem(i, 1, new QTableWidgetItem(stats.campaignStartTime.toString("yyyy-MM-dd")));
        campaignSummaryTable->setItem(i, 2, new QTableWidgetItem(QString::number(stats.totalSent)));
        campaignSummaryTable->setItem(i, 3, new QTableWidgetItem(QString::number(stats.successful)));
        campaignSummaryTable->setItem(i, 4, new QTableWidgetItem(QString::number(stats.failed)));
        campaignSummaryTable->setItem(i, 5, new QTableWidgetItem(QString::number(stats.bounced)));
        campaignSummaryTable->setItem(i, 6, new QTableWidgetItem(QString("%1%").arg(stats.deliveryRate, 0, 'f', 1)));
        campaignSummaryTable->setItem(i, 7, new QTableWidgetItem(QString("%1%").arg(stats.bounceRate, 0, 'f', 1)));
    }
    
    campaignSummaryTable->resizeColumnsToContents();
}

void AnalyticsDashboard::onCampaignFilterChanged()
{
    refreshData();
}

void AnalyticsDashboard::onDateRangeChanged()
{
    refreshData();
}

void AnalyticsDashboard::exportReport()
{
    if (!analytics) {
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, 
                                                   "Export Analytics Report",
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + 
                                                   QString("/analytics_report_%1.txt").arg(QDate::currentDate().toString("yyyy-MM-dd")),
                                                   "Text Files (*.txt);;JSON Files (*.json);;CSV Files (*.csv)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QString campaignId = campaignFilterCombo->currentData().toString();
    bool success = false;
    
    if (fileName.endsWith(".json")) {
        success = analytics->exportAnalyticsToJson(fileName, campaignId);
    } else if (fileName.endsWith(".csv")) {
        success = analytics->exportAnalyticsToCSV(fileName, campaignId);
    } else {
        // Text format
        QString report = analytics->generateAnalyticsReport(campaignId);
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << report;
            success = true;
        }
    }
    
    if (success) {
        QMessageBox::information(this, "Export Successful", 
                                QString("Analytics report exported to:\n%1").arg(fileName));
    } else {
        QMessageBox::warning(this, "Export Failed", "Failed to export analytics report.");
    }
}

void AnalyticsDashboard::scheduleAutoRefresh(bool enabled, int intervalMinutes)
{
    autoRefreshEnabled = enabled;
    
    if (enabled) {
        refreshTimer->start(intervalMinutes * 60 * 1000); // Convert to milliseconds
    } else {
        refreshTimer->stop();
    }
}

void AnalyticsDashboard::autoRefresh()
{
    if (autoRefreshEnabled) {
        refreshData();
    }
}
