#include "contactlistmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QCoreApplication>

ContactListManager::ContactListManager(QObject *parent)
    : QObject(parent)
    , defaultListId(-1)
{
    initializeDatabase();
}

ContactListManager::~ContactListManager()
{
    if (database.isOpen()) {
        database.close();
    }
}

void ContactListManager::initializeDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE", "contacts");
    
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    
    QString dbPath = QDir(dataPath).filePath("contacts.db");
    database.setDatabaseName(dbPath);
    
    if (!database.open()) {
        qWarning() << "Failed to open contacts database:" << database.lastError().text();
        return;
    }
    
    QSqlQuery query(database);
    
    // Create contact_lists table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS contact_lists (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            description TEXT,
            date_created DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_modified DATETIME DEFAULT CURRENT_TIMESTAMP,
            contact_count INTEGER DEFAULT 0,
            tags TEXT,
            is_default BOOLEAN DEFAULT 0
        )
    )");
    
    // Create contacts table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS contacts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT NOT NULL UNIQUE,
            first_name TEXT,
            last_name TEXT,
            company TEXT,
            phone TEXT,
            tags TEXT,
            date_added DATETIME DEFAULT CURRENT_TIMESTAMP,
            last_contacted DATETIME,
            is_subscribed BOOLEAN DEFAULT 1,
            is_blacklisted BOOLEAN DEFAULT 0,
            custom_fields TEXT
        )
    )");
    
    // Create junction table for many-to-many relationship
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS contact_list_memberships (
            contact_id INTEGER,
            list_id INTEGER,
            date_added DATETIME DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (contact_id, list_id),
            FOREIGN KEY (contact_id) REFERENCES contacts(id) ON DELETE CASCADE,
            FOREIGN KEY (list_id) REFERENCES contact_lists(id) ON DELETE CASCADE
        )
    )");
    
    // Create blacklist table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS email_blacklist (
            email TEXT PRIMARY KEY,
            reason TEXT,
            date_added DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )");
    
    // Create indexes for performance
    query.exec("CREATE INDEX IF NOT EXISTS idx_contacts_email ON contacts(email)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_contacts_company ON contacts(company)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_contacts_subscribed ON contacts(is_subscribed)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_contacts_blacklisted ON contacts(is_blacklisted)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_memberships_contact ON contact_list_memberships(contact_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_memberships_list ON contact_list_memberships(list_id)");
    
    // Create default contact list if none exists
    query.exec("SELECT COUNT(*) FROM contact_lists");
    if (query.next() && query.value(0).toInt() == 0) {
        int defaultId = createContactList("Default List", "Default contact list");
        setDefaultContactList(defaultId);
    }
    
    // Load default list ID
    query.exec("SELECT id FROM contact_lists WHERE is_default = 1");
    if (query.next()) {
        defaultListId = query.value(0).toInt();
    }
}

int ContactListManager::createContactList(const QString &name, const QString &description)
{
    QSqlQuery query(database);
    
    query.prepare(R"(
        INSERT INTO contact_lists (name, description, date_created, last_modified)
        VALUES (?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)
    )");
    
    query.addBindValue(name);
    query.addBindValue(description);
    
    if (!query.exec()) {
        qWarning() << "Failed to create contact list:" << query.lastError().text();
        return -1;
    }
    
    int listId = query.lastInsertId().toInt();
    
    ContactList list = getContactList(listId);
    emit contactListCreated(list);
    
    return listId;
}

bool ContactListManager::deleteContactList(int listId)
{
    if (listId == defaultListId) {
        qWarning() << "Cannot delete default contact list";
        return false;
    }
    
    QSqlQuery query(database);
    
    // Remove all memberships first
    query.prepare("DELETE FROM contact_list_memberships WHERE list_id = ?");
    query.addBindValue(listId);
    
    if (!query.exec()) {
        qWarning() << "Failed to remove contact list memberships:" << query.lastError().text();
        return false;
    }
    
    // Delete the list
    query.prepare("DELETE FROM contact_lists WHERE id = ?");
    query.addBindValue(listId);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete contact list:" << query.lastError().text();
        return false;
    }
    
    emit contactListDeleted(listId);
    return true;
}

