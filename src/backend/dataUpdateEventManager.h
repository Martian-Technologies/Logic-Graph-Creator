#ifndef dataUpdateEventManager_h
#define dataUpdateEventManager_h

class DataUpdateEventManager {
	friend class DataUpdateEventReceiver;
public:
	class EventData { virtual void A() { }; };
	class EventDataUnsignedInt : public EventData {
	public:
		EventDataUnsignedInt(unsigned int value) : value(value) { }
		unsigned int getValue() const { return value; }
	private:
		unsigned int value;
	};

	class DataUpdateEventReceiver {
		friend class DataUpdateEventManager;
	public:
		DataUpdateEventReceiver(DataUpdateEventManager* eventManager = nullptr);
		DataUpdateEventReceiver(const DataUpdateEventReceiver& other);
		DataUpdateEventReceiver& operator=(const DataUpdateEventReceiver& other);
		~DataUpdateEventReceiver();

		void linkFunction(const std::string& eventName, std::function<void(const EventData*)> function) { functions[eventName] = function; }

	private:
		std::map<std::string, std::function<void(const EventData*)>> functions;
		DataUpdateEventManager* eventManager = nullptr;
	};

	void sendEvent(const std::string& eventName) {
		for (DataUpdateEventReceiver* DataUpdateeventReceiver : dataUpdateEventReceivers) {
			for (auto pair : DataUpdateeventReceiver->functions) {
				if (pair.first == eventName) pair.second(nullptr);
			}
		}
	}

	template <class DataType>
	void sendEvent(const std::string& eventName, const DataType& eventData) {
		for (DataUpdateEventReceiver* DataUpdateeventReceiver : dataUpdateEventReceivers) {
			for (auto pair : DataUpdateeventReceiver->functions) {
				if (pair.first == eventName) pair.second((EventData*)&eventData);
			}
		}
	}

private:
	std::set<DataUpdateEventReceiver*> dataUpdateEventReceivers;

};

#endif /* dataUpdateEventManager_h */
