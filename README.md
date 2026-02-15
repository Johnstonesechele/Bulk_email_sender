# Bulk Email Manager - Professional Edition

A modern, feature-rich bulk email application built with Qt6 and C++ for Windows. This application provides a comprehensive solution for managing email campaigns, tracking delivery status, and cleaning email data.

## 🆕 **NEW: Email Template System**

### Template Features
- **Built-in Templates**: Pre-designed templates for common use cases
  - Welcome emails
  - Newsletters  
  - Promotional campaigns
  - Event invitations
  - Company announcements

- **Custom Templates**: Create your own email templates
- **Variable/Merge Fields**: Personalize emails with `{{variable_name}}` syntax
- **Template Management**: Save, edit, delete, and organize templates
- **Live Preview**: See how templates look with sample data
- **Template Categories**: Organize templates by type and category

### Template Variables
Common variables you can use:
- `{{first_name}}`, `{{last_name}}` - Recipient name
- `{{email}}` - Recipient email address
- `{{company_name}}` - Your company name
- `{{company_address}}` - Your company address
- `{{phone}}`, `{{website}}` - Contact information
- Any custom variables you define

### How to Use Templates
1. **Select Template**: Choose from dropdown in Email Management tab
2. **Create New**: Click "New Template" to create custom template
3. **Edit Existing**: Select template and click "Edit Template"
4. **Personalize**: Templates automatically populate with variable data
5. **Preview**: Use the template editor's preview feature

## 🚀 **NEW: Advanced Features**

### Campaign Analytics & Reporting
- **Real-time Analytics**: Track email delivery rates, bounce rates, and campaign performance
- **Interactive Dashboard**: Visual charts and graphs showing campaign metrics
- **Comprehensive Reports**: Export detailed analytics in JSON, CSV, or text format
- **Provider Analysis**: Track delivery rates by email provider (Gmail, Outlook, etc.)
- **Time-based Tracking**: Monitor campaign performance over time
- **Error Analysis**: Detailed breakdown of delivery errors and bounce types

### Advanced Logging System
- **Multi-level Logging**: Debug, Info, Warning, Error, and Critical levels
- **File and Console Logging**: Configurable output destinations
- **Log Rotation**: Automatic log file rotation and cleanup
- **Search and Filter**: Find specific log entries by category or time range
- **Export Logs**: Export logs for debugging or compliance purposes

### Contact List Management
- **Multiple Contact Lists**: Create and manage separate contact lists
- **Advanced Search**: Search contacts by name, email, company, or tags
- **Tag System**: Organize contacts with custom tags
- **Duplicate Detection**: Automatic detection and merging of duplicate contacts
- **Blacklist Management**: Maintain email blacklists with bounce handling
- **Import/Export**: Full CSV import/export with validation
- **Custom Fields**: Store additional contact information in JSON format

### Application Settings & Preferences
- **Comprehensive Settings**: Configure every aspect of the application
- **SMTP Presets**: Quick setup for popular email providers
- **Theme Customization**: Dark/Light theme support with custom fonts
- **Auto-save**: Automatic saving of work with configurable intervals
- **Backup Management**: Automatic database backups with retention policies
- **Settings Import/Export**: Backup and restore application settings

### Enhanced Email Features
- **HTML Email Builder**: Rich HTML email creation with template variables
- **Email Preview**: Preview emails before sending with template rendering
- **Send Delay**: Configurable delays between emails to avoid rate limiting
- **Retry Logic**: Automatic retry on failed deliveries with exponential backoff
- **Connection Pooling**: Efficient SMTP connection management
- **Network Diagnostics**: Built-in network testing and DNS resolution

### Data Management
- **Advanced Validation**: RFC 5322 compliant email validation
- **Data Cleaning**: Remove invalid emails, duplicates, and blacklisted addresses
- **Backup & Restore**: Full database backup and restore functionality
- **Data Retention**: Configurable data retention policies for compliance
- **Export Options**: Multiple export formats (CSV, JSON, XML)

## Features

### Core Features
- **Bulk Email Sending**: Send emails to multiple recipients with customizable content
- **Email Campaign Management**: Create, track, and manage email campaigns with full analytics
- **Real-time Status Tracking**: Monitor email delivery status with detailed progress tracking
- **Data Cleaning**: Advanced email validation and list management tools
- **Campaign History**: Comprehensive campaign history with detailed analytics and reporting

