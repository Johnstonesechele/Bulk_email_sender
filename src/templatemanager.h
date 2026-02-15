#ifndef TEMPLATEMANAGER_H
#define TEMPLATEMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include "emailtemplate.h"

class QSqlDatabase;

class TemplateManager : public QObject
{
    Q_OBJECT

public:
    explicit TemplateManager(QObject *parent = nullptr);
    ~TemplateManager();

    // Template management
    bool saveTemplate(EmailTemplate* emailTemplate);
    bool updateTemplate(EmailTemplate* emailTemplate);
    bool deleteTemplate(const QString &templateId);
    EmailTemplate* getTemplate(const QString &templateId);
    QList<EmailTemplate*> getAllTemplates();
    QList<EmailTemplate*> getTemplatesByType(TemplateType type);
    QList<EmailTemplate*> getTemplatesByCategory(const QString &category);
    QList<EmailTemplate*> searchTemplates(const QString &searchTerm);
    
    // Built-in templates
    bool loadBuiltInTemplates();
    bool isBuiltInTemplate(const QString &templateId);
    
    // Template operations
    EmailTemplate* duplicateTemplate(const QString &templateId, const QString &newName = QString());
    bool exportTemplate(const QString &templateId, const QString &filePath);
    EmailTemplate* importTemplate(const QString &filePath);
    bool exportAllTemplates(const QString &filePath);
    bool importTemplatesFromFile(const QString &filePath);
    
    // Database operations
    bool initializeDatabase();
    bool createTemplateTable();
    
    // Statistics
    int getTemplateCount();
    int getTemplateCountByType(TemplateType type);
    QStringList getAvailableCategories();
    QStringList getAvailableTags();
    
    // Validation
    bool validateTemplateData(const EmailTemplate* emailTemplate);
    QStringList getTemplateErrors(const EmailTemplate* emailTemplate);

signals:
    void templateAdded(const QString &templateId);
    void templateUpdated(const QString &templateId);
    void templateDeleted(const QString &templateId);
    void templatesLoaded();
    void errorOccurred(const QString &error);

private slots:
    void onTemplateChanged();

private:
    QList<EmailTemplate*> templates;
    QMap<QString, EmailTemplate*> templateMap;
    QSqlDatabase* database;
    QString databasePath;
    
    void loadTemplatesFromDatabase();
    bool saveTemplateToDatabase(const EmailTemplate* emailTemplate);
    bool updateTemplateInDatabase(const EmailTemplate* emailTemplate);
    bool deleteTemplateFromDatabase(const QString &templateId);
    EmailTemplate* loadTemplateFromDatabase(const QString &templateId);
    
    void addTemplateToMemory(EmailTemplate* emailTemplate);
    void removeTemplateFromMemory(const QString &templateId);
    void clearTemplates();
    
    QString generateUniqueTemplateName(const QString &baseName);
    QString typeToString(TemplateType type);
    TemplateType stringToType(const QString &typeStr);
};

#endif // TEMPLATEMANAGER_H
