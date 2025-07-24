import sys
import smtplib
import os
import re
import pickle
import google.auth
from google_auth_oauthlib.flow import InstalledAppFlow
from googleapiclient.discovery import build
from google.auth.transport.requests import Request
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import base64

# Scopes nécessaires pour l'API Gmail (envoi d'emails)
SCOPES = ['https://www.googleapis.com/auth/gmail.send']

# Fichier de token où les informations OAuth 2.0 seront stockées après la première authentification
TOKEN_PICKLE = 'token.pickle'

def authenticate_gmail_api():
    """Effectue l'authentification OAuth et renvoie les credentials pour l'API Gmail"""
    creds = None

    # Si un token a déjà été obtenu et est encore valide, le charger
    if os.path.exists(TOKEN_PICKLE):
        with open(TOKEN_PICKLE, 'rb') as token:
            creds = pickle.load(token)

    # Si aucun token n'existe ou s'il est expiré, il faut s'authentifier de nouveau
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            # Demander l'autorisation à l'utilisateur pour accéder à l'API Gmail
            flow = InstalledAppFlow.from_client_secrets_file(
                '/home/mads/Devel/questionnaire_gform/Email_Alert/src/credentials.json', SCOPES)
            creds = flow.run_local_server(port=0)  # Ceci ouvre une fenêtre de navigateur pour l'authentification

        # Sauvegarder les credentials pour les futures utilisations
        with open(TOKEN_PICKLE, 'wb') as token:
            pickle.dump(creds, token)

    return creds

def send_email(subject, body, to_email):
    """Envoie un email via l'API Gmail avec les credentials OAuth"""
    creds = authenticate_gmail_api()
    service = build('gmail', 'v1', credentials=creds)
    
    if "Température" in body:
        # Utilisation d'une expression régulière pour extraire la température numérique
        temp_match = re.search(r"(\d+\.\d+)", body)  # Cherche un nombre flottant
        if temp_match:
            temperature_str = temp_match.group(0)  # Extraire la température trouvée
            try:
                # Convertir la température en float après nettoyage et arrondir à 2 décimales
                temperature = float(temperature_str)
                temperature = round(temperature, 2)
                # Remplacer la température dans le corps du message
                body = body.replace(temperature_str, f"{temperature:.2f}")
            except ValueError:
                print("Erreur de conversion de la température.")
    
    message = MIMEMultipart()
    message['to'] = to_email
    message['subject'] = subject
    msg = MIMEText(body)
    message.attach(msg)
    raw_message = base64.urlsafe_b64encode(message.as_bytes()).decode()

    try:
        message = service.users().messages().send(userId="me", body={'raw': raw_message}).execute()
        print(f'Message Id: {message["id"]}')
    except Exception as error:
        print(f'An error occurred: {error}')

if __name__ == "__main__":
    # Récupère les arguments passés depuis C++
    subject = sys.argv[1]
    body = sys.argv[2]
    to_email = sys.argv[3]
    
    # Appelle la fonction pour envoyer l'email
    send_email(subject, body, to_email)
    
    
# Exemple d'appel
#send_email(
#    subject="Alerte Critique : Valeur anormale détectée",
#    body="Une valeur critique a été détectée. Détails : ...",
#    to_email="leandre.reynard@etu.univ-smb.fr"
#)
