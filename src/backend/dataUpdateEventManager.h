#ifndef dataUpdateEventManager_h
#define dataUpdateEventManager_h

class DataUpdateEventManager {
	friend class DataUpdateEventReceiver;
public:
	class EventData { public: virtual ~EventData() { }; };
	template <class T>
	class EventDataWithValue : public EventData {
	public:
		EventDataWithValue(const T& value) : value(value) { }
		inline const T& operator*() const { return value; }
		inline const T* operator->() const { return &value; }
		inline const T& get() const { return value; }
	private:
		T value;
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
		for (DataUpdateEventReceiver* dataUpdateEventReceiver : dataUpdateEventReceivers) {
			for (auto pair : dataUpdateEventReceiver->functions) {
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
