#include "templateeditor.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>

TemplateEditor::TemplateEditor(TemplateManager* templateManager, QWidget *parent)
    : QDialog(parent), templateManager(templateManager), currentTemplate(nullptr), isNewTemplate(true), hasUnsavedChanges(false)
{
    currentTemplate = new EmailTemplate();
    setupUI();
    connectSignals();
    populateTemplateTypes();
    updateUI();
    setWindowTitle("New Email Template");
}

TemplateEditor::TemplateEditor(TemplateManager* templateManager, EmailTemplate* emailTemplate, QWidget *parent)
    : QDialog(parent), templateManager(templateManager), currentTemplate(emailTemplate), isNewTemplate(false), hasUnsavedChanges(false)
{
    setupUI();
    connectSignals();
    populateTemplateTypes();
    loadTemplateData();
    updateUI();
    setWindowTitle(QString("Edit Template: %1").arg(emailTemplate->getName()));
}

TemplateEditor::~TemplateEditor()
{
    if (isNewTemplate && currentTemplate) {
        delete currentTemplate;
    }
}

void TemplateEditor::setupUI()
{
    setModal(true);
    resize(800, 600);
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    mainTabWidget = new QTabWidget();
    mainLayout->addWidget(mainTabWidget);
    
    // Template Info Tab
    QWidget* infoTab = new QWidget();
    QFormLayout* infoLayout = new QFormLayout(infoTab);
    
    nameEdit = new QLineEdit();
    subjectEdit = new QLineEdit();
    typeCombo = new QComboBox();
    categoryEdit = new QLineEdit();
    descriptionEdit = new QTextEdit();
    descriptionEdit->setMaximumHeight(80);
    tagsEdit = new QLineEdit();
    
    infoLayout->addRow("Name:", nameEdit);
    infoLayout->addRow("Subject:", subjectEdit);
    infoLayout->addRow("Type:", typeCombo);
    infoLayout->addRow("Category:", categoryEdit);
    infoLayout->addRow("Description:", descriptionEdit);
    infoLayout->addRow("Tags (comma-separated):", tagsEdit);
    
    mainTabWidget->addTab(infoTab, "Template Info");
    
    // Content Tab
    QWidget* contentTab = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentTab);
    
    // HTML Content
    QLabel* htmlLabel = new QLabel("HTML Content:");
    htmlEditor = new QTextEdit();
    htmlEditor->setPlaceholderText("Enter HTML content here...");
    
    contentLayout->addWidget(htmlLabel);
    contentLayout->addWidget(htmlEditor);
    
    // Text Content
    QLabel* textLabel = new QLabel("Plain Text Content:");
    textEditor = new QPlainTextEdit();
    textEditor->setPlaceholderText("Enter plain text content here...");
    
    contentLayout->addWidget(textLabel);
    contentLayout->addWidget(textEditor);
    
    mainTabWidget->addTab(contentTab, "Content");
    
    // Variables Tab
    setupVariablesPanel();
    
    // Preview Tab
    setupPreviewArea();
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    saveBtn = new QPushButton("Save");
    saveAsBtn = new QPushButton("Save As New");
    cancelBtn = new QPushButton("Cancel");
    previewBtn = new QPushButton("Update Preview");
    
    buttonLayout->addWidget(previewBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveAsBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(cancelBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void TemplateEditor::setupVariablesPanel()
{
    QWidget* variablesTab = new QWidget();
    QVBoxLayout* variablesLayout = new QVBoxLayout(variablesTab);
    
    variablesGroup = new QGroupBox("Template Variables");
    QVBoxLayout* groupLayout = new QVBoxLayout(variablesGroup);
    
    // Variables list
    variablesList = new QListWidget();
    groupLayout->addWidget(variablesList);
    
    // Add variable controls
    QHBoxLayout* addLayout = new QHBoxLayout();
    newVariableEdit = new QLineEdit();
    newVariableEdit->setPlaceholderText("Enter variable name (e.g., first_name)");
    addVariableBtn = new QPushButton("Add Variable");
    
    addLayout->addWidget(newVariableEdit);
    addLayout->addWidget(addVariableBtn);
    groupLayout->addLayout(addLayout);
    
    // Control buttons
    QHBoxLayout* controlLayout = new QHBoxLayout();
    insertVariableBtn = new QPushButton("Insert into Content");
    removeVariableBtn = new QPushButton("Remove Variable");
    
    controlLayout->addWidget(insertVariableBtn);
    controlLayout->addWidget(removeVariableBtn);
    controlLayout->addStretch();
    groupLayout->addLayout(controlLayout);
    
    variablesLayout->addWidget(variablesGroup);
    
    // Instructions
    QLabel* instructions = new QLabel(
        "Variables allow you to personalize emails. Use the format {{variable_name}} in your content.\n\n"
        "Common variables:\n"
        "• first_name, last_name - Recipient name\n"
        "• email - Recipient email\n"
        "• company_name - Your company name\n"
        "• custom_field - Any custom data"
    );
    instructions->setWordWrap(true);
    instructions->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }");
    variablesLayout->addWidget(instructions);
    
    mainTabWidget->addTab(variablesTab, "Variables");
}

void TemplateEditor::setupPreviewArea()
{
    QWidget* previewTab = new QWidget();
    QVBoxLayout* previewLayout = new QVBoxLayout(previewTab);
    
    QLabel* previewLabel = new QLabel("Template Preview (with sample data):");
    previewLayout->addWidget(previewLabel);
    
    previewTextEdit = new QTextEdit();
    previewTextEdit->setReadOnly(true);
    previewLayout->addWidget(previewTextEdit);
    
    mainTabWidget->addTab(previewTab, "Preview");
}

void TemplateEditor::connectSignals()
{
    // Template info signals
    connect(nameEdit, &QLineEdit::textChanged, this, &TemplateEditor::onNameChanged);
    connect(subjectEdit, &QLineEdit::textChanged, this, &TemplateEditor::onSubjectChanged);
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TemplateEditor::onTypeChanged);
    connect(categoryEdit, &QLineEdit::textChanged, this, &TemplateEditor::onCategoryChanged);
    connect(descriptionEdit, &QTextEdit::textChanged, this, &TemplateEditor::onDescriptionChanged);
    
    // Content signals
    connect(htmlEditor, &QTextEdit::textChanged, this, &TemplateEditor::onHtmlContentChanged);
    connect(textEditor, &QPlainTextEdit::textChanged, this, &TemplateEditor::onTextContentChanged);
    
    // Variables signals
    connect(addVariableBtn, &QPushButton::clicked, this, &TemplateEditor::onAddVariable);
    connect(removeVariableBtn, &QPushButton::clicked, this, &TemplateEditor::onRemoveVariable);
    connect(insertVariableBtn, &QPushButton::clicked, this, &TemplateEditor::onInsertVariable);
    connect(variablesList, &QListWidget::itemSelectionChanged, this, &TemplateEditor::onVariableSelected);
    
    // Button signals
    connect(saveBtn, &QPushButton::clicked, this, &TemplateEditor::saveTemplate);
    connect(saveAsBtn, &QPushButton::clicked, this, &TemplateEditor::saveAsTemplate);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(previewBtn, &QPushButton::clicked, this, &TemplateEditor::updatePreview);
}

