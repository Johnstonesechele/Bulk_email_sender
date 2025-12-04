#include "emailtemplate.h"
#include <QUuid>
#include <QRegularExpression>
#include <QJsonArray>

EmailTemplate::EmailTemplate(QObject *parent)
    : QObject(parent), type(TemplateType::Custom), htmlBuilder(nullptr)
{
    generateId();
    createdDate = QDateTime::currentDateTime();
    modifiedDate = createdDate;
    htmlBuilder = new HtmlEmailBuilder(this);
}

EmailTemplate::EmailTemplate(const QString &id, const QString &name, const QString &subject,
                           const QString &htmlContent, const QString &textContent,
                           TemplateType type, QObject *parent)
    : QObject(parent), id(id), name(name), subject(subject), 
      htmlContent(htmlContent), textContent(textContent), type(type), htmlBuilder(nullptr)
{
    if (this->id.isEmpty()) {
        generateId();
    }
    createdDate = QDateTime::currentDateTime();
    modifiedDate = createdDate;
    htmlBuilder = new HtmlEmailBuilder(this);
}

void EmailTemplate::setName(const QString &name)
{
    if (this->name != name) {
        this->name = name;
        updateModifiedDate();
        emit templateChanged();
    }
}

void EmailTemplate::setSubject(const QString &subject)
{
    if (this->subject != subject) {
        this->subject = subject;
        updateModifiedDate();
        emit templateChanged();
    }
}

void EmailTemplate::setHtmlContent(const QString &content)
{
    if (this->htmlContent != content) {
        this->htmlContent = content;
        updateModifiedDate();
        emit templateChanged();
    }
}

void EmailTemplate::setTextContent(const QString &content)
{
    if (this->textContent != content) {
        this->textContent = content;
        updateModifiedDate();
        emit templateChanged();
    }
}

void EmailTemplate::setType(TemplateType type)
{
    if (this->type != type) {
        this->type = type;
        updateModifiedDate();
        emit templateChanged();
    }
}

void EmailTemplate::setDescription(const QString &description)
{
    if (this->description != description) {
        this->description = description;
        updateModifiedDate();
        emit templateChanged();
    }
}

void EmailTemplate::setCategory(const QString &category)
{
    if (this->category != category) {
        this->category = category;
        updateModifiedDate();
        emit templateChanged();
    }
}

void EmailTemplate::setTags(const QStringList &tags)
{
    if (this->tags != tags) {
        this->tags = tags;
        updateModifiedDate();
        emit templateChanged();
    }
}

QString EmailTemplate::processTemplate(const QMap<QString, QString> &variables) const
{
    return processVariables(htmlContent, variables);
}

QString EmailTemplate::processSubject(const QMap<QString, QString> &variables) const
{
    return processVariables(subject, variables);
}

QStringList EmailTemplate::extractVariables() const
{
    QStringList variables;
    QRegularExpression regex(R"(\{\{([^}]+)\}\})");
    
    // Extract from subject
    QRegularExpressionMatchIterator iterator = regex.globalMatch(subject);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString variable = match.captured(1).trimmed();
        if (!variables.contains(variable)) {
            variables.append(variable);
        }
    }
    
    // Extract from HTML content
    iterator = regex.globalMatch(htmlContent);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString variable = match.captured(1).trimmed();
        if (!variables.contains(variable)) {
            variables.append(variable);
        }
    }
    
    // Extract from text content
    iterator = regex.globalMatch(textContent);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString variable = match.captured(1).trimmed();
        if (!variables.contains(variable)) {
            variables.append(variable);
        }
    }
    
    return variables;
}

bool EmailTemplate::validateTemplate() const
{
    // Check if template has required fields
    if (name.isEmpty() || subject.isEmpty()) {
        return false;
    }
    
    // Check if at least one content type is provided
    if (htmlContent.isEmpty() && textContent.isEmpty()) {
        return false;
    }
    
    // Validate variables syntax
    QStringList variables = extractVariables();
    for (const QString &variable : variables) {
        if (!isValidVariable(variable)) {
            return false;
        }
    }
    
    return true;
}

QJsonObject EmailTemplate::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["subject"] = subject;
    json["htmlContent"] = htmlContent;
    json["textContent"] = textContent;
    json["description"] = description;
    json["category"] = category;
    json["type"] = static_cast<int>(type);
    json["createdDate"] = createdDate.toString(Qt::ISODate);
    json["modifiedDate"] = modifiedDate.toString(Qt::ISODate);
    
    QJsonArray tagsArray;
    for (const QString &tag : tags) {
        tagsArray.append(tag);
    }
    json["tags"] = tagsArray;
    
    return json;
}

