#include "templatemanager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QDebug>

TemplateManager::TemplateManager(QObject *parent)
    : QObject(parent), database(nullptr)
{
    // Set up database path
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    databasePath = dataPath + "/templates.db";
    
    initializeDatabase();
    loadTemplatesFromDatabase();
    loadBuiltInTemplates();
}

TemplateManager::~TemplateManager()
{
    clearTemplates();
    if (database) {
        database->close();
        delete database;
    }
}

bool TemplateManager::saveTemplate(EmailTemplate* emailTemplate)
{
    if (!emailTemplate || !validateTemplateData(emailTemplate)) {
        emit errorOccurred("Invalid template data");
        return false;
    }
    
    if (saveTemplateToDatabase(emailTemplate)) {
        addTemplateToMemory(emailTemplate);
        connect(emailTemplate, &EmailTemplate::templateChanged, this, &TemplateManager::onTemplateChanged);
        emit templateAdded(emailTemplate->getId());
        return true;
    }
    
    return false;
}

bool TemplateManager::updateTemplate(EmailTemplate* emailTemplate)
{
    if (!emailTemplate || !validateTemplateData(emailTemplate)) {
        emit errorOccurred("Invalid template data");
        return false;
    }
    
    if (updateTemplateInDatabase(emailTemplate)) {
        emit templateUpdated(emailTemplate->getId());
        return true;
    }
    
    return false;
}

bool TemplateManager::deleteTemplate(const QString &templateId)
{
    if (isBuiltInTemplate(templateId)) {
        emit errorOccurred("Cannot delete built-in template");
        return false;
    }
    
    if (deleteTemplateFromDatabase(templateId)) {
        removeTemplateFromMemory(templateId);
        emit templateDeleted(templateId);
        return true;
    }
    
    return false;
}

EmailTemplate* TemplateManager::getTemplate(const QString &templateId)
{
    return templateMap.value(templateId, nullptr);
}

QList<EmailTemplate*> TemplateManager::getAllTemplates()
{
    return templates;
}

QList<EmailTemplate*> TemplateManager::getTemplatesByType(TemplateType type)
{
    QList<EmailTemplate*> result;
    for (EmailTemplate* emailTemplate : templates) {
        if (emailTemplate->getType() == type) {
            result.append(emailTemplate);
        }
    }
    return result;
}

QList<EmailTemplate*> TemplateManager::getTemplatesByCategory(const QString &category)
{
    QList<EmailTemplate*> result;
    for (EmailTemplate* emailTemplate : templates) {
        if (emailTemplate->getCategory().compare(category, Qt::CaseInsensitive) == 0) {
            result.append(emailTemplate);
        }
    }
    return result;
}

QList<EmailTemplate*> TemplateManager::searchTemplates(const QString &searchTerm)
{
    QList<EmailTemplate*> result;
    QString term = searchTerm.toLower();
    
    for (EmailTemplate* emailTemplate : templates) {
        bool matches = emailTemplate->getName().toLower().contains(term) ||
                      emailTemplate->getSubject().toLower().contains(term) ||
                      emailTemplate->getDescription().toLower().contains(term) ||
                      emailTemplate->getCategory().toLower().contains(term);
        
        if (!matches) {
            for (const QString &tag : emailTemplate->getTags()) {
                if (tag.toLower().contains(term)) {
                    matches = true;
                    break;
                }
            }
        }
        
        if (matches) {
            result.append(emailTemplate);
        }
    }
    
    return result;
}

bool TemplateManager::loadBuiltInTemplates()
{
    QList<EmailTemplate*> builtInTemplates = EmailTemplate::getBuiltInTemplates();
    
    for (EmailTemplate* emailTemplate : builtInTemplates) {
        // Check if built-in template already exists in database
        EmailTemplate* existing = getTemplate(emailTemplate->getId());
        if (!existing) {
            // Mark as built-in by adding special prefix to ID
            QString builtInId = "builtin_" + emailTemplate->getName().toLower().replace(" ", "_");
            emailTemplate->setProperty("id", builtInId);
            
            addTemplateToMemory(emailTemplate);
            connect(emailTemplate, &EmailTemplate::templateChanged, this, &TemplateManager::onTemplateChanged);
        } else {
            delete emailTemplate; // Clean up if already exists
        }
    }
    
    emit templatesLoaded();
    return true;
}

bool TemplateManager::isBuiltInTemplate(const QString &templateId)
{
    return templateId.startsWith("builtin_");
}

