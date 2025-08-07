#pragma once

#include <string>

class EmailSender
{
public:
    // Returns true if email was sent successfully
    bool sendEmail(const std::string &to, const std::string &subject, const std::string &body);
};