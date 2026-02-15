#ifndef HTMLEMAILBUILDER_H
#define HTMLEMAILBUILDER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QStringList>
#include <QJsonObject>

class HtmlEmailBuilder : public QObject
{
    Q_OBJECT

public:
    explicit HtmlEmailBuilder(QObject *parent = nullptr);
    
    // HTML Email Structure
    enum class EmailSection {
        Header,
        Body,
        Footer,
        Sidebar
    };
    
    enum class ContentBlock {
        Text,
        Image,
        Button,
        Divider,
        Spacer,
        Table,
        List,
        Social,
        Unsubscribe
    };
    
    // Email Building Methods
    QString generateHtmlEmail(const QString &templateContent, const QMap<QString, QString> &variables);
    QString createResponsiveWrapper(const QString &content);
    QString addEmailStyles();
    QString createContentBlock(ContentBlock type, const QMap<QString, QString> &properties);
    
    // Template Components
    QString createHeader(const QString &logoUrl = "", const QString &companyName = "", const QString &headerColor = "#ffffff");
    QString createFooter(const QString &companyName = "", const QString &address = "", const QString &unsubscribeLink = "");
    QString createButton(const QString &text, const QString &url, const QString &color = "#007bff", const QString &textColor = "#ffffff");
    QString createImage(const QString &src, const QString &alt = "", const QString &width = "auto", const QString &height = "auto");
    QString createDivider(const QString &color = "#e0e0e0", const QString &thickness = "1px");
    QString createSpacer(const QString &height = "20px");
    QString createSocialLinks(const QMap<QString, QString> &socialLinks);
    QString createUnsubscribeSection(const QString &unsubscribeUrl, const QString &preferencesUrl = "");
    
    // Email Validation & Testing
    bool validateHtmlEmail(const QString &html);
    QStringList checkEmailCompatibility(const QString &html);
    QString optimizeForMobile(const QString &html);
    QString inlineCss(const QString &html);
    
    // Template Utilities
    QStringList extractImages(const QString &html);
    QString replaceVariables(const QString &html, const QMap<QString, QString> &variables);
    QString sanitizeHtml(const QString &html);
    
    // Email Client Compatibility
    QString createOutlookCompatibleHtml(const QString &html);
    QString createGmailCompatibleHtml(const QString &html);
    QString createMobileCompatibleHtml(const QString &html);

private:
    QString baseEmailCss;
    QString responsiveBreakpoints;
    QStringList supportedEmailClients;
    
    void initializeEmailStyles();
    QString wrapInTable(const QString &content, const QString &attributes = "");
    QString addMediaQueries();
    QString createMetaTags();
};

#endif // HTMLEMAILBUILDER_H
