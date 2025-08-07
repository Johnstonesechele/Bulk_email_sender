# BulkEmailApp

A simple bulk email sender application with a dark blue and gold GUI, built with C++ and Qt 6.

## Features

* Import recipients from a CSV file
* Send bulk emails via SMTP (libcurl)
* Track email send status (Sent / Failed)
* Organize campaigns with the date sent

## Build Instructions (Windows)

1. Install [Qt 6](https://www.qt.io/download) with the *MSVC* or *MinGW* kit of your choice.
2. Install [CMake](https://cmake.org/) and ensure it is on your `PATH`.
3. Install [libcurl](https://curl.se/libcurl/). You can use vcpkg or build from source.
4. Clone this repository and open a *Qt Command Prompt* (or a Developer PowerShell) with the appropriate compiler environment.

```bash
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.5.1/msvc2019_64" ..
cmake --build . --config Release
```

5. After building, copy Qt and libcurl runtime DLLs next to `BulkEmailApp.exe` or use `windeployqt` to deploy.

## SMTP Settings

Edit `EmailSender.cpp` and change:

```
smtp://smtp.example.com:587
user@example.com
password
```

to match your SMTP credentials.

## TODO

This is a minimal proof-of-concept. For production use, you should:

* Add TLS certificate validation and better error handling.
* Support HTML email and attachments.
* Store campaigns and recipients in a SQLite database.
* Handle SMTP rate limits and retries.
* Parse bounce/receipt emails to update delivery statistics.