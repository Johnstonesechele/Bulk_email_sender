# Bulk Email Manager - SMTP Configuration Guide

## SMTP Configuration Options

The Bulk Email Manager now includes improved SMTP configuration with multiple encryption options:

### Encryption Types

1. **None** (Port 25)
   - No encryption
   - Use for testing or internal mail servers
   - Not recommended for external email providers

2. **SSL/TLS** (Port 465)
   - Direct SSL/TLS connection
   - Used by some email providers
   - Connection is encrypted from the start

3. **STARTTLS** (Port 587) - **Recommended**
   - Plain connection upgraded to TLS
   - Most common for external email providers
   - Default setting for Gmail, Outlook, etc.

### Common Email Provider Settings

#### Gmail
- SMTP Server: smtp.gmail.com
- Port: 587
- Encryption: STARTTLS
- Username: your-email@gmail.com
- Password: App Password (not regular password)

#### Outlook/Hotmail
- SMTP Server: smtp-mail.outlook.com
- Port: 587
- Encryption: STARTTLS
- Username: your-email@outlook.com
- Password: Your account password

#### Yahoo
- SMTP Server: smtp.mail.yahoo.com
- Port: 587
- Encryption: STARTTLS
- Username: your-email@yahoo.com
- Password: App Password

### Testing Connection

1. Fill in your SMTP settings
2. Click "Test Connection" button
3. The application will attempt to connect and authenticate
4. Success/failure will be shown in a dialog

### Features

- **HTML Email Support**: Create rich HTML emails using templates or manual HTML code
- **Template System**: Professional email templates with variable substitution
- **CSV/Excel Import**: Import contact lists from files
- **Email Validation**: Validate email addresses before sending
- **Progress Tracking**: Real-time progress during bulk sending
- **Error Handling**: Detailed error messages for troubleshooting

### HTML Email Preview

- Use "HTML Mode" to edit raw HTML
- Use "Rich Text Mode" for visual editing
- Click "Preview" to see how emails will appear
- Variables like {{name}} and {{email}} are automatically replaced

### Variable Substitution

Available variables in templates and emails:
- {{name}} - Recipient's name
- {{email}} - Recipient's email address
- {{company}} - Company name (if provided)
- {{subject}} - Email subject line

### Troubleshooting

1. **Failed to receive server greeting**
   - The most common issue - server may be slow to respond
   - Try different encryption settings:
     - Start with "None" encryption on port 25 for testing
     - Then try "STARTTLS" on port 587
     - Finally try "SSL/TLS" on port 465
   - Check if firewall/antivirus is blocking connections
   - Ensure the SMTP server address is correct

2. **TLS Initialization Failed**
   - Try different encryption types
   - Check if port number matches encryption type
   - Ensure firewall allows SMTP connections
   - Some corporate networks block TLS

3. **Authentication Failed**
   - Use App Passwords for Gmail/Yahoo (not regular password)
   - Check username/password are correct
   - For Gmail: Enable 2-factor auth and create app password
   - Some providers require "Less Secure Apps" enabled

4. **Connection Timeout**
   - Check network connection
   - Verify SMTP server address is reachable
   - Try ping smtp.gmail.com in command prompt
   - Try different ports (25, 465, 587)
   - Corporate firewalls often block these ports

5. **SSL Certificate Errors**
   - Application ignores non-critical SSL errors automatically
   - Ensure system date/time is correct
   - Some corporate networks may block SSL

### Quick Testing Steps

1. **First, test basic connectivity:**
   - Server: smtp.gmail.com
   - Port: 25
   - Encryption: None
   - No username/password
   - Click "Test Connection"

2. **If basic connection works, try with authentication:**
   - Add your email and password
   - Switch to port 587 with STARTTLS

3. **For Gmail specifically:**
   - Use your Gmail address as username
   - Use an App Password (not your regular password)
   - Go to Google Account → Security → App Passwords
   - Generate a password for "Mail"

4. **Debug output:**
   - Check the console/debug output for detailed SMTP communication
   - Look for response codes (220, 250, etc.)

### Common SMTP Response Codes
- **220** - Service ready (server greeting)
- **250** - Command completed successfully
- **354** - Start mail input (ready for email content)
- **421** - Service not available (server busy/closing)
- **535** - Authentication failed
- **550** - Mailbox unavailable (invalid recipient)

### Alternative Testing
If the application still fails to connect, try these external tools to verify SMTP settings:
- Telnet to SMTP server: `telnet smtp.gmail.com 25`
- Use online SMTP testing tools
- Try with Thunderbird or Outlook to verify credentials work

### Security Notes

- Use App Passwords when available (Gmail, Yahoo)
- Enable 2FA on email accounts
- Don't use personal email credentials
- Consider dedicated SMTP services for bulk sending
- Test with small batches first

## Getting Started

1. Configure SMTP settings in the Email Configuration section
2. Test connection using the "Test Connection" button
3. Add recipients manually or import from CSV/Excel
4. Create or select an email template
5. Preview your email content
6. Send to test recipient first
7. Send bulk emails to all recipients

The application now handles various SMTP configurations and provides better error messages to help diagnose connection issues.
