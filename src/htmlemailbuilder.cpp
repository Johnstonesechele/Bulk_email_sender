#include "htmlemailbuilder.h"
#include <QRegularExpression>
#include <QUrl>
#include <QDebug>

HtmlEmailBuilder::HtmlEmailBuilder(QObject *parent) : QObject(parent)
{
    initializeEmailStyles();
    
    supportedEmailClients = {
        "Gmail", "Outlook", "Apple Mail", "Yahoo Mail", 
        "Thunderbird", "Hotmail", "AOL Mail"
    };
}

void HtmlEmailBuilder::initializeEmailStyles()
{
    baseEmailCss = R"(
        /* Reset styles */
        body, table, td, p, a, li, blockquote { 
            -webkit-text-size-adjust: 100%; 
            -ms-text-size-adjust: 100%; 
        }
        
        /* Email container */
        .email-container {
            max-width: 600px;
            margin: 0 auto;
            background-color: #ffffff;
            font-family: Arial, Helvetica, sans-serif;
            line-height: 1.6;
            color: #333333;
        }
        
        /* Header styles */
        .email-header {
            background-color: #f8f9fa;
            padding: 20px;
            text-align: center;
            border-bottom: 1px solid #dee2e6;
        }
        
        /* Content area */
        .email-content {
            padding: 30px 20px;
        }
        
        /* Footer styles */
        .email-footer {
            background-color: #f8f9fa;
            padding: 20px;
            text-align: center;
            font-size: 12px;
            color: #6c757d;
            border-top: 1px solid #dee2e6;
        }
        
        /* Button styles */
        .email-button {
            display: inline-block;
            padding: 12px 30px;
            background-color: #007bff;
            color: #ffffff !important;
            text-decoration: none;
            border-radius: 5px;
            font-weight: bold;
            margin: 15px 0;
        }
        
        /* Responsive styles */
        @media only screen and (max-width: 600px) {
            .email-container { width: 100% !important; }
            .email-content { padding: 20px 15px !important; }
            .email-button { display: block !important; width: 90% !important; }
        }
    )";
    
    responsiveBreakpoints = R"(
        @media only screen and (max-width: 480px) {
            .hide-mobile { display: none !important; }
            .show-mobile { display: block !important; }
            .full-width-mobile { width: 100% !important; }
            .center-mobile { text-align: center !important; }
        }
    )";
}

QString HtmlEmailBuilder::generateHtmlEmail(const QString &templateContent, const QMap<QString, QString> &variables)
{
    QString html = templateContent;
    
    // Replace variables
    html = replaceVariables(html, variables);
    
    // Wrap in responsive container
    html = createResponsiveWrapper(html);
    
    // Inline CSS for email client compatibility
    html = inlineCss(html);
    
    // Optimize for various email clients
    html = createOutlookCompatibleHtml(html);
    html = createMobileCompatibleHtml(html);
    
    return html;
}

QString HtmlEmailBuilder::createResponsiveWrapper(const QString &content)
{
    QString html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <title>Email</title>
    <!--[if mso]>
    <noscript>
        <xml>
            <o:OfficeDocumentSettings>
                <o:PixelsPerInch>96</o:PixelsPerInch>
            </o:OfficeDocumentSettings>
        </xml>
    </noscript>
    <![endif]-->
    <style>
        %1
        %2
    </style>
</head>
<body style="margin: 0; padding: 0; background-color: #f4f4f4;">
    <!--[if mso]>
    <table role="presentation" cellpadding="0" cellspacing="0" border="0" width="100%%">
    <tr>
    <td>
    <![endif]-->
    
    <div class="email-container">
        %3
    </div>
    
    <!--[if mso]>
    </td>
    </tr>
    </table>
    <![endif]-->
</body>
</html>)";
    
    return html.arg(baseEmailCss, responsiveBreakpoints, content);
}

QString HtmlEmailBuilder::createHeader(const QString &logoUrl, const QString &companyName, const QString &headerColor)
{
    QString header = R"(
    <div class="email-header" style="background-color: %3; padding: 20px; text-align: center;">)";
    
    if (!logoUrl.isEmpty()) {
        header += QString(R"(
        <img src="%1" alt="%2 Logo" style="max-height: 60px; margin-bottom: 10px;">)").arg(logoUrl, companyName);
    }
    
    if (!companyName.isEmpty()) {
        header += QString(R"(
        <h1 style="margin: 0; color: #333333; font-size: 24px;">%1</h1>)").arg(companyName);
    }
    
    header += "\n    </div>";
    
    return header.arg(logoUrl, companyName, headerColor);
}