bool ContactListManager::updateContactList(int listId, const QString &name, const QString &description)
{
    QSqlQuery query(database);
    
    query.prepare(R"(
        UPDATE contact_lists 
        SET name = ?, description = ?, last_modified = CURRENT_TIMESTAMP
        WHERE id = ?
    )");
    
    query.addBindValue(name);
    query.addBindValue(description);
    query.addBindValue(listId);
    
    if (!query.exec()) {
        qWarning() << "Failed to update contact list:" << query.lastError().text();
        return false;
    }
    
    ContactList list = getContactList(listId);
    emit contactListUpdated(list);
    
    return true;
}

ContactList ContactListManager::getContactList(int listId) const
{
    ContactList list;
    list.id = -1;
    
    QSqlQuery query(database);
    
    query.prepare("SELECT * FROM contact_lists WHERE id = ?");
    query.addBindValue(listId);
    
    if (query.exec() && query.next()) {
        list.id = query.value("id").toInt();
        list.name = query.value("name").toString();
        list.description = query.value("description").toString();
        list.dateCreated = query.value("date_created").toDateTime();
        list.lastModified = query.value("last_modified").toDateTime();
        list.contactCount = query.value("contact_count").toInt();
        list.tags = query.value("tags").toString().split(",", Qt::SkipEmptyParts);
        list.isDefault = query.value("is_default").toBool();
    }
    
    return list;
}

QVector<ContactList> ContactListManager::getAllContactLists() const
{
    QVector<ContactList> lists;
    
    QSqlQuery query(database);
    
    query.exec("SELECT * FROM contact_lists ORDER BY name");
    
    while (query.next()) {
        ContactList list;
        list.id = query.value("id").toInt();
        list.name = query.value("name").toString();
        list.description = query.value("description").toString();
        list.dateCreated = query.value("date_created").toDateTime();
        list.lastModified = query.value("last_modified").toDateTime();
        list.contactCount = query.value("contact_count").toInt();
        list.tags = query.value("tags").toString().split(",", Qt::SkipEmptyParts);
        list.isDefault = query.value("is_default").toBool();
        
        lists.append(list);
    }
    
    return lists;
}

bool ContactListManager::setDefaultContactList(int listId)
{
    QSqlQuery query(database);
    
    // Clear current default
    query.exec("UPDATE contact_lists SET is_default = 0");
    
    // Set new default
    query.prepare("UPDATE contact_lists SET is_default = 1 WHERE id = ?");
    query.addBindValue(listId);
    
    if (!query.exec()) {
        qWarning() << "Failed to set default contact list:" << query.lastError().text();
        return false;
    }
    
    defaultListId = listId;
    return true;
}

int ContactListManager::getDefaultContactListId() const
{
    return defaultListId;
}

