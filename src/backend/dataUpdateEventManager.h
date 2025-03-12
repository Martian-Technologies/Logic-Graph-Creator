#ifndef dataUpdateEventManager_h
#define dataUpdateEventManager_h

class DataUpdateEventManager;

class DataUpdateEventReceiver {
	friend class DataUpdateEventManager;
public:
	DataUpdateEventReceiver(DataUpdateEventManager* eventManager = nullptr);
	DataUpdateEventReceiver(const DataUpdateEventReceiver& other);
	DataUpdateEventReceiver& operator=(const DataUpdateEventReceiver& other);
	~DataUpdateEventReceiver();

	void linkFunction(const std::string& eventName, std::function<void()> function) { functions[eventName] = function; }

private:
	std::map<std::string, std::function<void()>> functions;
	DataUpdateEventManager* eventManager = nullptr;
};

class DataUpdateEventManager {
	friend class DataUpdateEventReceiver;
public:
	void sendEvent(const std::string& eventName) {
		for (DataUpdateEventReceiver* DataUpdateeventReceiver : DataUpdateeventReceivers) {
			for (auto pair : DataUpdateeventReceiver->functions) {
				pair.second();
			}
		}
	}

private:
	std::set<DataUpdateEventReceiver*> DataUpdateeventReceivers;

};

#endif /* dataUpdateEventManager_h */
