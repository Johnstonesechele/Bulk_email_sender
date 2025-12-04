#ifndef CSVREADER_H
#define CSVREADER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QVariant>

struct ContactData {
    QString email;
    QString firstName;
    QString lastName;
    QString fullName;
    QString company;
    QString phone;
    QString address;
    QMap<QString, QString> customFields;
    
    bool isValid() const;
    QString getDisplayName() const;
    QMap<QString, QString> toVariableMap() const;
};

class CsvReader : public QObject
{
    Q_OBJECT

public:
    explicit CsvReader(QObject *parent = nullptr);
    
    // CSV Reading
    bool loadFromCsv(const QString &filePath, char delimiter = ',', bool hasHeader = true);
    bool loadFromExcel(const QString &filePath, const QString &sheetName = "");
    
    // Data Access
    QList<ContactData> getContacts() const { return contacts; }
    QStringList getHeaders() const { return headers; }
    int getContactCount() const { return contacts.size(); }
    int getValidContactCount() const;
    
    // Data Validation
    QStringList validateData();
    QStringList getInvalidEmails();
    QList<int> getDuplicateRows();
    
    // Export functionality
    bool exportToCsv(const QString &filePath, const QList<ContactData> &contactsToExport = {});
    bool exportToExcel(const QString &filePath, const QList<ContactData> &contactsToExport = {});
    
    // Utility methods
    void clearData();
    bool isEmpty() const { return contacts.isEmpty(); }
    
    // Column mapping
    void setColumnMapping(const QMap<QString, QString> &mapping);
    QMap<QString, QString> getColumnMapping() const { return columnMapping; }
    QStringList getDetectedColumns() const;
    
    // Data cleaning
    void removeDuplicates();
    void removeInvalidEmails();
    void cleanData();

signals:
    void dataLoaded(int contactCount);
    void errorOccurred(const QString &error);
    void progressChanged(int current, int total);

private:
    QList<ContactData> contacts;
    QStringList headers;
    QMap<QString, QString> columnMapping; // CSV column -> ContactData field mapping
    QString lastError;
    
    // CSV parsing
    QStringList parseCsvLine(const QString &line, char delimiter);
    QString unescapeCsvField(const QString &field);
    bool isValidEmailFormat(const QString &email);
    
    // Excel parsing (basic implementation)
    bool parseExcelFile(const QString &filePath, const QString &sheetName);
    
    // Data mapping
    ContactData mapRowToContact(const QStringList &row);
    void autoDetectColumnMapping(const QStringList &headerRow);
    QString findBestMatch(const QString &column, const QStringList &standardFields);
    
    // Validation helpers
    bool validateContactData(const ContactData &contact);
    QString cleanEmail(const QString &email);
    QString cleanName(const QString &name);
};

#endif // CSVREADER_H
