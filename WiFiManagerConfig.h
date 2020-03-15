#ifndef WiFiManagerConfig_h
#define WiFiManagerConfig_h

#include <WiFiManager.h>

#ifndef CONFIG_FILE_PATH
#define CONFIG_FILE_PATH "/config.json"
#endif

class ConfigParameter {
  public:
	ConfigParameter(const char *id, const char *description, const char *defaultValue, int length);
	ConfigParameter(const char *id, const char *description, const char *defaultValue, int length, const char *custom);
	~ConfigParameter();

	const char *getId();
	const char *getValue();
	WiFiManagerParameter* getWifiManagerParameter();
	
	void setValue(const char *value);		
	void updateValueFromWifiManager();
	
  private:
	const char *_id;
	const char *_description;
	char       *_value;
	int         _length;
	const char *_customHTML;

	WiFiManagerParameter* _wmParam = NULL;
	
	void init(const char *id, const char *description, const char *defaultValue, int length, const char *custom);
	void initFileSystem();
};
	
class WiFiManagerConfig
{
  public:
    WiFiManagerConfig();
    ~WiFiManagerConfig();
	
	void init(WiFiManager& wifiManager);
	void addParameter(const char *id, const char *description, const char *defaultValue, int length);
	void updateValuesFromWifiManager();
	void saveConfiguration();
	void setValue(const char *id, const char *value);
	void setValue(const char *id, int value);
	
    const char *getValue(const char *id);
    int getIntValue(const char *id);

	static WiFiManagerConfig* _instance;
	
  private:	
    ConfigParameter** _params;
    int _paramsCount;
    int _max_params;
	
	void initFileSystem();
	void addParameter(ConfigParameter *p);
	ConfigParameter* getParameter(const char *id);	
	
    boolean _debug = true;
	
    template <typename Generic>
    void DEBUG_WM(Generic text);
	
};

#endif