int ContactListManager::addContact(const Contact &contact, int listId)
{
    if (!validateContact(contact)) {
        qWarning() << "Invalid contact data";
        return -1;
    }
    
    if (isBlacklisted(contact.email)) {
        qWarning() << "Cannot add blacklisted email:" << contact.email;
        return -1;
    }
    
    if (listId == -1) {
        listId = defaultListId;
    }
    
    QSqlQuery query(database);
    
    query.prepare(R"(
        INSERT INTO contacts (email, first_name, last_name, company, phone, tags, 
                            last_contacted, is_subscribed, is_blacklisted, custom_fields)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(contact.email);
    query.addBindValue(contact.firstName);
    query.addBindValue(contact.lastName);
    query.addBindValue(contact.company);
    query.addBindValue(contact.phone);
    query.addBindValue(contact.tags);
    query.addBindValue(contact.lastContacted);
    query.addBindValue(contact.isSubscribed);
    query.addBindValue(contact.isBlacklisted);
    
    QJsonDocument customFieldsDoc(contact.customFields);
    query.addBindValue(customFieldsDoc.toJson(QJsonDocument::Compact));
    
    if (!query.exec()) {
        qWarning() << "Failed to add contact:" << query.lastError().text();
        return -1;
    }
    
    int contactId = query.lastInsertId().toInt();
    
    // Add to list
    if (listId > 0 && !addContactToList(contactId, listId)) {
        // If we can't add to list, remove the contact
        deleteContact(contactId);
        return -1;
    }
    
    updateListContactCount(listId);
    
    Contact addedContact = getContact(contactId);
    emit contactAdded(addedContact, listId);
    
    return contactId;
}

bool ContactListManager::updateContact(const Contact &contact)
{
    if (!validateContact(contact)) {
        return false;
    }
    
    QSqlQuery query(database);
    
    query.prepare(R"(
        UPDATE contacts 
        SET first_name = ?, last_name = ?, company = ?, phone = ?, tags = ?,
            last_contacted = ?, is_subscribed = ?, is_blacklisted = ?, custom_fields = ?
        WHERE id = ?
    )");
    
    query.addBindValue(contact.firstName);
    query.addBindValue(contact.lastName);
    query.addBindValue(contact.company);
    query.addBindValue(contact.phone);
    query.addBindValue(contact.tags);
    query.addBindValue(contact.lastContacted);
    query.addBindValue(contact.isSubscribed);
    query.addBindValue(contact.isBlacklisted);
    
    QJsonDocument customFieldsDoc(contact.customFields);
    query.addBindValue(customFieldsDoc.toJson(QJsonDocument::Compact));
    query.addBindValue(contact.id);
    
    if (!query.exec()) {
        qWarning() << "Failed to update contact:" << query.lastError().text();
        return false;
    }
    
    emit contactUpdated(contact);
    return true;
}

bool ContactListManager::deleteContact(int contactId)
{
    QSqlQuery query(database);
    
    // Get contact lists for updating counts
    QVector<int> listIds = getContactLists(contactId);
    
    // Delete memberships first
    query.prepare("DELETE FROM contact_list_memberships WHERE contact_id = ?");
    query.addBindValue(contactId);
    
    if (!query.exec()) {
        qWarning() << "Failed to remove contact list memberships:" << query.lastError().text();
        return false;
    }
    
    // Delete the contact
    query.prepare("DELETE FROM contacts WHERE id = ?");
    query.addBindValue(contactId);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete contact:" << query.lastError().text();
        return false;
    }
    
    // Update list contact counts
    for (int listId : listIds) {
        updateListContactCount(listId);
    }
    
    emit contactDeleted(contactId);
    return true;
}

Contact ContactListManager::getContact(int contactId) const
{
    Contact contact;
    contact.id = -1;
    
    QSqlQuery query(database);
    
    query.prepare("SELECT * FROM contacts WHERE id = ?");
    query.addBindValue(contactId);
    
    if (query.exec() && query.next()) {
        contact.id = query.value("id").toInt();
        contact.email = query.value("email").toString();
        contact.firstName = query.value("first_name").toString();
        contact.lastName = query.value("last_name").toString();
        contact.company = query.value("company").toString();
        contact.phone = query.value("phone").toString();
        contact.tags = query.value("tags").toString();
        contact.dateAdded = query.value("date_added").toDateTime();
        contact.lastContacted = query.value("last_contacted").toDateTime();
        contact.isSubscribed = query.value("is_subscribed").toBool();
        contact.isBlacklisted = query.value("is_blacklisted").toBool();
        
        QString customFieldsJson = query.value("custom_fields").toString();
        if (!customFieldsJson.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(customFieldsJson.toUtf8());
            contact.customFields = doc.object();
        }
    }
    
    return contact;
}

QVector<Contact> ContactListManager::getContactsInList(int listId) const
{
    QVector<Contact> contacts;
    
    QSqlQuery query(database);
    
    query.prepare(R"(
        SELECT c.* FROM contacts c
        JOIN contact_list_memberships clm ON c.id = clm.contact_id
        WHERE clm.list_id = ?
        ORDER BY c.last_name, c.first_name, c.email
    )");
    
    query.addBindValue(listId);
    
    if (query.exec()) {
        while (query.next()) {
            Contact contact;
            contact.id = query.value("id").toInt();
            contact.email = query.value("email").toString();
            contact.firstName = query.value("first_name").toString();
            contact.lastName = query.value("last_name").toString();
            contact.company = query.value("company").toString();
            contact.phone = query.value("phone").toString();
            contact.tags = query.value("tags").toString();
            contact.dateAdded = query.value("date_added").toDateTime();
            contact.lastContacted = query.value("last_contacted").toDateTime();
            contact.isSubscribed = query.value("is_subscribed").toBool();
            contact.isBlacklisted = query.value("is_blacklisted").toBool();
            
            QString customFieldsJson = query.value("custom_fields").toString();
            if (!customFieldsJson.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(customFieldsJson.toUtf8());
                contact.customFields = doc.object();
            }
            
            contacts.append(contact);
        }
    }
    
    return contacts;
}

