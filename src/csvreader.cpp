#include "csvreader.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QDir>
#include <QApplication>

bool ContactData::isValid() const
{
    // Email is required and must be valid format
    if (email.isEmpty()) return false;
    
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}

QString ContactData::getDisplayName() const
{
    if (!fullName.isEmpty()) return fullName;
    if (!firstName.isEmpty() && !lastName.isEmpty()) return firstName + " " + lastName;
    if (!firstName.isEmpty()) return firstName;
    if (!lastName.isEmpty()) return lastName;
    return email; // Fallback to email
}

QMap<QString, QString> ContactData::toVariableMap() const
{
    QMap<QString, QString> variables;
    variables["email"] = email;
    variables["first_name"] = firstName;
    variables["last_name"] = lastName;
    variables["full_name"] = getDisplayName();
    variables["company"] = company;
    variables["phone"] = phone;
    variables["address"] = address;
    
    // Add custom fields
    QMapIterator<QString, QString> it(customFields);
    while (it.hasNext()) {
        it.next();
        variables[it.key()] = it.value();
    }
    
    return variables;
}

CsvReader::CsvReader(QObject *parent) : QObject(parent)
{
    // Set up standard column mapping patterns
    columnMapping["email"] = "email";
    columnMapping["first_name"] = "firstName";
    columnMapping["last_name"] = "lastName";
    columnMapping["full_name"] = "fullName";
    columnMapping["name"] = "fullName";
    columnMapping["company"] = "company";
    columnMapping["phone"] = "phone";
    columnMapping["address"] = "address";
}

bool CsvReader::loadFromCsv(const QString &filePath, char delimiter, bool hasHeader)
{
    clearData();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        lastError = QString("Cannot open file: %1").arg(file.errorString());
        emit errorOccurred(lastError);
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    QStringList lines;
    while (!stream.atEnd()) {
        lines << stream.readLine();
    }
    file.close();
    
    if (lines.isEmpty()) {
        lastError = "File is empty";
        emit errorOccurred(lastError);
        return false;
    }
    
    int startRow = 0;
    if (hasHeader) {
        headers = parseCsvLine(lines.first(), delimiter);
        autoDetectColumnMapping(headers);
        startRow = 1;
    } else {
        // Generate default headers
        int colCount = parseCsvLine(lines.first(), delimiter).size();
        for (int i = 0; i < colCount; ++i) {
            headers << QString("Column_%1").arg(i + 1);
        }
    }
    
    int totalRows = lines.size() - startRow;
    int processedRows = 0;
    
    for (int i = startRow; i < lines.size(); ++i) {
        QStringList row = parseCsvLine(lines[i], delimiter);
        if (!row.isEmpty()) {
            ContactData contact = mapRowToContact(row);
            if (!contact.email.isEmpty()) { // Only add if email exists
                contacts.append(contact);
            }
        }
        
        processedRows++;
        if (processedRows % 100 == 0) {
            emit progressChanged(processedRows, totalRows);
            QApplication::processEvents();
        }
    }
    
    emit progressChanged(totalRows, totalRows);
    emit dataLoaded(contacts.size());
    
    return true;
}

bool CsvReader::loadFromExcel(const QString &filePath, const QString &sheetName)
{
    // For now, treat Excel files as CSV (basic implementation)
    // In a full implementation, you would use a library like QtXlsx or OpenXLSX
    lastError = "Excel support requires additional libraries. Please save as CSV format.";
    emit errorOccurred(lastError);
    return false;
}

QStringList CsvReader::parseCsvLine(const QString &line, char delimiter)
{
    QStringList fields;
    QString field;
    bool inQuotes = false;
    
    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        
        if (c == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                field += '"'; // Escaped quote
                ++i; // Skip next quote
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == delimiter && !inQuotes) {
            fields << unescapeCsvField(field.trimmed());
            field.clear();
        } else {
            field += c;
        }
    }
    
    fields << unescapeCsvField(field.trimmed());
    return fields;
}

