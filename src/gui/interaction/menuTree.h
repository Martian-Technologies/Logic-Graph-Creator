#ifndef menuTree_h
#define menuTree_h

#include <RmlUi/Core.h>

#include "util/algorithm.h"

class MenuTree {
public:
	typedef std::function<void(std::string)> ListenerFunction;
	MenuTree(Rml::ElementDocument* document, Rml::Element* parent, bool startOpen = true);
	void setListener(ListenerFunction listenerFunction) { this->listenerFunction = listenerFunction; }

	// Modifying items
	void setPaths(const std::vector<std::string>& paths);
	void setPaths(const std::vector<std::vector<std::string>>& paths) { setPaths(paths, parent); }
	
private:
	std::string getPath(Rml::Element* item);
	void setPaths(const std::vector<std::vector<std::string>>& paths, Rml::Element* current);

	ListenerFunction listenerFunction;
	Rml::ElementDocument* document;
	Rml::Element* parent;
	bool startOpen;
};

#endif /* menuTree_h */ 