QString HtmlEmailBuilder::createFooter(const QString &companyName, const QString &address, const QString &unsubscribeLink)
{
    QString footer = R"(
    <div class="email-footer" style="background-color: #f8f9fa; padding: 20px; text-align: center; font-size: 12px; color: #6c757d;">)";
    
    if (!companyName.isEmpty()) {
        footer += QString(R"(
        <p style="margin: 0 0 10px 0;"><strong>%1</strong></p>)").arg(companyName);
    }
    
    if (!address.isEmpty()) {
        footer += QString(R"(
        <p style="margin: 0 0 15px 0;">%1</p>)").arg(address);
    }
    
    if (!unsubscribeLink.isEmpty()) {
        footer += QString(R"(
        <p style="margin: 0;">
            <a href="%1" style="color: #6c757d; text-decoration: underline;">Unsubscribe</a> | 
            <a href="mailto:support@%2" style="color: #6c757d; text-decoration: underline;">Contact Support</a>
        </p>)").arg(unsubscribeLink, companyName.toLower().replace(" ", ""));
    }
    
    footer += "\n    </div>";
    
    return footer;
}

QString HtmlEmailBuilder::createButton(const QString &text, const QString &url, const QString &color, const QString &textColor)
{
    return QString(R"(
    <div style="text-align: center; margin: 20px 0;">
        <a href="%2" class="email-button" style="display: inline-block; padding: 12px 30px; background-color: %3; color: %4 !important; text-decoration: none; border-radius: 5px; font-weight: bold;">%1</a>
    </div>)").arg(text, url, color, textColor);
}

QString HtmlEmailBuilder::createImage(const QString &src, const QString &alt, const QString &width, const QString &height)
{
    QString style = QString("max-width: 100%%; height: auto;");
    if (width != "auto") style += QString(" width: %1;").arg(width);
    if (height != "auto") style += QString(" height: %1;").arg(height);
    
    return QString(R"(
    <div style="text-align: center; margin: 15px 0;">
        <img src="%1" alt="%2" style="%3">
    </div>)").arg(src, alt, style);
}

QString HtmlEmailBuilder::createDivider(const QString &color, const QString &thickness)
{
    return QString(R"(
    <hr style="border: none; border-top: %2 solid %1; margin: 20px 0;">)").arg(color, thickness);
}

QString HtmlEmailBuilder::createSpacer(const QString &height)
{
    return QString(R"(
    <div style="height: %1; line-height: %1;">&nbsp;</div>)").arg(height);
}

QString HtmlEmailBuilder::createSocialLinks(const QMap<QString, QString> &socialLinks)
{
    if (socialLinks.isEmpty()) return "";
    
    QString social = R"(
    <div style="text-align: center; margin: 20px 0;">
        <p style="margin: 0 0 10px 0; font-size: 14px; color: #6c757d;">Follow us on social media:</p>
        <div>)";
    
    QMapIterator<QString, QString> it(socialLinks);
    while (it.hasNext()) {
        it.next();
        social += QString(R"(
            <a href="%2" style="display: inline-block; margin: 0 10px; text-decoration: none;">
                <img src="https://via.placeholder.com/32x32/%1.png" alt="%1" style="width: 32px; height: 32px;">
            </a>)").arg(it.key().toLower(), it.value());
    }
    
    social += R"(
        </div>
    </div>)";
    
    return social;
}

QString HtmlEmailBuilder::createUnsubscribeSection(const QString &unsubscribeUrl, const QString &preferencesUrl)
{
    QString unsubscribe = R"(
    <div style="text-align: center; margin: 30px 0; padding: 20px; background-color: #f8f9fa; border-radius: 5px;">
        <p style="margin: 0 0 10px 0; font-size: 12px; color: #6c757d;">
            You're receiving this email because you subscribed to our mailing list.
        </p>)";
    
    if (!preferencesUrl.isEmpty()) {
        unsubscribe += QString(R"(
        <p style="margin: 0; font-size: 12px;">
            <a href="%1" style="color: #007bff; text-decoration: underline;">Update Preferences</a> | 
            <a href="%2" style="color: #6c757d; text-decoration: underline;">Unsubscribe</a>
        </p>)").arg(preferencesUrl, unsubscribeUrl);
    } else {
        unsubscribe += QString(R"(
        <p style="margin: 0; font-size: 12px;">
            <a href="%1" style="color: #6c757d; text-decoration: underline;">Unsubscribe from this list</a>
        </p>)").arg(unsubscribeUrl);
    }
    
    unsubscribe += "\n    </div>";
    
    return unsubscribe;
}

