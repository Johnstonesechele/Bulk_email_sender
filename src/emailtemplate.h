#ifndef EMAILTEMPLATE_H
#define EMAILTEMPLATE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QJsonObject>
#include "htmlemailbuilder.h"

enum class TemplateType {
    Custom,
    Newsletter,
    Promotional,
    Welcome,
    Transactional,
    Event,
    Announcement
};

class EmailTemplate : public QObject
{
    Q_OBJECT

public:
    explicit EmailTemplate(QObject *parent = nullptr);
    EmailTemplate(const QString &id, const QString &name, const QString &subject, 
                 const QString &htmlContent, const QString &textContent, 
                 TemplateType type = TemplateType::Custom, QObject *parent = nullptr);
    
    // Getters
    QString getId() const { return id; }
    QString getName() const { return name; }
    QString getSubject() const { return subject; }
    QString getHtmlContent() const { return htmlContent; }
    QString getTextContent() const { return textContent; }
    TemplateType getType() const { return type; }
    QDateTime getCreatedDate() const { return createdDate; }
    QDateTime getModifiedDate() const { return modifiedDate; }
    QString getDescription() const { return description; }
    QString getCategory() const { return category; }
    QStringList getTags() const { return tags; }
    
    // Setters
    void setName(const QString &name);
    void setSubject(const QString &subject);
    void setHtmlContent(const QString &content);
    void setTextContent(const QString &content);
    void setType(TemplateType type);
    void setDescription(const QString &description);
    void setCategory(const QString &category);
    void setTags(const QStringList &tags);
    
    // Template processing with HTML builder
    QString processTemplate(const QMap<QString, QString> &variables) const;
    QString processSubject(const QMap<QString, QString> &variables) const;
    QString generateFinalHtml(const QMap<QString, QString> &variables) const;
    QString generatePlainText(const QMap<QString, QString> &variables) const;
    
    // HTML email specific methods
    QString getOptimizedHtml() const;
    bool isHtmlTemplate() const;
    QStringList getRequiredImages() const;
    QStringList validateEmailCompatibility() const;
    QStringList extractVariables() const;
    bool validateTemplate() const;
    
    // Serialization
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);
    
    // Static methods for built-in templates
    static QList<EmailTemplate*> getBuiltInTemplates();
    static EmailTemplate* createWelcomeTemplate();
    static EmailTemplate* createNewsletterTemplate();
    static EmailTemplate* createPromotionalTemplate();
    static EmailTemplate* createEventTemplate();
    static EmailTemplate* createAnnouncementTemplate();
    
    // Template validation
    static bool isValidVariable(const QString &variable);
    static QString formatVariable(const QString &variable);

signals:
    void templateChanged();

private:
    QString id;
    QString name;
    QString subject;
    QString htmlContent;
    QString textContent;
    QString description;
    QString category;
    QStringList tags;
    TemplateType type;
    QDateTime createdDate;
    QDateTime modifiedDate;
    
    // HTML email builder instance
    mutable HtmlEmailBuilder* htmlBuilder;
    
    void generateId();
    void updateModifiedDate();
    QString processVariables(const QString &content, const QMap<QString, QString> &variables) const;
};

Q_DECLARE_METATYPE(TemplateType)

#endif // EMAILTEMPLATE_H