### User Interface
- **Modern Dark Theme**: Beautiful dark blue and dark gold color scheme with customizable themes
- **Tabbed Interface**: Organized into Email Management, Campaign History, Analytics Dashboard, and Settings
- **Responsive Design**: Clean and intuitive user interface with professional styling
- **Progress Tracking**: Real-time progress bars and status updates for all operations
- **Interactive Charts**: Visual analytics dashboard with delivery metrics and performance graphs

### Data Management
- **SQLite Database**: Local data storage for campaigns, contacts, analytics, and email lists
- **CSV/Excel Import/Export**: Import email lists from CSV files and export comprehensive results
- **Contact List Management**: Advanced contact organization with tags, custom fields, and multiple lists
- **Email Validation**: Comprehensive email validation with RFC 5322 compliance
- **Duplicate Detection**: Automatic detection and removal of duplicate emails across all lists
- **Blacklist Management**: Sophisticated email blacklist with bounce tracking and automatic management

### Technical Features
- **Advanced SMTP Support**: Full SMTP protocol with SSL/TLS, STARTTLS, connection pooling, and retry logic
- **Network Diagnostics**: Built-in DNS resolution testing, connection validation, and network troubleshooting
- **Comprehensive Analytics**: Real-time tracking with delivery metrics, provider analysis, and error reporting
- **Advanced Logging**: Multi-level logging with file rotation, search capabilities, and export options
- **Email Validation**: RFC 5322 compliant validation with domain verification and format checking
- **Error Handling**: Comprehensive error handling with detailed user feedback and automatic recovery
- **Cross-platform Ready**: Built with Qt6 for potential cross-platform support with Windows optimization

## Screenshots

