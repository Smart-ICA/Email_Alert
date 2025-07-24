#include <nlohmann/json.hpp>
#include <pugg/Kernel.h>
#include <sink.hpp>
#include <rang.hpp>
#include <iostream>
#include <cstdlib> 
#include <unistd.h>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;
using json = nlohmann::json;
using namespace rang;

// Définir le nom du plugin
#ifndef PLUGIN_NAME
#define PLUGIN_NAME "email_alert"
#endif

// Classe du plugin
class EmailAlertPlugin : public Sink<json> {

public:

  EmailAlertPlugin() {
    cout << "Constructeur du plugin EmailAlert appelé." << endl;
  }
  
  string kind() override {
    cout << "La méthode kind() a été appelée." << endl;
    return PLUGIN_NAME;
  }


  void set_params(void const *params) override {
    Sink::set_params(params);  // Appel à la méthode de la classe parente

    // Fusion des paramètres passés avec les paramètres par défaut
    _params.merge_patch(*(json *)params);

    // Initialisation des paramètres à partir du fichier de configuration
    if (_params.contains("to_email")) {
        _to_email = _params["to_email"].get<string>();
    } else {
        _to_email = "mail.defaut@domaine.fr";
    }

    if (_params.contains("temperature_threshold")) {
        _temperature_threshold = _params["temperature_threshold"].get<float>();
    } else {
        _temperature_threshold = 30.0;  // Valeur par défaut pour le seuil de température
    }
    
    if (_params.contains("python_path")) {
        python_path = _params["python_path"].get<string>();
    } else {
        python_path = "/home/mads/Devel/questionnaire_gform/Email_Alert/src/venv/bin/python3";
    }
    
    if (_params.contains("script_path")) {
        script_path = _params["script_path"].get<string>();
    } else {
        script_path = "/home/mads/Devel/questionnaire_gform/Email_Alert/src/email_alert.py";
    }

    cout << "Paramètres du plugin chargés : " << endl;
    cout << "Chemin python : " << python_path << endl;
    cout << "Chemin du fichier : " << script_path << endl;
    cout << "Alerte email à : " << _to_email << endl;
    cout << "Seuil de température : " << _temperature_threshold << "°C" << endl;
  }
  
  
  return_type load_data(json const &input, string topic = "") override {
    cout << "Début de la fonction load_data" << endl;
    
    if (topic.empty()) {
        cerr << "Erreur : Topic vide." << endl;
        return return_type::error;
    }
    
    cout << "Topic reçu : " << topic << endl;

    if (topic == "source_gsheet") {
        cout << "Topic 'source_gsheet' trouvé, traitement des données." << endl;
        
        // Vérification des données pour chaque critère
        process_data(input);

    } else {
        cout << "Topic inconnu, aucun traitement." << endl;
    }

    return return_type::success;
  }

  // Méthode de traitement des données et envoi d'alertes par email
  void process_data(const json& input) {
  
    string subject = "Alerte(s) Critique(s) détectée(s)";
    string body = "";
    
    // Vérification de la référence de la machine
    string machine_ref = "Référence inconnue";
    if (input.contains("Référence machine")) {
        machine_ref = input["Référence machine"].get<string>();
    }

    // Ajouter la référence de la machine dans l'objet et le corps du message
    subject = "Alerte Critique pour la machine : " + machine_ref;

    body += "Bonjour,\n\n";
    body += "Une alerte critique a été générée pour la machine " + machine_ref + ".\n\n";

    bool alert_detected = false;
    
    // Vérification du seuil de température
    if (input.contains("Température bloc froid (en °C)") && input["Température bloc froid (en °C)"].is_number_float()) {
        float temperature = input["Température bloc froid (en °C)"].get<float>();
        
        // Limiter la température à 2 décimales
        std::ostringstream temperature_stream;
        temperature_stream << std::fixed << std::setprecision(2) << temperature;
        string temperature_str = temperature_stream.str();
        
        if (temperature > _temperature_threshold) {
            cout << "ALERTE : Température élevée : " << temperature << endl;
            body += "Alerte : Température élevée du bloc froid : " 
                    + to_string(temperature) + "°C\n";
            alert_detected = true;
        }
    }

    // Vérification du niveau d'huile
    if (input.contains("Niveau d'huile") && input["Niveau d'huile"].get<string>() == "Bas") {
        cout << "ALERTE : Niveau d'huile bas détecté." << endl;
        body += "Alerte : Niveau d'huile bas.\n";
        alert_detected = true;
    }

    // Vérification du niveau lubrifiant
    if (input.contains("Niveau lubrifiant") && input["Niveau lubrifiant"].get<string>() == "Bas") {
        cout << "ALERTE : Niveau du lubrifiant bas détecté." << endl;
        body += "Alerte : Niveau lubrifiant bas.\n";
        alert_detected = true;
    }

    // Vérification du remplissage bac copeaux
    if (input.contains("Remplissage bac copeaux") && input["Remplissage bac copeaux"].get<string>() == "Haut") {
        cout << "ALERTE : Bac à copeaux rempli." << endl;
        body += "Alerte : Bac à copeaux rempli.\n";
        alert_detected = true;
    }
    
    // Vérification du tiroir haut de la desserte
    if (input.contains("Tiroir haut desserte") && input["Tiroir haut desserte"].get<string>() == "Manquant") {
        cout << "ALERTE : Tiroir haut de la desserte incomplet." << endl;
        body += "Alerte : Tiroir haut de la desserte incomplet.\n";
        alert_detected = true;
    }
    
    // Si des alertes ont été détectées, envoie un email avec toutes les alertes
    if (alert_detected) {
        send_email(subject, body, _to_email);
        cout << "Mail envoyé :\n" << body << endl;
    } else {
        cout << "Aucune alerte détectée, aucun mail envoyé." << endl;
    }
  }
  
  // Méthode pour envoyer un email
  void send_email(const string& subject, const string& body, const string& to_email) {
    cout << "Envoi d'un email : " << subject << endl;

    string command = python_path + " " + script_path + " \"" + subject + "\" \"" + body + "\" \"" + to_email + "\"";

    
    
    // Exécution du script Python pour envoyer l'email
    int ret = system(command.c_str());
    if (ret != 0) {
        cerr << "Erreur lors de l'exécution du script Python, code retour : " << ret << endl;
    } else {
        cout << "Commande Python exécutée" << endl;
    }
  }

  // Implémentation de la méthode info()
  map<string, string> info() override {
    return {
      {"name", "EmailAlertPlugin"},
      {"description", "Send mail when it reads a critical data"},
    };
  }
      
  private:
  string _to_email;               // Email du destinataire
  float _temperature_threshold;   // Seuil critique de température
  string python_path;
  string script_path;
};

// Installer le plugin
INSTALL_SINK_DRIVER(EmailAlertPlugin, json)
