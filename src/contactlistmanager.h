#ifndef CONTACTLISTMANAGER_H
#define CONTACTLISTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlDatabase>
#include <QVector>
#include <QMap>

struct Contact {
    int id;
    QString email;
    QString firstName;
    QString lastName;
    QString company;
    QString phone;
    QString tags;
    QDateTime dateAdded;
    QDateTime lastContacted;
    bool isSubscribed;
    bool isBlacklisted;
    QJsonObject customFields;
    
    QString fullName() const {
        return QString("%1 %2").arg(firstName, lastName).trimmed();
    }
    
    QStringList getTagsList() const {
        return tags.split(",", Qt::SkipEmptyParts);
    }
    
    void setTagsList(const QStringList &tagList) {
        tags = tagList.join(",");
    }
};

struct ContactList {
    int id;
    QString name;
    QString description;
    QDateTime dateCreated;
    QDateTime lastModified;
    int contactCount;
    QStringList tags;
    bool isDefault;
};

class ContactListManager : public QObject
{
    Q_OBJECT

public:
    explicit ContactListManager(QObject *parent = nullptr);
    ~ContactListManager();

    // Contact List Management
    int createContactList(const QString &name, const QString &description = "");
    bool deleteContactList(int listId);
    bool updateContactList(int listId, const QString &name, const QString &description = "");
    ContactList getContactList(int listId) const;
    QVector<ContactList> getAllContactLists() const;
    bool setDefaultContactList(int listId);
    int getDefaultContactListId() const;
    
    // Contact Management
    int addContact(const Contact &contact, int listId = -1);
    bool updateContact(const Contact &contact);
    bool deleteContact(int contactId);
    Contact getContact(int contactId) const;
    QVector<Contact> getContactsInList(int listId) const;
    QVector<Contact> getAllContacts() const;
    
    // List Membership
    bool addContactToList(int contactId, int listId);
    bool removeContactFromList(int contactId, int listId);
    QVector<int> getContactLists(int contactId) const; // Get lists a contact belongs to
    bool moveContactToList(int contactId, int fromListId, int toListId);
    
    // Search and Filter
    QVector<Contact> searchContacts(const QString &query, int listId = -1) const;
    QVector<Contact> getContactsByTag(const QString &tag, int listId = -1) const;
    QVector<Contact> getContactsByDateRange(const QDateTime &start, const QDateTime &end, int listId = -1) const;
    QVector<Contact> getSubscribedContacts(int listId = -1) const;
    QVector<Contact> getBlacklistedContacts() const;
    
    // Bulk Operations
    bool importContactsFromCSV(const QString &filePath, int listId, bool createNewList = false);
    bool exportContactsToCSV(const QString &filePath, int listId = -1) const;
    bool bulkUpdateSubscriptionStatus(const QVector<int> &contactIds, bool subscribed);
    bool bulkAddTags(const QVector<int> &contactIds, const QStringList &tags);
    bool bulkRemoveTags(const QVector<int> &contactIds, const QStringList &tags);
    
    // Blacklist Management
    bool addToBlacklist(const QString &email, const QString &reason = "");
    bool removeFromBlacklist(const QString &email);
    bool isBlacklisted(const QString &email) const;
    QStringList getBlacklistedEmails() const;
    
    // Validation and Cleanup
    bool validateContact(const Contact &contact) const;
    QVector<Contact> findDuplicateContacts(int listId = -1) const;
    bool mergeDuplicateContacts(int keepContactId, int deleteContactId);
    int cleanupInvalidEmails(int listId = -1); // Returns count of removed contacts
    
    // Statistics
    int getTotalContactCount() const;
    int getActiveContactCount(int listId = -1) const; // Subscribed, non-blacklisted
    QMap<QString, int> getTagStatistics(int listId = -1) const;
    QMap<QString, int> getCompanyStatistics(int listId = -1) const;
    
    // Backup and Restore
    bool backupData(const QString &filePath) const;
    bool restoreData(const QString &filePath);

signals:
    void contactAdded(const Contact &contact, int listId);
    void contactUpdated(const Contact &contact);
    void contactDeleted(int contactId);
    void contactListCreated(const ContactList &list);
    void contactListDeleted(int listId);
    void contactListUpdated(const ContactList &list);

private:
    void initializeDatabase();
    bool isValidEmail(const QString &email) const;
    void updateListContactCount(int listId);
    QString sanitizeString(const QString &str) const;
    
    QSqlDatabase database;
    int defaultListId;
};

#endif // CONTACTLISTMANAGER_H
