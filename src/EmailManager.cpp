#include "EmailManager.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

EmailManager::EmailManager(QObject *parent)
    : QObject(parent)
{
    loadContactsFromDatabase();
}

bool EmailManager::addContact(const QString& name, const QString& email)
{
    if (!isValidEmail(email)) {
        return false;
    }
    
    // Check if email already exists
    for (const auto& contact : m_contacts) {
        if (contact.email.toLower() == email.toLower()) {
            return false; // Email already exists
        }
    }
    
    Contact newContact;
    newContact.name = name;
    newContact.email = email;
    newContact.status = "active";
    newContact.dateAdded = QDateTime::currentDateTime();
    newContact.lastCampaign = QDateTime();
    
    m_contacts.append(newContact);
    saveContactToDatabase(newContact);
    
    emit contactAdded(newContact);
    return true;
}

bool EmailManager::removeContact(const QString& email)
{
    for (int i = 0; i < m_contacts.size(); ++i) {
        if (m_contacts[i].email.toLower() == email.toLower()) {
            m_contacts.removeAt(i);
            removeContactFromDatabase(email);
            emit contactRemoved(email);
            return true;
        }
    }
    return false;
}

void EmailManager::updateContactStatus(const QString& email, const QString& status)
{
    for (auto& contact : m_contacts) {
        if (contact.email.toLower() == email.toLower()) {
            contact.status = status;
            contact.lastCampaign = QDateTime::currentDateTime();
            saveContactToDatabase(contact);
            emit contactUpdated(contact);
            break;
        }
    }
}

QList<Contact> EmailManager::getAllContacts() const
{
    return m_contacts;
}

Contact EmailManager::getContact(const QString& email) const
{
    for (const auto& contact : m_contacts) {
        if (contact.email.toLower() == email.toLower()) {
            return contact;
        }
    }
    return Contact(); // Return empty contact if not found
}

bool EmailManager::importContacts(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for reading:" << fileName;
        return false;
    }
    
    QTextStream in(&file);
    QString line = in.readLine(); // Skip header line if present
    
    int importedCount = 0;
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList fields = line.split(',');
        
        if (fields.size() >= 2) {
            QString name = fields[0].trimmed().remove('"');
            QString email = fields[1].trimmed().remove('"');
            
            if (addContact(name, email)) {
                importedCount++;
            }
        }
    }
    
    file.close();
    qDebug() << "Imported" << importedCount << "contacts from" << fileName;
    return importedCount > 0;
}

bool EmailManager::exportContacts(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Could not open file for writing:" << fileName;
        return false;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "Name,Email,Status,Last Campaign,Date Added\n";
    
    // Write contacts
    for (const auto& contact : m_contacts) {
        out << "\"" << contact.name << "\","
            << "\"" << contact.email << "\","
            << "\"" << contact.status << "\","
            << "\"" << contact.lastCampaign.toString("yyyy-MM-dd hh:mm:ss") << "\","
            << "\"" << contact.dateAdded.toString("yyyy-MM-dd hh:mm:ss") << "\"\n";
    }
    
    file.close();
    qDebug() << "Exported" << m_contacts.size() << "contacts to" << fileName;
    return true;
}

bool EmailManager::isValidEmail(const QString& email) const
{
    QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return regex.match(email).hasMatch();
}

int EmailManager::getTotalContacts() const
{
    return m_contacts.size();
}

int EmailManager::getActiveContacts() const
{
    int count = 0;
    for (const auto& contact : m_contacts) {
        if (contact.status == "active" || contact.status == "delivered") {
            count++;
        }
    }
    return count;
}

int EmailManager::getBouncedContacts() const
{
    int count = 0;
    for (const auto& contact : m_contacts) {
        if (contact.status == "bounced") {
            count++;
        }
    }
    return count;
}

void EmailManager::loadContactsFromDatabase()
{
    // For now, we'll use a simple text file as our "database"
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    QString dbFile = dataDir + "/contacts.csv";
    
    QFile file(dbFile);
    if (!file.exists()) {
        // Create sample contacts for demonstration
        addContact("John Doe", "john.doe@example.com");
        addContact("Jane Smith", "jane.smith@example.com");
        addContact("Bob Johnson", "bob.johnson@example.com");
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream in(&file);
    QString line = in.readLine(); // Skip header
    
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList fields = line.split(',');
        
        if (fields.size() >= 5) {
            Contact contact;
            contact.name = fields[0].trimmed().remove('"');
            contact.email = fields[1].trimmed().remove('"');
            contact.status = fields[2].trimmed().remove('"');
            contact.lastCampaign = QDateTime::fromString(fields[3].trimmed().remove('"'), "yyyy-MM-dd hh:mm:ss");
            contact.dateAdded = QDateTime::fromString(fields[4].trimmed().remove('"'), "yyyy-MM-dd hh:mm:ss");
            
            m_contacts.append(contact);
        }
    }
    
    file.close();
}

void EmailManager::saveContactToDatabase(const Contact& contact)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    QString dbFile = dataDir + "/contacts.csv";
    
    // For simplicity, we'll rewrite the entire file each time
    // In a real application, you'd use a proper database
    QFile file(dbFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream out(&file);
    out << "Name,Email,Status,Last Campaign,Date Added\n";
    
    for (const auto& c : m_contacts) {
        out << "\"" << c.name << "\","
            << "\"" << c.email << "\","
            << "\"" << c.status << "\","
            << "\"" << c.lastCampaign.toString("yyyy-MM-dd hh:mm:ss") << "\","
            << "\"" << c.dateAdded.toString("yyyy-MM-dd hh:mm:ss") << "\"\n";
    }
    
    file.close();
}

void EmailManager::removeContactFromDatabase(const QString& email)
{
    // The contact has already been removed from m_contacts,
    // so we just need to save the updated list
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    QString dbFile = dataDir + "/contacts.csv";
    
    QFile file(dbFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream out(&file);
    out << "Name,Email,Status,Last Campaign,Date Added\n";
    
    for (const auto& c : m_contacts) {
        out << "\"" << c.name << "\","
            << "\"" << c.email << "\","
            << "\"" << c.status << "\","
            << "\"" << c.lastCampaign.toString("yyyy-MM-dd hh:mm:ss") << "\","
            << "\"" << c.dateAdded.toString("yyyy-MM-dd hh:mm:ss") << "\"\n";
    }
    
    file.close();
}

#include "EmailManager.moc"