QString CsvReader::unescapeCsvField(const QString &field)
{
    QString result = field;
    
    // Remove surrounding quotes
    if (result.startsWith('"') && result.endsWith('"')) {
        result = result.mid(1, result.length() - 2);
    }
    
    // Unescape double quotes
    result.replace("\"\"", "\"");
    
    return result;
}

ContactData CsvReader::mapRowToContact(const QStringList &row)
{
    ContactData contact;
    
    // Map based on column mapping
    for (int i = 0; i < headers.size() && i < row.size(); ++i) {
        QString header = headers[i].toLower().trimmed();
        QString value = row[i].trimmed();
        
        if (value.isEmpty()) continue;
        
        // Try to map to known fields
        if (header.contains("email") || header.contains("e-mail") || header.contains("e_mail")) {
            contact.email = cleanEmail(value);
        } else if (header.contains("first") && header.contains("name")) {
            contact.firstName = cleanName(value);
        } else if (header.contains("last") && header.contains("name")) {
            contact.lastName = cleanName(value);
        } else if (header.contains("full") && header.contains("name")) {
            contact.fullName = cleanName(value);
        } else if (header.contains("name") && !header.contains("company")) {
            contact.fullName = cleanName(value);
        } else if (header.contains("company") || header.contains("organization") || header.contains("org")) {
            contact.company = value;
        } else if (header.contains("phone") || header.contains("tel") || header.contains("mobile")) {
            contact.phone = value;
        } else if (header.contains("address") || header.contains("location")) {
            contact.address = value;
        } else {
            // Store as custom field
            contact.customFields[header] = value;
        }
    }
    
    // If no full name but have first/last, combine them
    if (contact.fullName.isEmpty() && (!contact.firstName.isEmpty() || !contact.lastName.isEmpty())) {
        contact.fullName = (contact.firstName + " " + contact.lastName).trimmed();
    }
    
    return contact;
}

void CsvReader::autoDetectColumnMapping(const QStringList &headerRow)
{
    columnMapping.clear();
    
    for (const QString &header : headerRow) {
        QString lowerHeader = header.toLower().trimmed();
        
        // Email detection
        if (lowerHeader.contains("email") || lowerHeader.contains("e-mail") || lowerHeader.contains("e_mail")) {
            columnMapping[header] = "email";
        }
        // Name detection
        else if (lowerHeader.contains("first") && lowerHeader.contains("name")) {
            columnMapping[header] = "firstName";
        } else if (lowerHeader.contains("last") && lowerHeader.contains("name")) {
            columnMapping[header] = "lastName";
        } else if (lowerHeader.contains("full") && lowerHeader.contains("name")) {
            columnMapping[header] = "fullName";
        } else if (lowerHeader == "name" || lowerHeader == "full name" || lowerHeader == "fullname") {
            columnMapping[header] = "fullName";
        }
        // Company detection
        else if (lowerHeader.contains("company") || lowerHeader.contains("organization") || lowerHeader == "org") {
            columnMapping[header] = "company";
        }
        // Phone detection
        else if (lowerHeader.contains("phone") || lowerHeader.contains("tel") || lowerHeader.contains("mobile")) {
            columnMapping[header] = "phone";
        }
        // Address detection
        else if (lowerHeader.contains("address") || lowerHeader.contains("location")) {
            columnMapping[header] = "address";
        }
    }
}

QStringList CsvReader::validateData()
{
    QStringList issues;
    
    int invalidEmails = 0;
    int duplicates = 0;
    QStringList emailsSeen;
    
    for (int i = 0; i < contacts.size(); ++i) {
        const ContactData &contact = contacts[i];
        
        // Check email validity
        if (!contact.isValid()) {
            invalidEmails++;
        }
        
        // Check for duplicates
        if (emailsSeen.contains(contact.email.toLower())) {
            duplicates++;
        } else {
            emailsSeen << contact.email.toLower();
        }
    }
    
    if (invalidEmails > 0) {
        issues << QString("Found %1 invalid email addresses").arg(invalidEmails);
    }
    
    if (duplicates > 0) {
        issues << QString("Found %1 duplicate email addresses").arg(duplicates);
    }
    
    if (contacts.isEmpty()) {
        issues << "No valid contacts found";
    }
    
    return issues;
}