QString HtmlEmailBuilder::replaceVariables(const QString &html, const QMap<QString, QString> &variables)
{
    QString result = html;
    
    QMapIterator<QString, QString> it(variables);
    while (it.hasNext()) {
        it.next();
        QString placeholder = QString("{{%1}}").arg(it.key());
        result.replace(placeholder, it.value(), Qt::CaseInsensitive);
    }
    
    return result;
}

QString HtmlEmailBuilder::inlineCss(const QString &html)
{
    // Basic CSS inlining for email compatibility
    QString result = html;
    
    // Replace class-based styles with inline styles for better email client support
    result.replace("class=\"email-button\"", 
        "style=\"display: inline-block; padding: 12px 30px; background-color: #007bff; color: #ffffff !important; text-decoration: none; border-radius: 5px; font-weight: bold; margin: 15px 0;\"");
    
    result.replace("class=\"email-header\"", 
        "style=\"background-color: #f8f9fa; padding: 20px; text-align: center; border-bottom: 1px solid #dee2e6;\"");
    
    result.replace("class=\"email-content\"", 
        "style=\"padding: 30px 20px;\"");
    
    result.replace("class=\"email-footer\"", 
        "style=\"background-color: #f8f9fa; padding: 20px; text-align: center; font-size: 12px; color: #6c757d; border-top: 1px solid #dee2e6;\"");
    
    return result;
}

QString HtmlEmailBuilder::createOutlookCompatibleHtml(const QString &html)
{
    QString result = html;
    
    // Add Outlook-specific conditional comments and table-based layout
    result.replace("<div class=\"email-container\"", 
        "<!--[if mso]><table role=\"presentation\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\" width=\"600\"><tr><td><![endif]-->"
        "<div class=\"email-container\"");
    
    result.replace("</div>", 
        "</div>"
        "<!--[if mso]></td></tr></table><![endif]-->");
    
    return result;
}

QString HtmlEmailBuilder::createMobileCompatibleHtml(const QString &html)
{
    QString result = html;
    
    // Ensure mobile viewport meta tag is present
    if (!result.contains("viewport")) {
        result.replace("<head>", 
            "<head>\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
    }
    
    return result;
}

QString HtmlEmailBuilder::sanitizeHtml(const QString &html)
{
    QString sanitized = html;
    
    // Remove potentially dangerous elements
    QRegularExpression scriptTags("<script[^>]*>.*?</script>", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    sanitized.remove(scriptTags);
    
    QRegularExpression styleTags("<style[^>]*>.*?</style>", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    sanitized.remove(styleTags);
    
    // Remove javascript: URLs
    QRegularExpression jsUrls("javascript:", QRegularExpression::CaseInsensitiveOption);
    sanitized.remove(jsUrls);
    
    return sanitized;
}

bool HtmlEmailBuilder::validateHtmlEmail(const QString &html)
{
    // Check for required elements
    if (!html.contains("<!DOCTYPE", Qt::CaseInsensitive)) return false;
    if (!html.contains("<html", Qt::CaseInsensitive)) return false;
    if (!html.contains("<head>", Qt::CaseInsensitive)) return false;
    if (!html.contains("<body", Qt::CaseInsensitive)) return false;
    
    // Check for email-specific requirements
    if (!html.contains("charset=", Qt::CaseInsensitive)) return false;
    
    return true;
}

QStringList HtmlEmailBuilder::checkEmailCompatibility(const QString &html)
{
    QStringList issues;
    
    if (html.contains("<script", Qt::CaseInsensitive)) {
        issues << "JavaScript is not supported in most email clients";
    }
    
    if (html.contains("position: absolute", Qt::CaseInsensitive) || 
        html.contains("position: fixed", Qt::CaseInsensitive)) {
        issues << "Absolute/fixed positioning may not work in email clients";
    }
    
    if (html.contains("background-image", Qt::CaseInsensitive)) {
        issues << "Background images may not display in some email clients";
    }
    
    if (html.contains("@media", Qt::CaseInsensitive) && 
        !html.contains("only screen", Qt::CaseInsensitive)) {
        issues << "Media queries should include 'only screen' for better compatibility";
    }
    
    return issues;
}

QStringList HtmlEmailBuilder::extractImages(const QString &html)
{
    QStringList images;
    QRegularExpression imgRegex("<img[^>]+src=[\"']([^\"']+)[\"'][^>]*>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = imgRegex.globalMatch(html);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        images << match.captured(1);
    }
    
    return images;
}
