#ifndef dataUpdateEventManager_h
#define dataUpdateEventManager_h

class DataUpdateEventManager {
	friend class DataUpdateEventReceiver;
public:
	template <class T>
	class EventDataWithValue;
	class EventData {
	public:
		virtual ~EventData() { }
		template <class V>
		inline const DataUpdateEventManager::EventDataWithValue<V>* cast() const;
	};
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
		DataUpdateEventReceiver(DataUpdateEventManager* eventManager);
		DataUpdateEventReceiver(const DataUpdateEventReceiver& other);
		DataUpdateEventReceiver(DataUpdateEventReceiver&& other);
		DataUpdateEventReceiver& operator=(const DataUpdateEventReceiver& other);
		~DataUpdateEventReceiver();

		void linkFunction(const std::string& eventName, std::function<void(const EventData*)> function) { functions[eventName] = function; }

	private:
		std::map<std::string, std::function<void(const EventData*)>> functions;
		DataUpdateEventManager* eventManager = nullptr;
	};

	~DataUpdateEventManager();

	void sendEvent(const std::string& eventName) {
		for (DataUpdateEventReceiver* dataUpdateEventReceiver : dataUpdateEventReceivers) {
			for (auto pair : dataUpdateEventReceiver->functions) {
				if (pair.first == eventName) pair.second(nullptr);
			}
		}
	}

	template <class V>
	void sendEvent(const std::string& eventName, const V& value) {
		DataUpdateEventManager::EventDataWithValue<V> eventDataWithValue(value);
		for (DataUpdateEventReceiver* DataUpdateeventReceiver : dataUpdateEventReceivers) {
			for (auto pair : DataUpdateeventReceiver->functions) {
				if (pair.first == eventName) pair.second((EventData*)&eventDataWithValue);
			}
		}
	}

private:
	std::set<DataUpdateEventReceiver*> dataUpdateEventReceivers;

};

template <class V>
inline const DataUpdateEventManager::EventDataWithValue<V>* DataUpdateEventManager::EventData::cast() const { return dynamic_cast<const EventDataWithValue<V>*>(this); }


#endif /* dataUpdateEventManager_h */
