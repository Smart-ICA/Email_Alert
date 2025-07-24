# Email Alert MADS Plugin

This plugin listens to a specific MQTT topic and **sends email alerts** when critical values (e.g., temperature, oil level) exceed defined thresholds. The plugin uses the **Gmail API** for sending emails and OAuth2 authentication for secure access.

## Features

* Monitors data published to a MADS MQTT topic.
* Sends email alerts for critical conditions.
* Configurable alert thresholds and email recipients.
* Supports OAuth2 authentication to send emails securely using the Gmail API.
* Allows easy configuration via the mads.ini file.

## Requirements

* MADS platform installed and configured.

* Python3 and required libraries for Gmail API and authentication.

* A Google account and a project created on Google Cloud Console for enabling the Gmail API.

## Installation

1. Clone the plugin

Place the plugin folder inside a email_alert directory.

2. Build and install

In your terminal, navigate to the plugin directory and run:

```bash
cmake -Bbuild -DCMAKE_INSTALL_PREFIX="$(mads -p)"
cmake --build build -j4
sudo cmake --install build
```

## MADS Configuration in the INI settings

The plugin supports the following settings in the mads.ini file:

```ini
[email_alert]
sub_topic = ["topic"] 
python_path = "/path/to/venv/bin/python3"
script_path = "/path/to/email_alert.py"
to_email = "recipient1@example.com, recipient2@example.com"
temperature_threshold = 30
```

**sub_topic** : MQTT Topic listens by the plugin

**python_path** : Path to the Python executable inside the virtual environment.

**script_path** : Path to the Python script used to send the emails via the Gmail API.

**to_email** : Comma-separated list of email addresses to receive the alerts.

**temperature_threshold** : Threshold for the temperature to trigger an alert.

## Google API Authentication

The plugin uses OAuth2 for Gmail authentication. To authenticate, you need to set up a project in the Google Cloud Console and enable the Gmail API.

1. Create a Google Cloud Project

* Go to [Google Cloud Console](https://console.cloud.google.com).
* Create a new project (if needed).
* Enable the Gmail API.
* Create OAuth2 credentials for the application :

        Select "OAuth client ID".
        Configure the OAuth consent screen.
        Download the credentials.json file.

2. Save the OAuth Credentials

Place the downloaded credentials.json in the same directory as email_alert.py.

3. First Execution to Authenticate

When running the script for the first time, you will be prompted to allow access to your Gmail account via OAuth2 authentication. Follow the on-screen instructions to retrieve the authorization code.

Once authenticated, the access token will be saved to token.json, which will be used for subsequent requests.

## Create a Python Virtual Environment

To isolate Python dependencies, we recommend creating a virtual environment. Here’s how to do it:

1. Create the virtual environment in your plugin directory:

```bash
python3 -m venv venv
```
2. Activate the virtual environment:

```bash
source venv/bin/activate  # On Linux/macOS
# OR
venv\Scripts\activate  # On Windows
```

3. Install the required Python libraries:

```bash
pip install --upgrade google-api-python-client google-auth-httplib2 google-auth-oauthlib
```

Once installed, the environment will be ready to send emails via Gmail API.
Don't forget to set up python3 path in mads.ini.

##Run the Plugin

Once everything is configured and compiled, you can run the plugin:

```bash
mads sink email_alert.plugin
```

This will start the plugin, which will listen to the topic and send email alerts if critical thresholds are exceeded.

Authors

Main author:
Léandre Reynard (Université Savoie Mont-Blanc)

Contributors:
Guillaume Cohen (Université de Toulouse) &
Anna-Carla Araujo (INSA Toulouse)