void TemplateEditor::populateTemplateTypes()
{
    typeCombo->addItem("Custom", static_cast<int>(TemplateType::Custom));
    typeCombo->addItem("Newsletter", static_cast<int>(TemplateType::Newsletter));
    typeCombo->addItem("Promotional", static_cast<int>(TemplateType::Promotional));
    typeCombo->addItem("Welcome", static_cast<int>(TemplateType::Welcome));
    typeCombo->addItem("Transactional", static_cast<int>(TemplateType::Transactional));
    typeCombo->addItem("Event", static_cast<int>(TemplateType::Event));
    typeCombo->addItem("Announcement", static_cast<int>(TemplateType::Announcement));
}

void TemplateEditor::loadTemplateData()
{
    if (!currentTemplate) return;
    
    nameEdit->setText(currentTemplate->getName());
    subjectEdit->setText(currentTemplate->getSubject());
    htmlEditor->setHtml(currentTemplate->getHtmlContent());
    textEditor->setPlainText(currentTemplate->getTextContent());
    categoryEdit->setText(currentTemplate->getCategory());
    descriptionEdit->setPlainText(currentTemplate->getDescription());
    tagsEdit->setText(currentTemplate->getTags().join(", "));
    
    // Set type
    int typeIndex = typeCombo->findData(static_cast<int>(currentTemplate->getType()));
    if (typeIndex >= 0) {
        typeCombo->setCurrentIndex(typeIndex);
    }
    
    updateVariablesList();
    updatePreview();
}

