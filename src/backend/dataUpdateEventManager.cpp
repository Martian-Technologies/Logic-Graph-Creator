#include "dataUpdateEventManager.h"

DataUpdateEventManager::DataUpdateEventReceiver::DataUpdateEventReceiver(DataUpdateEventManager* eventManager) : eventManager(eventManager) {
	if (eventManager) eventManager->dataUpdateEventReceivers.emplace(this);
}

DataUpdateEventManager::DataUpdateEventReceiver::DataUpdateEventReceiver(const DataUpdateEventReceiver& other) : eventManager(other.eventManager) {
	if (eventManager) eventManager->dataUpdateEventReceivers.emplace(this); 
}

DataUpdateEventManager::DataUpdateEventReceiver& DataUpdateEventManager::DataUpdateEventReceiver::operator=(const DataUpdateEventReceiver& other) {
	if (this != &other) {
		eventManager = other.eventManager;
		if (eventManager) eventManager->dataUpdateEventReceivers.emplace(this);
		functions = other.functions;
	}
	return *this;
};

DataUpdateEventManager::DataUpdateEventReceiver::~DataUpdateEventReceiver() {
	eventManager->dataUpdateEventReceivers.erase(this);
}
