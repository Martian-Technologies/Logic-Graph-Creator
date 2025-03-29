#ifndef toolManagerManager_h
#define toolManagerManager_h

#include "backend/circuitView/circuitView.h"

class ToolManagerManager {
public:
	ToolManagerManager(std::set<CircuitView*>* circuitViews) : circuitViews(circuitViews) { }

	inline void setBlock(BlockType blockType) {
		setTool("placement/placement");
		for (auto view : *circuitViews) {
			view->getToolManager().selectBlock(blockType);
		}
	}

	inline void setTool(std::string toolName) {
		std::transform(toolName.begin(), toolName.end(), toolName.begin(), ::tolower);
		auto iter = tools.find(toolName);
		if (iter == tools.end()) return;
		activeTool = toolName;
		for (auto view : *circuitViews) {
			view->getToolManager().selectTool(iter->second.getInstance());
		}
		sendChangedSignal();
	}

	inline void setMode(std::string tool) {
		for (auto view : *circuitViews) {
			view->getToolManager().setMode(tool);
		}
	}

	const std::optional<std::vector<std::string>> getActiveToolModes() const {
		auto iter = tools.find(activeTool);
		if (iter == tools.end()) { return std::nullopt; }
		return iter->second.getModes();
	}

	SharedCircuitTool getToolInstance() const {
		auto iter = tools.find(activeTool);
		if (iter == tools.end()) { return nullptr; }
		return iter->second.getInstance();
	}

	template<class T>
	static void registerTool() { tools[T::getPath_()] = ToolTypeMaker<T>(); }

	/* ----------- listener ----------- */

	typedef std::function<void(const ToolManagerManager&)> ListenerFunction;

	// subject to change
	void connectListener(void* object, ListenerFunction func) { listenerFunctions[object] = func; }
	// subject to change
	void disconnectListener(void* object) { auto iter = listenerFunctions.find(object); if (iter != listenerFunctions.end()) listenerFunctions.erase(iter); }

private:
	struct BaseToolTypeMaker {
		virtual SharedCircuitTool getInstance() const;
		virtual std::vector<std::string> getModes() const;
	};
	template <class T> struct ToolTypeMaker : public BaseToolTypeMaker {
		SharedCircuitTool getInstance() const override final { return std::make_shared<T>(); }
		std::vector<std::string> getModes() const override final { return T::getModes_(); }
	};

	inline void sendChangedSignal() {
		for (auto pair : listenerFunctions) pair.second(*this);
	}

	std::set<CircuitView*>* circuitViews;

	std::map<void*, ListenerFunction> listenerFunctions;
	std::string activeTool;

	static std::map<std::string, BaseToolTypeMaker> tools;
};

#endif /* toolManagerManager_h */