EmailTemplate* TemplateManager::duplicateTemplate(const QString &templateId, const QString &newName)
{
    EmailTemplate* original = getTemplate(templateId);
    if (!original) {
        emit errorOccurred("Template not found");
        return nullptr;
    }
    
    EmailTemplate* duplicate = new EmailTemplate();
    duplicate->setName(newName.isEmpty() ? generateUniqueTemplateName(original->getName() + " Copy") : newName);
    duplicate->setSubject(original->getSubject());
    duplicate->setHtmlContent(original->getHtmlContent());
    duplicate->setTextContent(original->getTextContent());
    duplicate->setType(original->getType());
    duplicate->setDescription(original->getDescription());
    duplicate->setCategory(original->getCategory());
    duplicate->setTags(original->getTags());
    
    if (saveTemplate(duplicate)) {
        return duplicate;
    } else {
        delete duplicate;
        return nullptr;
    }
}

bool TemplateManager::exportTemplate(const QString &templateId, const QString &filePath)
{
    EmailTemplate* emailTemplate = getTemplate(templateId);
    if (!emailTemplate) {
        emit errorOccurred("Template not found");
        return false;
    }
    
    QJsonObject json = emailTemplate->toJson();
    QJsonDocument doc(json);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Cannot open file for writing: " + filePath);
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toJson();
    file.close();
    
    return true;
}

EmailTemplate* TemplateManager::importTemplate(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open file for reading: " + filePath);
        return nullptr;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emit errorOccurred("Invalid JSON file");
        return nullptr;
    }
    
    EmailTemplate* emailTemplate = new EmailTemplate();
    if (!emailTemplate->fromJson(doc.object())) {
        delete emailTemplate;
        emit errorOccurred("Invalid template format");
        return nullptr;
    }
    
    // Generate new ID and name if template already exists
    if (getTemplate(emailTemplate->getId())) {
        emailTemplate->setName(generateUniqueTemplateName(emailTemplate->getName()));
    }
    
    if (saveTemplate(emailTemplate)) {
        return emailTemplate;
    } else {
        delete emailTemplate;
        return nullptr;
    }
}

bool TemplateManager::exportAllTemplates(const QString &filePath)
{
    QJsonArray templatesArray;
    
    for (EmailTemplate* emailTemplate : templates) {
        if (!isBuiltInTemplate(emailTemplate->getId())) { // Don't export built-in templates
            templatesArray.append(emailTemplate->toJson());
        }
    }
    
    QJsonObject root;
    root["templates"] = templatesArray;
    root["exportDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["version"] = "1.0";
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Cannot open file for writing: " + filePath);
        return false;
    }
    
    QTextStream stream(&file);
    stream << doc.toJson();
    file.close();
    
    return true;
}

bool TemplateManager::importTemplatesFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open file for reading: " + filePath);
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emit errorOccurred("Invalid JSON file");
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray templatesArray = root["templates"].toArray();
    
    int importedCount = 0;
    for (const QJsonValue &value : templatesArray) {
        EmailTemplate* emailTemplate = new EmailTemplate();
        if (emailTemplate->fromJson(value.toObject())) {
            // Generate new name if template already exists
            if (getTemplate(emailTemplate->getId())) {
                emailTemplate->setName(generateUniqueTemplateName(emailTemplate->getName()));
            }
            
            if (saveTemplate(emailTemplate)) {
                importedCount++;
            } else {
                delete emailTemplate;
            }
        } else {
            delete emailTemplate;
        }
    }
    
    return importedCount > 0;
}

bool TemplateManager::initializeDatabase()
{
    database = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "templates"));
    database->setDatabaseName(databasePath);
    
    if (!database->open()) {
        emit errorOccurred("Cannot open template database: " + database->lastError().text());
        return false;
    }
    
    return createTemplateTable();
}

bool TemplateManager::createTemplateTable()
{
    QSqlQuery query(*database);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS templates (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            subject TEXT NOT NULL,
            html_content TEXT,
            text_content TEXT,
            description TEXT,
            category TEXT,
            tags TEXT,
            type INTEGER,
            created_date TEXT,
            modified_date TEXT
        )
    )";
    
    if (!query.exec(sql)) {
        emit errorOccurred("Cannot create templates table: " + query.lastError().text());
        return false;
    }
    
    return true;
}

int TemplateManager::getTemplateCount()
{
    return templates.count();
}

int TemplateManager::getTemplateCountByType(TemplateType type)
{
    return getTemplatesByType(type).count();
}

QStringList TemplateManager::getAvailableCategories()
{
    QStringList categories;
    for (EmailTemplate* emailTemplate : templates) {
        QString category = emailTemplate->getCategory();
        if (!category.isEmpty() && !categories.contains(category, Qt::CaseInsensitive)) {
            categories.append(category);
        }
    }
    categories.sort(Qt::CaseInsensitive);
    return categories;
}