void TemplateEditor::updateUI()
{
    if (!currentTemplate) return;
    
    bool isBuiltIn = templateManager && templateManager->isBuiltInTemplate(currentTemplate->getId());
    saveBtn->setEnabled(!isBuiltIn);
    
    if (isBuiltIn) {
        saveBtn->setText("Cannot Save (Built-in)");
        saveBtn->setToolTip("Built-in templates cannot be modified. Use 'Save As New' to create a copy.");
    }
}

void TemplateEditor::saveTemplate()
{
    if (!validateTemplate()) return;
    
    saveTemplateData();
    
    bool success = false;
    if (isNewTemplate) {
        success = templateManager->saveTemplate(currentTemplate);
        if (success) {
            isNewTemplate = false; // Template is now saved
        }
    } else {
        success = templateManager->updateTemplate(currentTemplate);
    }
    
    if (success) {
        hasUnsavedChanges = false;
        accept();
    } else {
        QMessageBox::warning(this, "Save Failed", "Failed to save the template. Please try again.");
    }
}

void TemplateEditor::saveAsTemplate()
{
    if (!validateTemplate()) return;
    
    // Create a new template
    EmailTemplate* newTemplate = new EmailTemplate();
    saveTemplateData(); // Save current data to currentTemplate
    
    // Copy data to new template
    newTemplate->setName(currentTemplate->getName() + " Copy");
    newTemplate->setSubject(currentTemplate->getSubject());
    newTemplate->setHtmlContent(currentTemplate->getHtmlContent());
    newTemplate->setTextContent(currentTemplate->getTextContent());
    newTemplate->setType(currentTemplate->getType());
    newTemplate->setCategory(currentTemplate->getCategory());
    newTemplate->setDescription(currentTemplate->getDescription());
    newTemplate->setTags(currentTemplate->getTags());
    
    if (templateManager->saveTemplate(newTemplate)) {
        QMessageBox::information(this, "Template Saved", 
            QString("Template saved as '%1'").arg(newTemplate->getName()));
        accept();
    } else {
        delete newTemplate;
        QMessageBox::warning(this, "Save Failed", "Failed to save the template. Please try again.");
    }
}

bool TemplateEditor::validateTemplate()
{
    QStringList errors;
    
    if (nameEdit->text().trimmed().isEmpty()) {
        errors.append("Template name is required");
    }
    
    if (subjectEdit->text().trimmed().isEmpty()) {
        errors.append("Template subject is required");
    }
    
    if (htmlEditor->toPlainText().trimmed().isEmpty() && textEditor->toPlainText().trimmed().isEmpty()) {
        errors.append("Template must have HTML or text content");
    }
    
    if (!errors.isEmpty()) {
        showValidationErrors(errors);
        return false;
    }
    
    return true;
}

void TemplateEditor::showValidationErrors(const QStringList &errors)
{
    QString message = "Please fix the following errors:\n\n";
    for (const QString &error : errors) {
        message += "• " + error + "\n";
    }
    QMessageBox::warning(this, "Validation Errors", message);
}

void TemplateEditor::saveTemplateData()
{
    if (!currentTemplate) return;
    
    currentTemplate->setName(nameEdit->text().trimmed());
    currentTemplate->setSubject(subjectEdit->text());
    currentTemplate->setHtmlContent(htmlEditor->toHtml());
    currentTemplate->setTextContent(textEditor->toPlainText());
    currentTemplate->setCategory(categoryEdit->text().trimmed());
    currentTemplate->setDescription(descriptionEdit->toPlainText().trimmed());
    
    QStringList tags;
    QString tagsText = tagsEdit->text().trimmed();
    if (!tagsText.isEmpty()) {
        QStringList rawTags = tagsText.split(',');
        for (const QString &tag : rawTags) {
            QString trimmedTag = tag.trimmed();
            if (!trimmedTag.isEmpty()) {
                tags.append(trimmedTag);
            }
        }
    }
    currentTemplate->setTags(tags);
    
    // Set type
    int typeIndex = typeCombo->currentIndex();
    if (typeIndex >= 0) {
        TemplateType type = static_cast<TemplateType>(typeCombo->itemData(typeIndex).toInt());
        currentTemplate->setType(type);
    }
}

QMap<QString, QString> TemplateEditor::getSampleVariables()
{
    QMap<QString, QString> sampleVars;
    sampleVars["first_name"] = "John";
    sampleVars["last_name"] = "Doe";
    sampleVars["email"] = "john.doe@example.com";
    sampleVars["company_name"] = "Your Company";
    sampleVars["company_address"] = "123 Main St, City, State";
    sampleVars["phone"] = "+1-555-123-4567";
    sampleVars["website"] = "https://yourcompany.com";
    return sampleVars;
}