QVector<Contact> ContactListManager::getAllContacts() const
{
    QVector<Contact> contacts;
    
    QSqlQuery query(database);
    
    query.exec("SELECT * FROM contacts ORDER BY last_name, first_name, email");
    
    while (query.next()) {
        Contact contact;
        contact.id = query.value("id").toInt();
        contact.email = query.value("email").toString();
        contact.firstName = query.value("first_name").toString();
        contact.lastName = query.value("last_name").toString();
        contact.company = query.value("company").toString();
        contact.phone = query.value("phone").toString();
        contact.tags = query.value("tags").toString();
        contact.dateAdded = query.value("date_added").toDateTime();
        contact.lastContacted = query.value("last_contacted").toDateTime();
        contact.isSubscribed = query.value("is_subscribed").toBool();
        contact.isBlacklisted = query.value("is_blacklisted").toBool();
        
        QString customFieldsJson = query.value("custom_fields").toString();
        if (!customFieldsJson.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(customFieldsJson.toUtf8());
            contact.customFields = doc.object();
        }
        
        contacts.append(contact);
    }
    
    return contacts;
}

bool ContactListManager::addContactToList(int contactId, int listId)
{
    QSqlQuery query(database);
    
    query.prepare(R"(
        INSERT OR IGNORE INTO contact_list_memberships (contact_id, list_id)
        VALUES (?, ?)
    )");
    
    query.addBindValue(contactId);
    query.addBindValue(listId);
    
    if (!query.exec()) {
        qWarning() << "Failed to add contact to list:" << query.lastError().text();
        return false;
    }
    
    updateListContactCount(listId);
    return true;
}

bool ContactListManager::removeContactFromList(int contactId, int listId)
{
    QSqlQuery query(database);
    
    query.prepare("DELETE FROM contact_list_memberships WHERE contact_id = ? AND list_id = ?");
    query.addBindValue(contactId);
    query.addBindValue(listId);
    
    if (!query.exec()) {
        qWarning() << "Failed to remove contact from list:" << query.lastError().text();
        return false;
    }
    
    updateListContactCount(listId);
    return true;
}

QVector<int> ContactListManager::getContactLists(int contactId) const
{
    QVector<int> listIds;
    
    QSqlQuery query(database);
    
    query.prepare("SELECT list_id FROM contact_list_memberships WHERE contact_id = ?");
    query.addBindValue(contactId);
    
    if (query.exec()) {
        while (query.next()) {
            listIds.append(query.value("list_id").toInt());
        }
    }
    
    return listIds;
}

bool ContactListManager::moveContactToList(int contactId, int fromListId, int toListId)
{
    if (!removeContactFromList(contactId, fromListId)) {
        return false;
    }
    
    return addContactToList(contactId, toListId);
}

