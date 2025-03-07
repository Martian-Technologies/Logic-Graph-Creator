#ifndef toolManagerManager_h
#define toolManagerManager_h

#include "gui/circuitView/circuitView.h"

class ToolManagerManager {
public:
	ToolManagerManager(std::set<CircuitView*>* circuitViews) : circuitViews(circuitViews) {}

	inline const std::string& getActiveToo() const { return activeTool; }
	
	inline void setBlock(BlockType blockType) {
		setTool("placement/placement");
		for (auto view : *circuitViews) {
			view->setSelectedBlock(blockType);
		}
	}
	
	inline void setTool(std::string toolName) {
		std::transform(toolName.begin(), toolName.end(), toolName.begin(), ::tolower);
		if (activeTool == toolName) return;
		activeTool = toolName;
		for (auto view : *circuitViews) {
			view->setSelectedTool(toolName);
		}
		sendChangedSignal();
	}
	
	inline void setMode(std::string tool) {
		for (auto view : *circuitViews) {
			view->setSelectedToolMode(tool);
		}
	}

	const std::vector<std::string>* getActiveToolModes() const {
		auto iter = modes.find(activeTool);
		if (iter == modes.end()) { return nullptr; }
		return &(iter->second);
	}	

	static void registerToolModes(std::string toolName, const std::vector<std::string>& toolModes) {
		modes[toolName] = toolModes;
	}

	/* ----------- listener ----------- */

	typedef std::function<void(const ToolManagerManager&)> ListenerFunction;

	// subject to change
	void connectListener(void* object, ListenerFunction func) { listenerFunctions[object] = func; }
	// subject to change
	void disconnectListener(void* object) { auto iter = listenerFunctions.find(object); if (iter != listenerFunctions.end()) listenerFunctions.erase(iter); }

private:
	inline void sendChangedSignal() {
		for (auto pair : listenerFunctions) pair.second(*this);
	}

	std::set<CircuitView*>* circuitViews;

	std::map<void*, ListenerFunction> listenerFunctions;
	std::string activeTool;

	static std::map<std::string, std::vector<std::string>> modes;
};

#endif /* toolManagerManager_h */
