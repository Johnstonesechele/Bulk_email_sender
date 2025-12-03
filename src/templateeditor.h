#ifndef TEMPLATEEDITOR_H
#define TEMPLATEEDITOR_H

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPlainTextEdit>
// #include <QWebEngineView> // Temporarily commented out
#include "emailtemplate.h"
#include "templatemanager.h"

class TemplateEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TemplateEditor(TemplateManager* templateManager, QWidget *parent = nullptr);
    explicit TemplateEditor(TemplateManager* templateManager, EmailTemplate* emailTemplate, QWidget *parent = nullptr);
    ~TemplateEditor();

    EmailTemplate* getTemplate() const { return currentTemplate; }

public slots:
    void setTemplate(EmailTemplate* emailTemplate);
    void newTemplate();
    void saveTemplate();
    void saveAsTemplate();
    void previewTemplate();

private slots:
    void onNameChanged();
    void onSubjectChanged();
    void onHtmlContentChanged();
    void onTextContentChanged();
    void onTypeChanged();
    void onCategoryChanged();
    void onDescriptionChanged();
    void onVariableSelected();
    void onAddVariable();
    void onRemoveVariable();
    void onInsertVariable();
    void updateVariablesList();
    void updatePreview();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupMainArea();
    void setupVariablesPanel();
    void setupPreviewArea();
    void connectSignals();
    void updateUI();
    void populateTemplateTypes();
    void loadTemplateData();
    void saveTemplateData();
    bool validateTemplate();
    void showValidationErrors(const QStringList &errors);
    QMap<QString, QString> getSampleVariables();

    // UI Components
    QTabWidget* mainTabWidget;
    QSplitter* mainSplitter;
    
    // Template Info
    QLineEdit* nameEdit;
    QLineEdit* subjectEdit;
    QComboBox* typeCombo;
    QLineEdit* categoryEdit;
    QTextEdit* descriptionEdit;
    QLineEdit* tagsEdit;
    
    // Content Editors
    QTextEdit* htmlEditor;
    QPlainTextEdit* textEditor;
    
    // Variables Panel
    QGroupBox* variablesGroup;
    QListWidget* variablesList;
    QLineEdit* newVariableEdit;
    QPushButton* addVariableBtn;
    QPushButton* removeVariableBtn;
    QPushButton* insertVariableBtn;
    
    // Preview
    QWebEngineView* previewWebView;
    QTextEdit* previewTextEdit;
    
    // Buttons
    QPushButton* saveBtn;
    QPushButton* saveAsBtn;
    QPushButton* cancelBtn;
    QPushButton* previewBtn;
    
    // Data
    TemplateManager* templateManager;
    EmailTemplate* currentTemplate;
    bool isNewTemplate;
    bool hasUnsavedChanges;
};

#endif // TEMPLATEEDITOR_H
