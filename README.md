# Bulk Email Manager

A standalone C++ application for Windows that allows sending bulk emails with comprehensive data cleaning and validation features. Built with Qt6 framework for a modern, responsive GUI.

## Features

### Core Functionality
- **Bulk Email Sending**: Send emails to multiple recipients with customizable content
- **Email Validation**: Comprehensive validation including format, domain, disposable email detection, and role-based email filtering
- **Data Cleaning**: Remove invalid emails and update delivery status for enhanced data quality
- **Campaign Management**: Track email campaigns with detailed statistics and history
- **SMTP Configuration**: Flexible SMTP server settings with connection testing

### GUI Features
- **Modern Dark Theme**: Beautiful dark blue and gold color scheme
- **Tabbed Interface**: Organized tabs for email management, campaign history, and settings
- **Real-time Updates**: Live status updates and progress tracking
- **Import/Export**: CSV file support for email list management
- **Multi-threading**: Responsive UI with background email processing

### Data Management
- **SQLite Database**: Local storage for emails, campaigns, and settings
- **Email Tracking**: Track sent, received, and failed email status
- **Campaign History**: View detailed campaign statistics and dates
- **Data Export**: Export results and campaign data

## Build Requirements

### Prerequisites
- **Qt6**: Core, Widgets, Network, and Sql modules
- **CMake**: Version 3.16 or higher
- **C++ Compiler**: Supporting C++17 standard
- **Windows**: Visual Studio 2019/2022 or MinGW

### Dependencies
- Qt6 Core
- Qt6 Widgets
- Qt6 Network
- Qt6 Sql

## Building the Application

### Using CMake

1. **Clone or download the project files**
   ```bash
   mkdir bulk-email-manager
   cd bulk-email-manager
   # Copy all source files to this directory
   ```

2. **Create build directory**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**
   ```bash
   cmake ..
   ```

4. **Build the application**
   ```bash
   cmake --build . --config Release
   ```

### Using Qt Creator

1. **Open Qt Creator**
2. **Open Project**: Select the `CMakeLists.txt` file
3. **Configure Project**: Select your Qt6 kit
4. **Build**: Press Ctrl+B or use the Build menu

## Usage

### First Run
1. **Launch the application**
2. **Configure SMTP Settings**:
   - Go to the Settings tab
   - Enter your SMTP server details
   - Test the connection
   - Save settings

### Sending Bulk Emails

1. **Prepare Email List**:
   - Use the Import button to load a CSV file with email addresses
   - Or manually add emails to the list
   - Validate emails using the Validate button

2. **Compose Email**:
   - Enter the email subject
   - Write your email content in the text area
   - Set sender information

3. **Send Emails**:
   - Click the Send button
   - Monitor progress in the status bar
   - View results in the email table

### Data Cleaning

1. **Validate Emails**:
   - Click Validate to check email format and quality
   - Review validation results
   - Remove invalid emails

2. **Clean Data**:
   - Use the Clean button to remove invalid emails
   - Update delivery status for received/unreceived emails
   - Export cleaned data

### Campaign Management

1. **View Campaign History**:
   - Go to the Campaign History tab
   - Set date range to filter campaigns
   - View detailed statistics

2. **Export Results**:
   - Export email lists and campaign data
   - Generate reports for analysis

## File Structure

```
bulk-email-manager/
├── CMakeLists.txt          # Build configuration
├── README.md              # This file
├── src/
│   ├── main.cpp           # Application entry point
│   ├── mainwindow.h       # Main window header
│   ├── mainwindow.cpp     # Main window implementation
│   ├── emailmanager.h     # Email management header
│   ├── emailmanager.cpp   # Email management implementation
│   ├── database.h         # Database operations header
│   ├── database.cpp       # Database operations implementation
│   ├── emailvalidator.h   # Email validation header
│   └── emailvalidator.cpp # Email validation implementation
```

## Configuration

### SMTP Settings
- **Server**: Your SMTP server address (e.g., smtp.gmail.com)
- **Port**: SMTP port (usually 587 for TLS or 465 for SSL)
- **Username**: Your email address
- **Password**: Your email password or app-specific password
- **Security**: TLS/SSL encryption

### Email Validation
The application validates emails using multiple criteria:
- **Format Validation**: RFC 5322 compliant email format
- **Domain Validation**: Valid domain structure
- **Disposable Email Detection**: Filters out temporary email services
- **Role-based Email Detection**: Identifies generic role accounts
- **Spam Pattern Detection**: Identifies suspicious email patterns

## Database

The application uses SQLite for local data storage:
- **Emails Table**: Stores email addresses, status, and delivery information
- **Campaigns Table**: Tracks campaign history and statistics
- **Settings Table**: Stores application configuration

Database location: `%APPDATA%/Bulk Email Manager/bulk_email.db`

## Troubleshooting

### Build Issues
- **Qt6 not found**: Ensure Qt6 is installed and CMake can find it
- **Missing modules**: Install Qt6 Core, Widgets, Network, and Sql modules
- **Compiler errors**: Ensure your compiler supports C++17

### Runtime Issues
- **SMTP Connection Failed**: Check your SMTP settings and network connection
- **Database Errors**: Ensure write permissions in the application data directory
- **Email Validation Issues**: Check email format and domain validity

### Performance
- **Slow Email Sending**: The application uses multi-threading for better performance
- **Large Email Lists**: Consider splitting very large lists into smaller batches
- **Memory Usage**: Monitor memory usage with very large email lists

## Security Notes

- **SMTP Credentials**: Store credentials securely, consider using app-specific passwords
- **Email Privacy**: Respect recipient privacy and anti-spam laws
- **Data Protection**: Email lists are stored locally, ensure proper data protection

## License

This application is provided as-is for educational and personal use. Ensure compliance with email regulations and anti-spam laws in your jurisdiction.

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review the Qt6 documentation for framework-specific issues
3. Ensure all dependencies are properly installed

## Future Enhancements

Potential improvements for future versions:
- **Online Email Validation**: Integration with email validation APIs
- **Advanced Templates**: Email template system with variables
- **Scheduling**: Schedule email campaigns for later delivery
- **Analytics**: Advanced campaign analytics and reporting
- **Multi-language Support**: Internationalization support
- **Cloud Integration**: Sync with cloud email services