bool EmailTemplate::fromJson(const QJsonObject &json)
{
    if (!json.contains("id") || !json.contains("name") || !json.contains("subject")) {
        return false;
    }
    
    id = json["id"].toString();
    name = json["name"].toString();
    subject = json["subject"].toString();
    htmlContent = json["htmlContent"].toString();
    textContent = json["textContent"].toString();
    description = json["description"].toString();
    category = json["category"].toString();
    type = static_cast<TemplateType>(json["type"].toInt());
    
    if (json.contains("createdDate")) {
        createdDate = QDateTime::fromString(json["createdDate"].toString(), Qt::ISODate);
    }
    if (json.contains("modifiedDate")) {
        modifiedDate = QDateTime::fromString(json["modifiedDate"].toString(), Qt::ISODate);
    }
    
    if (json.contains("tags")) {
        QJsonArray tagsArray = json["tags"].toArray();
        tags.clear();
        for (const QJsonValue &value : tagsArray) {
            tags.append(value.toString());
        }
    }
    
    return true;
}

QList<EmailTemplate*> EmailTemplate::getBuiltInTemplates()
{
    QList<EmailTemplate*> templates;
    templates.append(createWelcomeTemplate());
    templates.append(createNewsletterTemplate());
    templates.append(createPromotionalTemplate());
    templates.append(createEventTemplate());
    templates.append(createAnnouncementTemplate());
    return templates;
}