QVector<Contact> ContactListManager::searchContacts(const QString &query, int listId) const
{
    QVector<Contact> contacts;
    
    QSqlQuery sqlQuery(database);
    QString sql;
    
    if (listId == -1) {
        sql = R"(
            SELECT DISTINCT c.* FROM contacts c
            WHERE c.email LIKE ? OR c.first_name LIKE ? OR c.last_name LIKE ? 
                  OR c.company LIKE ? OR c.tags LIKE ?
            ORDER BY c.last_name, c.first_name, c.email
        )";
    } else {
        sql = R"(
            SELECT DISTINCT c.* FROM contacts c
            JOIN contact_list_memberships clm ON c.id = clm.contact_id
            WHERE clm.list_id = ? AND (
                c.email LIKE ? OR c.first_name LIKE ? OR c.last_name LIKE ? 
                OR c.company LIKE ? OR c.tags LIKE ?
            )
            ORDER BY c.last_name, c.first_name, c.email
        )";
    }
    
    sqlQuery.prepare(sql);
    
    QString searchPattern = QString("%%1%").arg(query);
    
    if (listId != -1) {
        sqlQuery.addBindValue(listId);
    }
    
    sqlQuery.addBindValue(searchPattern);
    sqlQuery.addBindValue(searchPattern);
    sqlQuery.addBindValue(searchPattern);
    sqlQuery.addBindValue(searchPattern);
    sqlQuery.addBindValue(searchPattern);
    
    if (sqlQuery.exec()) {
        while (sqlQuery.next()) {
            Contact contact;
            contact.id = sqlQuery.value("id").toInt();
            contact.email = sqlQuery.value("email").toString();
            contact.firstName = sqlQuery.value("first_name").toString();
            contact.lastName = sqlQuery.value("last_name").toString();
            contact.company = sqlQuery.value("company").toString();
            contact.phone = sqlQuery.value("phone").toString();
            contact.tags = sqlQuery.value("tags").toString();
            contact.dateAdded = sqlQuery.value("date_added").toDateTime();
            contact.lastContacted = sqlQuery.value("last_contacted").toDateTime();
            contact.isSubscribed = sqlQuery.value("is_subscribed").toBool();
            contact.isBlacklisted = sqlQuery.value("is_blacklisted").toBool();
            
            QString customFieldsJson = sqlQuery.value("custom_fields").toString();
            if (!customFieldsJson.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(customFieldsJson.toUtf8());
                contact.customFields = doc.object();
            }
            
            contacts.append(contact);
        }
    }
    
    return contacts;
}

QVector<Contact> ContactListManager::getContactsByTag(const QString &tag, int listId) const
{
    QVector<Contact> contacts;
    
    QSqlQuery query(database);
    QString sql;
    
    if (listId == -1) {
        sql = "SELECT * FROM contacts WHERE tags LIKE ? ORDER BY last_name, first_name, email";
    } else {
        sql = R"(
            SELECT c.* FROM contacts c
            JOIN contact_list_memberships clm ON c.id = clm.contact_id
            WHERE clm.list_id = ? AND c.tags LIKE ?
            ORDER BY c.last_name, c.first_name, c.email
        )";
    }
    
    query.prepare(sql);
    
    if (listId != -1) {
        query.addBindValue(listId);
    }
    
    query.addBindValue(QString("%%1%").arg(tag));
    
    if (query.exec()) {
        while (query.next()) {
            Contact contact;
            contact.id = query.value("id").toInt();
            contact.email = query.value("email").toString();
            contact.firstName = query.value("first_name").toString();
            contact.lastName = query.value("last_name").toString();
            contact.company = query.value("company").toString();
            contact.phone = query.value("phone").toString();
            contact.tags = query.value("tags").toString();
            contact.dateAdded = query.value("date_added").toDateTime();
            contact.lastContacted = query.value("last_contacted").toDateTime();
            contact.isSubscribed = query.value("is_subscribed").toBool();
            contact.isBlacklisted = query.value("is_blacklisted").toBool();
            
            QString customFieldsJson = query.value("custom_fields").toString();
            if (!customFieldsJson.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(customFieldsJson.toUtf8());
                contact.customFields = doc.object();
            }
            
            contacts.append(contact);
        }
    }
    
    return contacts;
}

