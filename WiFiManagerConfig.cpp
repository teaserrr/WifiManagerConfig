#include <FS.h>
#include <ArduinoJson.h>

#include "WiFiManagerConfig.h"

void saveConfigCallback() {
	WiFiManagerConfig::_instance->saveConfiguration();
}


ConfigParameter::ConfigParameter(const char *id, const char *description, const char *defaultValue, int length) {
  init(id, description, defaultValue, length, "");
}

ConfigParameter::ConfigParameter(const char *id, const char *description, const char *defaultValue, int length, const char *custom) {
  init(id, description, defaultValue, length, custom);
}

void ConfigParameter::init(const char *id, const char *description, const char *defaultValue, int length, const char *custom) {
  _id = id;
  _description = description;
  _length = length;
  _value = new char[length + 1];
  for (int i = 0; i < length + 1; i++) {
    _value[i] = 0;
  }
  if (defaultValue != NULL) {
    setValue(defaultValue);
  }

  _customHTML = custom;
}

ConfigParameter::~ConfigParameter() {
  if (_value != NULL) {
    delete[] _value;
  }
}

const char* ConfigParameter::getId() {
  return _id;
}

const char* ConfigParameter::getValue() {
  return _value;
}

WiFiManagerParameter ConfigParameter::getWifiManagerParameter() {
	return WiFiManagerParameter(_id, _description, _value, _length, _customHTML);
}

void ConfigParameter::setValue(const char *value) {
    strncpy(_value, value, _length);
}	

WiFiManagerConfig* WiFiManagerConfig::_instance;

WiFiManagerConfig::WiFiManagerConfig() {
    _max_params = WIFI_MANAGER_MAX_PARAMS;
    _params = (ConfigParameter**)malloc(_max_params * sizeof(ConfigParameter*));
  _instance = this;
}

WiFiManagerConfig::~WiFiManagerConfig()
{
    if (_params != NULL)
    {
        DEBUG_WM(F("freeing allocated params"));
        free(_params);
    }
}
const char *WiFiManagerConfig::getValue(const char *id) {
	ConfigParameter* p = getParameter(id);
	if (p == NULL)
		return "";
	return p->getValue();
}

int WiFiManagerConfig::getIntValue(const char *id) {
	return atoi(getValue(id));
}

void WiFiManagerConfig::init(WiFiManager& wifiManager) {
	initFileSystem();
  
	for (int i = 0; i < _paramsCount; i++) {
		WiFiManagerParameter p = _params[i]->getWifiManagerParameter();
		wifiManager.addParameter(&p);
	} 
	
	wifiManager.setSaveConfigCallback(&saveConfigCallback);
}

ConfigParameter* WiFiManagerConfig::getParameter(const char *id) {
	for (int i = 0; i < _paramsCount; i++) {
		if (strncmp(id, _params[i]->getId(), sizeof(id)) == 0)
			return _params[i];
	}
	return NULL;
}

void WiFiManagerConfig::initFileSystem() {
	if (SPIFFS.begin()) {
    DEBUG_WM(F("mounted file system"));
    if (SPIFFS.exists(CONFIG_FILE_PATH)) {
      //file exists, reading and loading
      DEBUG_WM(F("reading config file"));
      File configFile = SPIFFS.open(CONFIG_FILE_PATH, "r");
      if (configFile) {
        DEBUG_WM(F("opened config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument doc(size);
		DeserializationError error = deserializeJson(doc, buf.get());
        if (!error) {
			DEBUG_WM(F("parsed json"));
		  
			serializeJson(doc, Serial);

			for (int i = 0; i < _paramsCount; i++) {
				_params[i]->setValue(doc[_params[i]->getId()]);
			}
        } else {
          DEBUG_WM(F("failed to load json config"));
        }
        configFile.close();
      }
    }
  } 
  else {
    DEBUG_WM(F("failed to mount FS"));
  }
}

void WiFiManagerConfig::addParameter(const char *id, const char *description, const char *defaultValue, int length) {
	ConfigParameter* p = new ConfigParameter(id, description, defaultValue, length);
	addParameter(p);
}

void WiFiManagerConfig::addParameter(ConfigParameter *p) {
  if(_paramsCount + 1 > _max_params)
  {
    // resize the params array
    _max_params += WIFI_MANAGER_MAX_PARAMS;
    DEBUG_WM(F("Increasing _max_params to:"));
    DEBUG_WM(_max_params);
    ConfigParameter** new_params = (ConfigParameter**)realloc(_params, _max_params * sizeof(ConfigParameter*));
    if (new_params != NULL) {
      _params = new_params;
    } else {
      DEBUG_WM(F("ERROR: failed to realloc params, size not increased!"));
      return;
    }
  }
  
  DEBUG_WM(F("Adding parameter"));
  DEBUG_WM(p->getId());

  _params[_paramsCount] = p;
  _paramsCount++;
}

void WiFiManagerConfig::saveConfiguration() {
	DEBUG_WM(F("Saving parameters"));
	DynamicJsonDocument doc(_paramsCount * 256);
	
	for (int i = 0; i < _paramsCount; i++) {
		doc[_params[i]->getId()] = _params[i]->getValue();
	}  
	
    File configFile = SPIFFS.open(CONFIG_FILE_PATH, "w");
    if (!configFile) {
      DEBUG_WM(F("failed to open config file for writing"));
    }

    serializeJson(doc, Serial);
	serializeJson(doc, configFile);
    configFile.close();
}

template <typename Generic>
void WiFiManagerConfig::DEBUG_WM(Generic text) {
  if (_debug) {
    Serial.print("*WMC: ");
    Serial.println(text);
  }
}
