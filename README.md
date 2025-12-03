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

## Features

### Core Features
- **Bulk Email Sending**: Send emails to multiple recipients with customizable content
- **Email Campaign Management**: Create, track, and manage email campaigns
- **Real-time Status Tracking**: Monitor email delivery status (sent, failed, pending)
- **Data Cleaning**: Clean and validate email addresses automatically
- **Campaign History**: View and export campaign history with detailed statistics

### User Interface
- **Modern Dark Theme**: Beautiful dark blue and dark gold color scheme
- **Tabbed Interface**: Organized into Email Management, Campaign History, and Data Cleaning tabs
- **Responsive Design**: Clean and intuitive user interface
- **Progress Tracking**: Real-time progress bars for bulk operations

### Data Management
- **SQLite Database**: Local data storage for campaigns and email lists
- **CSV Import/Export**: Import email lists from CSV files and export results
- **Email Validation**: Comprehensive email validation with format checking
- **Duplicate Detection**: Automatic detection and removal of duplicate emails

### Technical Features
- **SMTP Support**: Configurable SMTP server settings
- **Email Validation**: RFC 5322 compliant email validation
- **Error Handling**: Comprehensive error handling and user feedback
- **Cross-platform Ready**: Built with Qt6 for potential cross-platform support

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
- **Qt6**: Version 6.2 or later
- **CMake**: Version 3.16 or later
- **C++ Compiler**: MSVC 2019 or later, or MinGW-w64
- **Build Tools**: Visual Studio 2019/2022 or Qt Creator

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

The application uses SQLite for local data storage:

- **Database Location**: `%APPDATA%/BulkEmailManager/bulk_email_manager.db`
- **Tables**:
  - `campaigns`: Email campaign information
  - `emails`: Email addresses and delivery status

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
│   ├── main.cpp              # Application entry point
│   ├── mainwindow.h/cpp      # Main window implementation
│   ├── emailmanager.h/cpp    # Email sending functionality
│   ├── emailcampaign.h/cpp   # Campaign management
│   ├── database.h/cpp        # Data persistence
│   ├── emailvalidator.h/cpp  # Email validation
│   └── mainwindow.ui         # UI definition
├── CMakeLists.txt            # Build configuration
└── README.md                 # This file
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

### v1.0.0 (Current)
- Initial release
- Basic email sending functionality
- Campaign management
- Data cleaning features
- Modern dark theme UI
- SQLite database support
- CSV import/export
- Email validation

---

**Note**: This application is designed for legitimate email marketing purposes. Please ensure compliance with email marketing laws and regulations in your jurisdiction.