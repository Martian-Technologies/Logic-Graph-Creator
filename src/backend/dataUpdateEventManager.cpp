#include "dataUpdateEventManager.h"

DataUpdateEventReceiver::DataUpdateEventReceiver(DataUpdateEventManager* eventManager) : eventManager(eventManager) {
	if (eventManager) eventManager->DataUpdateeventReceivers.emplace(this);
}

DataUpdateEventReceiver::DataUpdateEventReceiver(const DataUpdateEventReceiver& other) : eventManager(other.eventManager) {
	if (eventManager) eventManager->DataUpdateeventReceivers.emplace(this); 
}

DataUpdateEventReceiver& DataUpdateEventReceiver::operator=(const DataUpdateEventReceiver& other) {
	if (this != &other) {
		eventManager = other.eventManager;
		if (eventManager) eventManager->DataUpdateeventReceivers.emplace(this);
		functions = other.functions;
	}
	return *this;
};

DataUpdateEventReceiver::~DataUpdateEventReceiver() {
	eventManager->DataUpdateeventReceivers.erase(this);
}