QStringList TemplateManager::getAvailableTags()
{
    QStringList allTags;
    for (EmailTemplate* emailTemplate : templates) {
        for (const QString &tag : emailTemplate->getTags()) {
            if (!allTags.contains(tag, Qt::CaseInsensitive)) {
                allTags.append(tag);
            }
        }
    }
    allTags.sort(Qt::CaseInsensitive);
    return allTags;
}

bool TemplateManager::validateTemplateData(const EmailTemplate* emailTemplate)
{
    return emailTemplate && emailTemplate->validateTemplate();
}

QStringList TemplateManager::getTemplateErrors(const EmailTemplate* emailTemplate)
{
    QStringList errors;
    
    if (!emailTemplate) {
        errors.append("Template is null");
        return errors;
    }
    
    if (emailTemplate->getName().isEmpty()) {
        errors.append("Template name is required");
    }
    
    if (emailTemplate->getSubject().isEmpty()) {
        errors.append("Template subject is required");
    }
    
    if (emailTemplate->getHtmlContent().isEmpty() && emailTemplate->getTextContent().isEmpty()) {
        errors.append("Template must have HTML or text content");
    }
    
    // Validate variables
    QStringList variables = emailTemplate->extractVariables();
    for (const QString &variable : variables) {
        if (!EmailTemplate::isValidVariable(variable)) {
            errors.append(QString("Invalid variable name: %1").arg(variable));
        }
    }
    
    return errors;
}

void TemplateManager::onTemplateChanged()
{
    EmailTemplate* emailTemplate = qobject_cast<EmailTemplate*>(sender());
    if (emailTemplate) {
        updateTemplateInDatabase(emailTemplate);
        emit templateUpdated(emailTemplate->getId());
    }
}

void TemplateManager::loadTemplatesFromDatabase()
{
    QSqlQuery query(*database);
    query.prepare("SELECT * FROM templates");
    
    if (query.exec()) {
        while (query.next()) {
            EmailTemplate* emailTemplate = new EmailTemplate();
            
            QJsonObject json;
            json["id"] = query.value("id").toString();
            json["name"] = query.value("name").toString();
            json["subject"] = query.value("subject").toString();
            json["htmlContent"] = query.value("html_content").toString();
            json["textContent"] = query.value("text_content").toString();
            json["description"] = query.value("description").toString();
            json["category"] = query.value("category").toString();
            json["type"] = query.value("type").toInt();
            json["createdDate"] = query.value("created_date").toString();
            json["modifiedDate"] = query.value("modified_date").toString();
            
            QString tagsStr = query.value("tags").toString();
            QJsonArray tagsArray;
            if (!tagsStr.isEmpty()) {
                QStringList tags = tagsStr.split(",");
                for (const QString &tag : tags) {
                    tagsArray.append(tag.trimmed());
                }
            }
            json["tags"] = tagsArray;
            
            if (emailTemplate->fromJson(json)) {
                addTemplateToMemory(emailTemplate);
                connect(emailTemplate, &EmailTemplate::templateChanged, this, &TemplateManager::onTemplateChanged);
            } else {
                delete emailTemplate;
            }
        }
    }
}