QVector<Contact> ContactListManager::getSubscribedContacts(int listId) const
{
    QVector<Contact> contacts;
    
    QSqlQuery query(database);
    QString sql;
    
    if (listId == -1) {
        sql = R"(
            SELECT * FROM contacts 
            WHERE is_subscribed = 1 AND is_blacklisted = 0
            ORDER BY last_name, first_name, email
        )";
    } else {
        sql = R"(
            SELECT c.* FROM contacts c
            JOIN contact_list_memberships clm ON c.id = clm.contact_id
            WHERE clm.list_id = ? AND c.is_subscribed = 1 AND c.is_blacklisted = 0
            ORDER BY c.last_name, c.first_name, c.email
        )";
    }
    
    query.prepare(sql);
    
    if (listId != -1) {
        query.addBindValue(listId);
    }
    
    if (query.exec()) {
        while (query.next()) {
            Contact contact;
            contact.id = query.value("id").toInt();
            contact.email = query.value("email").toString();
            contact.firstName = query.value("first_name").toString();
            contact.lastName = query.value("last_name").toString();
            contact.company = query.value("company").toString();
            contact.phone = query.value("phone").toString();
            contact.tags = query.value("tags").toString();
            contact.dateAdded = query.value("date_added").toDateTime();
            contact.lastContacted = query.value("last_contacted").toDateTime();
            contact.isSubscribed = query.value("is_subscribed").toBool();
            contact.isBlacklisted = query.value("is_blacklisted").toBool();
            
            QString customFieldsJson = query.value("custom_fields").toString();
            if (!customFieldsJson.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(customFieldsJson.toUtf8());
                contact.customFields = doc.object();
            }
            
            contacts.append(contact);
        }
    }
    
    return contacts;
}

bool ContactListManager::addToBlacklist(const QString &email, const QString &reason)
{
    QSqlQuery query(database);
    
    query.prepare("INSERT OR REPLACE INTO email_blacklist (email, reason) VALUES (?, ?)");
    query.addBindValue(email.toLower());
    query.addBindValue(reason);
    
    if (!query.exec()) {
        qWarning() << "Failed to add to blacklist:" << query.lastError().text();
        return false;
    }
    
    // Update existing contacts
    query.prepare("UPDATE contacts SET is_blacklisted = 1 WHERE LOWER(email) = ?");
    query.addBindValue(email.toLower());
    query.exec();
    
    return true;
}

bool ContactListManager::removeFromBlacklist(const QString &email)
{
    QSqlQuery query(database);
    
    query.prepare("DELETE FROM email_blacklist WHERE LOWER(email) = ?");
    query.addBindValue(email.toLower());
    
    if (!query.exec()) {
        qWarning() << "Failed to remove from blacklist:" << query.lastError().text();
        return false;
    }
    
    // Update existing contacts
    query.prepare("UPDATE contacts SET is_blacklisted = 0 WHERE LOWER(email) = ?");
    query.addBindValue(email.toLower());
    query.exec();
    
    return true;
}

bool ContactListManager::isBlacklisted(const QString &email) const
{
    QSqlQuery query(database);
    
    query.prepare("SELECT 1 FROM email_blacklist WHERE LOWER(email) = ?");
    query.addBindValue(email.toLower());
    
    return query.exec() && query.next();
}

bool ContactListManager::validateContact(const Contact &contact) const
{
    return isValidEmail(contact.email);
}

bool ContactListManager::isValidEmail(const QString &email) const
{
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}

void ContactListManager::updateListContactCount(int listId)
{
    QSqlQuery query(database);
    
    query.prepare(R"(
        UPDATE contact_lists 
        SET contact_count = (
            SELECT COUNT(*) FROM contact_list_memberships WHERE list_id = ?
        ),
        last_modified = CURRENT_TIMESTAMP
        WHERE id = ?
    )");
    
    query.addBindValue(listId);
    query.addBindValue(listId);
    
    if (!query.exec()) {
        qWarning() << "Failed to update list contact count:" << query.lastError().text();
    }
}

int ContactListManager::getTotalContactCount() const
{
    QSqlQuery query(database);
    
    query.exec("SELECT COUNT(*) FROM contacts");
    if (query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

int ContactListManager::getActiveContactCount(int listId) const
{
    QSqlQuery query(database);
    
    if (listId == -1) {
        query.exec("SELECT COUNT(*) FROM contacts WHERE is_subscribed = 1 AND is_blacklisted = 0");
    } else {
        query.prepare(R"(
            SELECT COUNT(*) FROM contacts c
            JOIN contact_list_memberships clm ON c.id = clm.contact_id
            WHERE clm.list_id = ? AND c.is_subscribed = 1 AND c.is_blacklisted = 0
        )");
        query.addBindValue(listId);
        query.exec();
    }
    
    if (query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QString ContactListManager::sanitizeString(const QString &str) const
{
    return str.trimmed();
}
