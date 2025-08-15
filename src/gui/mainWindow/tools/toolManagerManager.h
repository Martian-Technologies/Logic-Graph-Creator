#ifndef toolManagerManager_h
#define toolManagerManager_h

#include "gui/viewPortManager/circuitView/circuitView.h"

class ToolManagerManager {
public:
	ToolManagerManager(DataUpdateEventManager* dataUpdateEventManager);

	void addCircuitView(CircuitView* circuitView) {
		circuitViews.insert(circuitView);
	}
	void removeCircuitView(CircuitView* circuitView) {
		circuitViews.erase(circuitView);
	}

	inline void setBlock(BlockType blockType) {
		setTool("placement");
		for (auto view : circuitViews) {
			view->getToolManager().selectBlock(blockType);
		}
	}

	inline void setTool(std::string toolName) {
		std::transform(toolName.begin(), toolName.end(), toolName.begin(), ::tolower);
		auto iter = tools.find(toolName);
		if (iter == tools.end()) return;
		activeTool = toolName;
		for (auto view : circuitViews) {
			view->getToolManager().selectTool(iter->second->getInstance());
		}
		sendChangedSignal();
	}

	inline const std::string& getActiveTool() const {
		return activeTool;
	}

	inline void setMode(std::string tool) {
		for (auto view : circuitViews) {
			view->getToolManager().setMode(tool);
		}
	}

	const std::optional<std::vector<std::string>> getActiveToolModes() const {
		auto iter = tools.find(activeTool);
		if (iter == tools.end()) { return std::nullopt; }
		return iter->second->getModes();
	}

	SharedCircuitTool getToolInstance() const {
		auto iter = tools.find(activeTool);
		if (iter == tools.end()) { return nullptr; }
		return iter->second->getInstance();
	}

	const std::map<std::string, std::unique_ptr<void>>& getAllTools() const { return *reinterpret_cast<const std::map<std::string, std::unique_ptr<void>>*>(&tools); }

	template<class T>
	static void registerTool() { tools[T::getPath_()] = std::make_unique<ToolTypeMaker<T>>(); }

	/* ----------- listener ----------- */

	typedef std::function<void(const ToolManagerManager&)> ListenerFunction;

	// subject to change
	void connectListener(void* object, ListenerFunction func) { listenerFunctions[object] = func; }
	// subject to change
	void disconnectListener(void* object) { auto iter = listenerFunctions.find(object); if (iter != listenerFunctions.end()) listenerFunctions.erase(iter); }

private:
	struct BaseToolTypeMaker {
		virtual ~BaseToolTypeMaker() {}
		virtual SharedCircuitTool getInstance() const = 0;
		virtual std::vector<std::string> getModes() const = 0;
	};
	template <class T> struct ToolTypeMaker : public BaseToolTypeMaker {
		SharedCircuitTool getInstance() const override final { return std::make_shared<T>(); }
		std::vector<std::string> getModes() const override final { return T::getModes_(); }
	};

	inline void sendChangedSignal() {
		dataUpdateEventManager->sendEvent("setToolUpdate");
		for (auto pair : listenerFunctions) pair.second(*this);
	}

	std::set<CircuitView*> circuitViews;

	std::map<void*, ListenerFunction> listenerFunctions;
	std::string activeTool;

	DataUpdateEventManager* dataUpdateEventManager;

	static std::map<std::string, std::unique_ptr<BaseToolTypeMaker>> tools;
};

#endif /* toolManagerManager_h */