EmailTemplate* EmailTemplate::createWelcomeTemplate()
{
    QString htmlContent = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Welcome to {{company_name}}</title>
    <style>
        body { font-family: Arial, sans-serif; line-height: 1.6; color: #333; }
        .container { max-width: 600px; margin: 0 auto; padding: 20px; }
        .header { background-color: #4CAF50; color: white; padding: 20px; text-align: center; }
        .content { padding: 20px; background-color: #f9f9f9; }
        .button { background-color: #4CAF50; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; display: inline-block; margin: 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Welcome to {{company_name}}!</h1>
        </div>
        <div class="content">
            <h2>Hello {{first_name}}!</h2>
            <p>We're excited to have you as part of our community. Your account has been successfully created.</p>
            <p>Here are your account details:</p>
            <ul>
                <li><strong>Email:</strong> {{email}}</li>
                <li><strong>Account Type:</strong> {{account_type}}</li>
                <li><strong>Registration Date:</strong> {{registration_date}}</li>
            </ul>
            <p>To get started, please verify your email address:</p>
            <a href="{{verification_link}}" class="button">Verify Email Address</a>
            <p>If you have any questions, feel free to contact our support team.</p>
            <p>Best regards,<br>The {{company_name}} Team</p>
        </div>
    </div>
</body>
</html>)";

    QString textContent = R"(
Welcome to {{company_name}}!

Hello {{first_name}},

We're excited to have you as part of our community. Your account has been successfully created.

Account Details:
- Email: {{email}}
- Account Type: {{account_type}}
- Registration Date: {{registration_date}}

To get started, please verify your email address by visiting:
{{verification_link}}

If you have any questions, feel free to contact our support team.

Best regards,
The {{company_name}} Team
)";

    EmailTemplate* welcome = new EmailTemplate();
    welcome->setName("Welcome Email");
    welcome->setSubject("Welcome to {{company_name}}, {{first_name}}!");
    welcome->setHtmlContent(htmlContent);
    welcome->setTextContent(textContent);
    welcome->setType(TemplateType::Welcome);
    welcome->setDescription("A warm welcome email for new users");
    welcome->setCategory("Onboarding");
    welcome->setTags({"welcome", "onboarding", "verification"});
    
    return welcome;
}

EmailTemplate* EmailTemplate::createNewsletterTemplate()
{
    QString htmlContent = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>{{newsletter_title}}</title>
    <style>
        body { font-family: Arial, sans-serif; line-height: 1.6; color: #333; margin: 0; padding: 0; }
        .container { max-width: 600px; margin: 0 auto; }
        .header { background-color: #2196F3; color: white; padding: 20px; text-align: center; }
        .content { padding: 20px; }
        .article { margin-bottom: 30px; border-bottom: 1px solid #eee; padding-bottom: 20px; }
        .article h3 { color: #2196F3; }
        .footer { background-color: #f5f5f5; padding: 20px; text-align: center; font-size: 12px; }
        .button { background-color: #2196F3; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; display: inline-block; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>{{newsletter_title}}</h1>
            <p>{{newsletter_date}}</p>
        </div>
        <div class="content">
            <p>Hi {{first_name}},</p>
            <p>{{newsletter_intro}}</p>
            
            <div class="article">
                <h3>{{article_1_title}}</h3>
                <p>{{article_1_content}}</p>
                <a href="{{article_1_link}}" class="button">Read More</a>
            </div>
            
            <div class="article">
                <h3>{{article_2_title}}</h3>
                <p>{{article_2_content}}</p>
                <a href="{{article_2_link}}" class="button">Read More</a>
            </div>
            
            <div class="article">
                <h3>{{article_3_title}}</h3>
                <p>{{article_3_content}}</p>
                <a href="{{article_3_link}}" class="button">Read More</a>
            </div>
        </div>
        <div class="footer">
            <p>Thank you for subscribing to {{company_name}} Newsletter</p>
            <p><a href="{{unsubscribe_link}}">Unsubscribe</a> | <a href="{{preferences_link}}">Update Preferences</a></p>
        </div>
    </div>
</body>
</html>)";

    EmailTemplate* newsletter = new EmailTemplate();
    newsletter->setName("Newsletter Template");
    newsletter->setSubject("{{newsletter_title}} - {{newsletter_date}}");
    newsletter->setHtmlContent(htmlContent);
    newsletter->setType(TemplateType::Newsletter);
    newsletter->setDescription("Professional newsletter template with multiple articles");
    newsletter->setCategory("Marketing");
    newsletter->setTags({"newsletter", "marketing", "content"});
    
    return newsletter;
}

EmailTemplate* EmailTemplate::createPromotionalTemplate()
{
    QString htmlContent = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>{{promotion_title}}</title>
    <style>
        body { font-family: Arial, sans-serif; line-height: 1.6; color: #333; margin: 0; padding: 0; }
        .container { max-width: 600px; margin: 0 auto; }
        .header { background: linear-gradient(135deg, #FF6B6B, #FF8E53); color: white; padding: 30px; text-align: center; }
        .content { padding: 30px; }
        .highlight { background-color: #FFD93D; padding: 20px; text-align: center; margin: 20px 0; border-radius: 10px; }
        .cta-button { background-color: #FF6B6B; color: white; padding: 15px 30px; text-decoration: none; border-radius: 25px; display: inline-block; font-weight: bold; margin: 20px 0; }
        .footer { background-color: #f5f5f5; padding: 20px; text-align: center; font-size: 12px; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>{{promotion_title}}</h1>
            <p>{{promotion_subtitle}}</p>
        </div>
        <div class="content">
            <p>Hi {{first_name}},</p>
            <p>{{promotion_intro}}</p>
            
            <div class="highlight">
                <h2>{{discount_percent}}% OFF</h2>
                <p>Use code: <strong>{{promo_code}}</strong></p>
                <p>Valid until: {{expiry_date}}</p>
            </div>
            
            <p>{{promotion_details}}</p>
            
            <div style="text-align: center;">
                <a href="{{shop_link}}" class="cta-button">Shop Now</a>
            </div>
            
            <p><em>This offer is exclusive to our valued customers like you!</em></p>
        </div>
        <div class="footer">
            <p>{{company_name}} | {{company_address}}</p>
            <p><a href="{{unsubscribe_link}}">Unsubscribe</a></p>
        </div>
    </div>
</body>
</html>)";

    EmailTemplate* promotional = new EmailTemplate();
    promotional->setName("Promotional Sale");
    promotional->setSubject("🎉 {{promotion_title}} - {{discount_percent}}% OFF for {{first_name}}!");
    promotional->setHtmlContent(htmlContent);
    promotional->setType(TemplateType::Promotional);
    promotional->setDescription("Eye-catching promotional email for sales and discounts");
    promotional->setCategory("Sales");
    promotional->setTags({"promotion", "sale", "discount", "marketing"});
    
    return promotional;
}

EmailTemplate* EmailTemplate::createEventTemplate()
{
    QString htmlContent = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>{{event_name}} Invitation</title>
    <style>
        body { font-family: Arial, sans-serif; line-height: 1.6; color: #333; margin: 0; padding: 0; }
        .container { max-width: 600px; margin: 0 auto; }
        .header { background-color: #9C27B0; color: white; padding: 30px; text-align: center; }
        .content { padding: 30px; }
        .event-details { background-color: #f8f9fa; padding: 20px; border-radius: 10px; margin: 20px 0; }
        .detail-row { display: flex; justify-content: space-between; margin: 10px 0; padding: 5px 0; border-bottom: 1px solid #dee2e6; }
        .rsvp-button { background-color: #9C27B0; color: white; padding: 15px 30px; text-decoration: none; border-radius: 5px; display: inline-block; font-weight: bold; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>You're Invited!</h1>
            <h2>{{event_name}}</h2>
        </div>
        <div class="content">
            <p>Dear {{first_name}},</p>
            <p>{{event_description}}</p>
            
            <div class="event-details">
                <h3>Event Details</h3>
                <div class="detail-row">
                    <strong>Date:</strong> {{event_date}}
                </div>
                <div class="detail-row">
                    <strong>Time:</strong> {{event_time}}
                </div>
                <div class="detail-row">
                    <strong>Location:</strong> {{event_location}}
                </div>
                <div class="detail-row">
                    <strong>Duration:</strong> {{event_duration}}
                </div>
            </div>
            
            <p>{{event_agenda}}</p>
            
            <div style="text-align: center;">
                <a href="{{rsvp_link}}" class="rsvp-button">RSVP Now</a>
            </div>
            
            <p>Please RSVP by {{rsvp_deadline}} so we can prepare accordingly.</p>
            <p>We look forward to seeing you there!</p>
            <p>Best regards,<br>{{organizer_name}}</p>
        </div>
    </div>
</body>
</html>)";

    EmailTemplate* event = new EmailTemplate();
    event->setName("Event Invitation");
    event->setSubject("You're Invited: {{event_name}} on {{event_date}}");
    event->setHtmlContent(htmlContent);
    event->setType(TemplateType::Event);
    event->setDescription("Professional event invitation template");
    event->setCategory("Events");
    event->setTags({"event", "invitation", "rsvp"});
    
    return event;
}

EmailTemplate* EmailTemplate::createAnnouncementTemplate()
{
    QString htmlContent = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>{{announcement_title}}</title>
    <style>
        body { font-family: Arial, sans-serif; line-height: 1.6; color: #333; margin: 0; padding: 0; }
        .container { max-width: 600px; margin: 0 auto; }
        .header { background-color: #607D8B; color: white; padding: 30px; text-align: center; }
        .content { padding: 30px; }
        .important-box { background-color: #E3F2FD; border-left: 5px solid #2196F3; padding: 15px; margin: 20px 0; }
        .action-button { background-color: #607D8B; color: white; padding: 12px 25px; text-decoration: none; border-radius: 5px; display: inline-block; margin: 15px 0; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Important Announcement</h1>
            <p>{{announcement_date}}</p>
        </div>
        <div class="content">
            <h2>{{announcement_title}}</h2>
            <p>Dear {{first_name}},</p>
            <p>{{announcement_intro}}</p>
            
            <div class="important-box">
                <h3>Key Points:</h3>
                <ul>
                    <li>{{key_point_1}}</li>
                    <li>{{key_point_2}}</li>
                    <li>{{key_point_3}}</li>
                </ul>
            </div>
            
            <p>{{announcement_details}}</p>
            
            <div style="text-align: center;">
                <a href="{{more_info_link}}" class="action-button">Learn More</a>
            </div>
            
            <p>If you have any questions, please don't hesitate to contact us.</p>
            <p>Thank you for your attention.</p>
            <p>Best regards,<br>{{company_name}} Team</p>
        </div>
    </div>
</body>
</html>)";

    EmailTemplate* announcement = new EmailTemplate();
    announcement->setName("Company Announcement");
    announcement->setSubject("Important: {{announcement_title}}");
    announcement->setHtmlContent(htmlContent);
    announcement->setType(TemplateType::Announcement);
    announcement->setDescription("Professional announcement template for company updates");
    announcement->setCategory("Corporate");
    announcement->setTags({"announcement", "corporate", "update"});
    
    return announcement;
}

void EmailTemplate::generateId()
{
    id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void EmailTemplate::updateModifiedDate()
{
    modifiedDate = QDateTime::currentDateTime();
}

QString EmailTemplate::processVariables(const QString &content, const QMap<QString, QString> &variables) const
{
    QString result = content;
    
    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it) {
        QString placeholder = QString("{{%1}}").arg(it.key());
        result.replace(placeholder, it.value());
    }
    
    return result;
}

bool EmailTemplate::isValidVariable(const QString &variable)
{
    // Check if variable name is valid (alphanumeric, underscore, dash)
    QRegularExpression regex("^[a-zA-Z0-9_-]+$");
    return regex.match(variable).hasMatch() && !variable.isEmpty();
}

QString EmailTemplate::formatVariable(const QString &variable)
{
    return QString("{{%1}}").arg(variable.trimmed());
}