bool CsvReader::exportToCsv(const QString &filePath, const QList<ContactData> &contactsToExport)
{
    QList<ContactData> dataToExport = contactsToExport.isEmpty() ? contacts : contactsToExport;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        lastError = QString("Cannot create file: %1").arg(file.errorString());
        emit errorOccurred(lastError);
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    // Write header
    QStringList csvHeaders = {"Email", "First Name", "Last Name", "Full Name", "Company", "Phone", "Address"};
    
    // Add custom field headers
    QStringList customFields;
    for (const ContactData &contact : dataToExport) {
        QMapIterator<QString, QString> it(contact.customFields);
        while (it.hasNext()) {
            it.next();
            if (!customFields.contains(it.key())) {
                customFields << it.key();
            }
        }
    }
    csvHeaders.append(customFields);
    
    // Escape and quote fields as needed
    QStringList quotedHeaders;
    for (const QString &header : csvHeaders) {
        quotedHeaders << QString("\"%1\"").arg(header);
    }
    stream << quotedHeaders.join(",") << "\n";
    
    // Write data rows
    for (const ContactData &contact : dataToExport) {
        QStringList row;
        row << QString("\"%1\"").arg(contact.email);
        row << QString("\"%1\"").arg(contact.firstName);
        row << QString("\"%1\"").arg(contact.lastName);
        row << QString("\"%1\"").arg(contact.fullName);
        row << QString("\"%1\"").arg(contact.company);
        row << QString("\"%1\"").arg(contact.phone);
        row << QString("\"%1\"").arg(contact.address);
        
        // Add custom fields
        for (const QString &field : customFields) {
            row << QString("\"%1\"").arg(contact.customFields.value(field, ""));
        }
        
        stream << row.join(",") << "\n";
    }
    
    file.close();
    return true;
}

QString CsvReader::cleanEmail(const QString &email)
{
    return email.toLower().trimmed().remove(QRegularExpression("[\\s\\t\\n\\r]"));
}

QString CsvReader::cleanName(const QString &name)
{
    return name.trimmed();
}

bool CsvReader::isValidEmailFormat(const QString &email)
{
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}

void CsvReader::clearData()
{
    contacts.clear();
    headers.clear();
    columnMapping.clear();
    lastError.clear();
}

int CsvReader::getValidContactCount() const
{
    int count = 0;
    for (const ContactData &contact : contacts) {
        if (contact.isValid()) count++;
    }
    return count;
}

QStringList CsvReader::getInvalidEmails()
{
    QStringList invalid;
    for (const ContactData &contact : contacts) {
        if (!contact.isValid()) {
            invalid << contact.email;
        }
    }
    return invalid;
}

QList<int> CsvReader::getDuplicateRows()
{
    QList<int> duplicates;
    QStringList emailsSeen;
    
    for (int i = 0; i < contacts.size(); ++i) {
        QString email = contacts[i].email.toLower();
        if (emailsSeen.contains(email)) {
            duplicates << i;
        } else {
            emailsSeen << email;
        }
    }
    
    return duplicates;
}

void CsvReader::removeDuplicates()
{
    QStringList emailsSeen;
    QList<ContactData> uniqueContacts;
    
    for (const ContactData &contact : contacts) {
        QString email = contact.email.toLower();
        if (!emailsSeen.contains(email)) {
            emailsSeen << email;
            uniqueContacts << contact;
        }
    }
    
    contacts = uniqueContacts;
}

void CsvReader::removeInvalidEmails()
{
    QList<ContactData> validContacts;
    for (const ContactData &contact : contacts) {
        if (contact.isValid()) {
            validContacts << contact;
        }
    }
    contacts = validContacts;
}

void CsvReader::cleanData()
{
    removeInvalidEmails();
    removeDuplicates();
}
