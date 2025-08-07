#ifndef EMAILMANAGER_H
#define EMAILMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>

struct Contact {
    QString name;
    QString email;
    QString status;          // "active", "bounced", "delivered", "pending"
    QDateTime lastCampaign;
    QDateTime dateAdded;
};

class EmailManager : public QObject
{
    Q_OBJECT

public:
    explicit EmailManager(QObject *parent = nullptr);
    
    // Contact management
    bool addContact(const QString& name, const QString& email);
    bool removeContact(const QString& email);
    void updateContactStatus(const QString& email, const QString& status);
    QList<Contact> getAllContacts() const;
    Contact getContact(const QString& email) const;
    
    // Import/Export
    bool importContacts(const QString& fileName);
    bool exportContacts(const QString& fileName);
    
    // Email validation
    bool isValidEmail(const QString& email) const;
    
    // Statistics
    int getTotalContacts() const;
    int getActiveContacts() const;
    int getBouncedContacts() const;

signals:
    void contactAdded(const Contact& contact);
    void contactRemoved(const QString& email);
    void contactUpdated(const Contact& contact);

private:
    void loadContactsFromDatabase();
    void saveContactToDatabase(const Contact& contact);
    void removeContactFromDatabase(const QString& email);
    
    QList<Contact> m_contacts;
};

#endif // EMAILMANAGER_H