bool TemplateManager::saveTemplateToDatabase(const EmailTemplate* emailTemplate)
{
    QSqlQuery query(*database);
    query.prepare(R"(
        INSERT INTO templates (id, name, subject, html_content, text_content, description, category, tags, type, created_date, modified_date)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(emailTemplate->getId());
    query.addBindValue(emailTemplate->getName());
    query.addBindValue(emailTemplate->getSubject());
    query.addBindValue(emailTemplate->getHtmlContent());
    query.addBindValue(emailTemplate->getTextContent());
    query.addBindValue(emailTemplate->getDescription());
    query.addBindValue(emailTemplate->getCategory());
    query.addBindValue(emailTemplate->getTags().join(","));
    query.addBindValue(static_cast<int>(emailTemplate->getType()));
    query.addBindValue(emailTemplate->getCreatedDate().toString(Qt::ISODate));
    query.addBindValue(emailTemplate->getModifiedDate().toString(Qt::ISODate));
    
    if (!query.exec()) {
        emit errorOccurred("Cannot save template to database: " + query.lastError().text());
        return false;
    }
    
    return true;
}

bool TemplateManager::updateTemplateInDatabase(const EmailTemplate* emailTemplate)
{
    QSqlQuery query(*database);
    query.prepare(R"(
        UPDATE templates SET name=?, subject=?, html_content=?, text_content=?, description=?, category=?, tags=?, type=?, modified_date=?
        WHERE id=?
    )");
    
    query.addBindValue(emailTemplate->getName());
    query.addBindValue(emailTemplate->getSubject());
    query.addBindValue(emailTemplate->getHtmlContent());
    query.addBindValue(emailTemplate->getTextContent());
    query.addBindValue(emailTemplate->getDescription());
    query.addBindValue(emailTemplate->getCategory());
    query.addBindValue(emailTemplate->getTags().join(","));
    query.addBindValue(static_cast<int>(emailTemplate->getType()));
    query.addBindValue(emailTemplate->getModifiedDate().toString(Qt::ISODate));
    query.addBindValue(emailTemplate->getId());
    
    return query.exec();
}

bool TemplateManager::deleteTemplateFromDatabase(const QString &templateId)
{
    QSqlQuery query(*database);
    query.prepare("DELETE FROM templates WHERE id=?");
    query.addBindValue(templateId);
    
    return query.exec();
}

EmailTemplate* TemplateManager::loadTemplateFromDatabase(const QString &templateId)
{
    QSqlQuery query(*database);
    query.prepare("SELECT * FROM templates WHERE id=?");
    query.addBindValue(templateId);
    
    if (query.exec() && query.next()) {
        EmailTemplate* emailTemplate = new EmailTemplate();
        
        QJsonObject json;
        json["id"] = query.value("id").toString();
        json["name"] = query.value("name").toString();
        json["subject"] = query.value("subject").toString();
        json["htmlContent"] = query.value("html_content").toString();
        json["textContent"] = query.value("text_content").toString();
        json["description"] = query.value("description").toString();
        json["category"] = query.value("category").toString();
        json["type"] = query.value("type").toInt();
        json["createdDate"] = query.value("created_date").toString();
        json["modifiedDate"] = query.value("modified_date").toString();
        
        QString tagsStr = query.value("tags").toString();
        QJsonArray tagsArray;
        if (!tagsStr.isEmpty()) {
            QStringList tags = tagsStr.split(",");
            for (const QString &tag : tags) {
                tagsArray.append(tag.trimmed());
            }
        }
        json["tags"] = tagsArray;
        
        if (emailTemplate->fromJson(json)) {
            return emailTemplate;
        } else {
            delete emailTemplate;
        }
    }
    
    return nullptr;
}

void TemplateManager::addTemplateToMemory(EmailTemplate* emailTemplate)
{
    if (emailTemplate && !templateMap.contains(emailTemplate->getId())) {
        templates.append(emailTemplate);
        templateMap[emailTemplate->getId()] = emailTemplate;
        emailTemplate->setParent(this);
    }
}

void TemplateManager::removeTemplateFromMemory(const QString &templateId)
{
    EmailTemplate* emailTemplate = templateMap.value(templateId);
    if (emailTemplate) {
        templates.removeOne(emailTemplate);
        templateMap.remove(templateId);
        emailTemplate->deleteLater();
    }
}

void TemplateManager::clearTemplates()
{
    for (EmailTemplate* emailTemplate : templates) {
        emailTemplate->deleteLater();
    }
    templates.clear();
    templateMap.clear();
}

QString TemplateManager::generateUniqueTemplateName(const QString &baseName)
{
    QString name = baseName;
    int counter = 1;
    
    while (true) {
        bool nameExists = false;
        for (EmailTemplate* emailTemplate : templates) {
            if (emailTemplate->getName().compare(name, Qt::CaseInsensitive) == 0) {
                nameExists = true;
                break;
            }
        }
        
        if (!nameExists) {
            return name;
        }
        
        name = QString("%1 (%2)").arg(baseName).arg(counter++);
    }
}

QString TemplateManager::typeToString(TemplateType type)
{
    switch (type) {
        case TemplateType::Custom: return "Custom";
        case TemplateType::Newsletter: return "Newsletter";
        case TemplateType::Promotional: return "Promotional";
        case TemplateType::Welcome: return "Welcome";
        case TemplateType::Transactional: return "Transactional";
        case TemplateType::Event: return "Event";
        case TemplateType::Announcement: return "Announcement";
        default: return "Custom";
    }
}

TemplateType TemplateManager::stringToType(const QString &typeStr)
{
    if (typeStr == "Newsletter") return TemplateType::Newsletter;
    if (typeStr == "Promotional") return TemplateType::Promotional;
    if (typeStr == "Welcome") return TemplateType::Welcome;
    if (typeStr == "Transactional") return TemplateType::Transactional;
    if (typeStr == "Event") return TemplateType::Event;
    if (typeStr == "Announcement") return TemplateType::Announcement;
    return TemplateType::Custom;
}