The application features a modern dark theme with:
- Dark blue background (#1a2332)
- Dark gold accents (#FFD700)
- Clean, professional interface
- Tabbed navigation
- Real-time status updates

## Requirements

### System Requirements
- **Operating System**: Windows 10/11 (64-bit)
- **Memory**: 4GB RAM minimum, 8GB recommended
- **Storage**: 100MB free disk space
- **Network**: Internet connection for email sending

### Development Requirements
- **Qt6**: Version 6.2 or later with Charts module
- **CMake**: Version 3.16 or later
- **C++ Compiler**: MSVC 2019 or later, or MinGW-w64
- **Build Tools**: Visual Studio 2019/2022 or Qt Creator
- **Additional Qt Modules**: Core, Widgets, Network, Sql, Charts

## Installation

### Option 1: Build from Source

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd bulk-email-manager
   ```

2. **Install Qt6**
   - Download Qt6 from [qt.io](https://www.qt.io/download)
   - Install Qt6 with the following components:
     - Qt 6.2+ (Core, Widgets, Network, Sql)
     - Qt Creator (optional)
     - CMake

3. **Configure the build**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.2.0/msvc2019_64"
   ```

4. **Build the application**
   ```bash
   cmake --build . --config Release
   ```

5. **Run the application**
   ```bash
   ./Release/BulkEmailApp.exe
   ```

### Option 2: Using Qt Creator

1. Open Qt Creator
2. Open the `CMakeLists.txt` file
3. Configure the project with your Qt6 installation
4. Build and run the project

## Usage

### Getting Started

1. **Configure SMTP Settings**
   - Enter your SMTP server details (e.g., smtp.gmail.com)
   - Provide your email credentials
   - Test the connection

2. **Create Email Content**
   - Enter a subject line
   - Write your email message
   - Use HTML formatting if needed

3. **Add Recipients**
   - Manually add email addresses
   - Import from CSV file
   - Validate email addresses

4. **Send Emails**
   - Review your email list
   - Click "Send Bulk Emails"
   - Monitor progress in real-time

### Email Management Tab

- **Email Configuration**: Set up SMTP server and sender information
- **Email Content**: Create email subject and message
- **Email List**: Manage recipient list with add/remove/import options
- **Send Controls**: Load, save, and send email campaigns

### Campaign History Tab

- **Campaign Overview**: View all email campaigns
- **Statistics**: See delivery statistics and success rates
- **Export Results**: Export campaign data to CSV

### Data Cleaning Tab

- **Email Validation**: Validate email addresses
- **Duplicate Removal**: Remove duplicate emails
- **Data Export**: Export cleaned email lists

## Configuration

### SMTP Settings

The application supports various SMTP providers:

**Gmail**
- Server: smtp.gmail.com
- Port: 587
- Security: STARTTLS

**Outlook/Hotmail**
- Server: smtp-mail.outlook.com
- Port: 587
- Security: STARTTLS

**Custom SMTP**
- Configure your own SMTP server settings
- Support for custom ports and security settings

### Email Validation

The application includes comprehensive email validation:

- **Format Validation**: RFC 5322 compliant email format checking
- **Domain Validation**: Validates email domains
- **Disposable Email Detection**: Identifies disposable email addresses
- **Role-based Email Detection**: Identifies role-based email addresses

## Data Storage

The application uses SQLite for comprehensive data storage:

- **Database Location**: `%APPDATA%/BulkEmailManager/`
  - `bulk_email_manager.db` - Main application database
  - `contacts.db` - Contact list management
  - `analytics.db` - Campaign analytics and reporting
- **Tables**:
  - `campaigns`: Email campaign information and metadata
  - `emails`: Email addresses, delivery status, and tracking
  - `contacts`: Contact information with custom fields
  - `contact_lists`: Multiple contact list management
  - `email_deliveries`: Detailed delivery tracking and analytics
  - `email_blacklist`: Blacklisted email addresses with reasons
  - `email_templates`: HTML email templates and variables
- **Backup**: Automatic database backups with configurable retention
- **Logs**: Comprehensive application logs in `%APPDATA%/BulkEmailManager/logs/`

## Troubleshooting

### Common Issues

1. **SMTP Connection Failed**
   - Verify SMTP server settings
   - Check firewall settings
   - Ensure correct credentials

2. **Email Validation Errors**
   - Check email format
   - Verify domain exists
   - Remove invalid characters

3. **Build Errors**
   - Ensure Qt6 is properly installed
   - Check CMake version
   - Verify compiler compatibility

### Support

For technical support and issues:
- Check the troubleshooting section
- Review error messages in the status bar
- Verify system requirements

## Development

### Project Structure

```
bulk-email-manager/
├── src/
│   ├── main.cpp                    # Application entry point
│   ├── mainwindow.h/cpp           # Main window implementation
│   ├── emailmanager.h/cpp         # Email sending functionality
│   ├── emailcampaign.h/cpp        # Campaign management
│   ├── database.h/cpp             # Data persistence
│   ├── emailvalidator.h/cpp       # Email validation
│   ├── emailtemplate.h/cpp        # Email template management
│   ├── templatemanager.h/cpp      # Template organization
│   ├── templateeditor.h/cpp       # Template editor UI
│   ├── htmlemailbuilder.h/cpp     # HTML email construction
│   ├── csvreader.h/cpp            # CSV import/export
│   ├── smtpemailsender.h/cpp      # SMTP email delivery
│   ├── analytics.h/cpp            # Campaign analytics
│   ├── logger.h/cpp               # Advanced logging system
│   ├── contactlistmanager.h/cpp   # Contact management
│   ├── appsettings.h/cpp          # Application settings
│   ├── settingsdialog.h/cpp       # Settings UI
│   ├── analyticsdashboard.h/cpp   # Analytics dashboard
│   └── mainwindow.ui              # UI definition
├── templates/                      # Built-in email templates
├── docs/                          # Documentation
│   └── SMTP_Configuration_Guide.md
├── CMakeLists.txt                 # Build configuration
├── build.bat                      # Windows build script
├── run.bat                        # Quick run script
└── README.md                      # This file
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Built with Qt6 framework
- Uses SQLite for data storage
- Modern C++17 features
- Professional UI design

## Version History

### v2.0.0 (Professional Edition - Current)
- **Campaign Analytics & Reporting**: Real-time analytics dashboard with interactive charts
- **Advanced Contact Management**: Multiple contact lists with tags, custom fields, and search
- **Professional Email Templates**: HTML email builder with 5+ built-in templates and variables
- **Advanced Logging System**: Multi-level logging with file rotation and export capabilities
- **Comprehensive Settings**: Full application configuration with import/export support
- **Network Diagnostics**: Built-in connection testing and DNS resolution tools
- **Enhanced SMTP**: Connection pooling, retry logic, and detailed error reporting
- **Data Analytics**: Provider analysis, delivery tracking, and performance metrics
- **Backup & Restore**: Automatic database backups with configurable retention
- **Professional UI**: Analytics dashboard, settings dialog, and improved user experience

### v1.0.0 (Initial Release)
- Initial release with basic email sending functionality
- Campaign management and tracking
- Data cleaning features with email validation
- Modern dark theme UI with professional styling
- SQLite database support for local storage
- CSV import/export capabilities
- RFC 5322 compliant email validation
- Basic SMTP configuration and sending

---

**Note**: This application is designed for legitimate email marketing purposes. Please ensure compliance with email marketing laws and regulations in your jurisdiction.