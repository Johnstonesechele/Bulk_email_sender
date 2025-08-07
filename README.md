# Bulk Email Manager

A professional bulk email management application built with C++ and Qt6, featuring a beautiful dark blue and gold themed GUI. This application allows you to send bulk emails, track delivery status, manage contacts, and analyze campaign performance.

## Features

### 🎨 Beautiful Modern UI
- Dark blue and gold color scheme
- Professional tabbed interface
- Responsive design with modern styling
- High DPI support for crisp display

### 📧 Email Management
- **Contact Management**: Add, remove, import/export contacts via CSV
- **Bulk Email Sending**: Send emails to multiple recipients with customizable delays
- **Email Templates**: Pre-built templates for newsletters, promotions, and welcome emails
- **SMTP Configuration**: Support for Gmail, Outlook, and custom SMTP servers

### 📊 Campaign Tracking
- **Real-time Status Tracking**: Monitor sent, delivered, and bounced emails
- **Campaign History**: View all past campaigns with detailed statistics
- **Analytics Dashboard**: Delivery rates, open rates, and click-through rates
- **Export Reports**: Generate detailed campaign reports in CSV format

### 🔧 Advanced Features
- **Data Cleaning**: Track bounced emails for list hygiene
- **Backup & Restore**: Automatic data backup and restore functionality
- **Settings Management**: Persistent configuration storage
- **Windows Integration**: Native Windows application with proper manifest

## Screenshots

The application features four main tabs:
- **Contacts**: Manage your email lists with import/export functionality
- **Campaign**: Compose and send bulk emails with progress tracking
- **History**: View past campaigns with detailed analytics
- **Settings**: Configure SMTP settings and application preferences

## System Requirements

- **Operating System**: Windows 7/8/8.1/10/11
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 100MB free space
- **Network**: Internet connection for sending emails

## Building from Source

### Prerequisites

1. **Qt6**: Download and install from [qt.io](https://www.qt.io/download)
2. **CMake**: Download from [cmake.org](https://cmake.org/download/)
3. **MinGW or Visual Studio**: C++ compiler

### Windows Build Instructions

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd BulkEmailApp
   ```

2. **Run the build script**:
   ```bash
   build-windows.bat
   ```

3. **Manual build** (alternative):
   ```bash
   mkdir build
   cd build
   cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --config Release
   windeployqt BulkEmailApp.exe
   ```

### Dependencies

The application uses the following Qt6 modules:
- Qt6Core
- Qt6Widgets
- Qt6Network

All dependencies will be automatically linked during the build process.

## Configuration

### SMTP Settings

Configure your email provider settings in the Settings tab:

**Gmail Example**:
- SMTP Server: `smtp.gmail.com`
- Port: `587`
- Username: Your Gmail address
- Password: App-specific password (recommended)

**Outlook Example**:
- SMTP Server: `smtp-mail.outlook.com`
- Port: `587`
- Username: Your Outlook address
- Password: Your account password

### Security Notes

- Use app-specific passwords when available
- Enable 2-factor authentication for your email accounts
- Store credentials securely

## Usage Guide

### 1. Import Contacts
- Go to the **Contacts** tab
- Click **Import CSV** to load your contact list
- Format: `Name,Email` (header row optional)

### 2. Compose Campaign
- Switch to the **Campaign** tab
- Enter your subject line and message
- Choose from available templates or create custom content

### 3. Send Bulk Emails
- Click **Send Bulk Email** to start the campaign
- Monitor progress in real-time
- View delivery status updates

### 4. Analyze Results
- Check the **History** tab for campaign analytics
- Export detailed reports for further analysis
- Track bounced emails for list cleaning

## Data Management

### File Locations
- **Contacts**: Stored in CSV format for easy editing
- **Campaigns**: JSON format with full analytics data
- **Settings**: JSON configuration file
- **Backups**: Automatic backups in dedicated folder

### Data Export Options
- Contact lists (CSV)
- Campaign reports (CSV/TXT)
- Full database backup (ZIP)

## Troubleshooting

### Common Issues

1. **SMTP Connection Failed**:
   - Verify server settings and credentials
   - Check firewall and antivirus settings
   - Ensure app-specific passwords are used

2. **High Bounce Rate**:
   - Clean your contact list regularly
   - Remove invalid email addresses
   - Use double opt-in for new subscribers

3. **Slow Sending**:
   - Increase delay between emails in settings
   - Check your SMTP provider's rate limits
   - Consider upgrading your email service plan

## License

This project is proprietary software. All rights reserved.

## Support

For technical support and feature requests, please contact:
- Email: support@emailpro.com
- Documentation: [Online Help Center]

## Version History

### v1.0.0 (Current)
- Initial release
- Full bulk email functionality
- Campaign tracking and analytics
- Windows native application
- Dark blue and gold themed UI

---

**Built with ❤️ using C++ and Qt6**