// Slot implementations
void TemplateEditor::onNameChanged() { hasUnsavedChanges = true; }
void TemplateEditor::onSubjectChanged() { hasUnsavedChanges = true; }
void TemplateEditor::onHtmlContentChanged() { hasUnsavedChanges = true; }
void TemplateEditor::onTextContentChanged() { hasUnsavedChanges = true; }
void TemplateEditor::onTypeChanged() { hasUnsavedChanges = true; }
void TemplateEditor::onCategoryChanged() { hasUnsavedChanges = true; }
void TemplateEditor::onDescriptionChanged() { hasUnsavedChanges = true; }

void TemplateEditor::onVariableSelected()
{
    bool hasSelection = !variablesList->selectedItems().isEmpty();
    removeVariableBtn->setEnabled(hasSelection);
    insertVariableBtn->setEnabled(hasSelection);
}

void TemplateEditor::onAddVariable()
{
    QString variableName = newVariableEdit->text().trimmed();
    if (variableName.isEmpty()) return;
    
    if (!EmailTemplate::isValidVariable(variableName)) {
        QMessageBox::warning(this, "Invalid Variable", 
            "Variable names can only contain letters, numbers, underscores, and dashes.");
        return;
    }
    
    QString formattedVar = EmailTemplate::formatVariable(variableName);
    
    // Check if already exists
    for (int i = 0; i < variablesList->count(); ++i) {
        if (variablesList->item(i)->text() == formattedVar) {
            QMessageBox::information(this, "Variable Exists", "This variable already exists in the list.");
            return;
        }
    }
    
    variablesList->addItem(formattedVar);
    newVariableEdit->clear();
    hasUnsavedChanges = true;
}

void TemplateEditor::onRemoveVariable()
{
    QListWidgetItem* item = variablesList->currentItem();
    if (item) {
        delete item;
        hasUnsavedChanges = true;
    }
}

void TemplateEditor::onInsertVariable()
{
    QListWidgetItem* item = variablesList->currentItem();
    if (!item) return;
    
    QString variable = item->text();
    
    // Insert into current editor
    QWidget* currentWidget = mainTabWidget->currentWidget();
    if (mainTabWidget->indexOf(currentWidget) == 1) { // Content tab
        QTextEdit* activeEditor = htmlEditor;
        if (activeEditor->hasFocus()) {
            activeEditor->insertPlainText(variable);
        } else if (textEditor->hasFocus()) {
            textEditor->insertPlainText(variable);
        } else {
            // Default to HTML editor
            htmlEditor->insertPlainText(variable);
        }
        hasUnsavedChanges = true;
    }
}

void TemplateEditor::updateVariablesList()
{
    if (!currentTemplate) return;
    
    variablesList->clear();
    QStringList variables = currentTemplate->extractVariables();
    for (const QString &variable : variables) {
        variablesList->addItem(EmailTemplate::formatVariable(variable));
    }
}

void TemplateEditor::updatePreview()
{
    if (!currentTemplate) return;
    
    // Save current data to template
    saveTemplateData();
    
    // Generate preview with sample data
    QMap<QString, QString> sampleVars = getSampleVariables();
    QString processedHtml = currentTemplate->processTemplate(sampleVars);
    QString processedSubject = currentTemplate->processSubject(sampleVars);
    
    QString preview = QString("<h3>Subject: %1</h3><hr>%2").arg(processedSubject, processedHtml);
    previewTextEdit->setHtml(preview);
    
    // Switch to preview tab
    mainTabWidget->setCurrentIndex(3);
}

void TemplateEditor::setTemplate(EmailTemplate* emailTemplate)
{
    currentTemplate = emailTemplate;
    isNewTemplate = false;
    loadTemplateData();
    updateUI();
}

void TemplateEditor::newTemplate()
{
    if (currentTemplate && isNewTemplate) {
        delete currentTemplate;
    }
    currentTemplate = new EmailTemplate();
    isNewTemplate = true;
    hasUnsavedChanges = false;
    
    // Clear UI
    nameEdit->clear();
    subjectEdit->clear();
    htmlEditor->clear();
    textEditor->clear();
    categoryEdit->clear();
    descriptionEdit->clear();
    tagsEdit->clear();
    typeCombo->setCurrentIndex(0);
    variablesList->clear();
    previewTextEdit->clear();
    
    updateUI();
}

void TemplateEditor::previewTemplate()
{
    updatePreview();
}
