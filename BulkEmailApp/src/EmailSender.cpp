#include "EmailSender.h"
#include <curl/curl.h>

bool EmailSender::sendEmail(const std::string &to, const std::string &subject, const std::string &body)
{
    // NOTE: This is a minimal placeholder implementation using libcurl SMTP.
    // In production, provide proper SMTP server credentials and handle TLS.

    CURL *curl = curl_easy_init();
    if (!curl)
        return false;

    struct curl_slist *recipients = nullptr;

    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.example.com:587");
    curl_easy_setopt(curl, CURLOPT_USERNAME, "user@example.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "password");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, "<user@example.com>");
    recipients = curl_slist_append(recipients, ("<" + to + ">").c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    std::string data = "To: " + to + "\r\n";
    data += "From: Bulk Sender <user@example.com>\r\n";
    data += "Subject: " + subject + "\r\n\r\n";
    data += body;

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, [](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t {
        std::string *uploadData = static_cast<std::string*>(userdata);
        size_t bufferSize = size * nmemb;
        size_t copySize = std::min(uploadData->size(), bufferSize);
        if (copySize > 0)
        {
            memcpy(ptr, uploadData->c_str(), copySize);
            uploadData->erase(0, copySize);
        }
        return copySize;
    });

    curl_easy_setopt(curl, CURLOPT_READDATA, &data);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}