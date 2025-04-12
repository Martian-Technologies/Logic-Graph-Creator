#ifndef menuTree_h
#define menuTree_h

#include <RmlUi/Core.h>

class MenuTree {
public:
	typedef std::function<void(std::string)> ListenerFunction;
	MenuTree(Rml::ElementDocument* document, Rml::Element* parent);
	Rml::Element* addPath(const std::vector<std::string>& path);
	void clear(const std::vector<std::string>& path = {});
	void setListener(ListenerFunction listenerFunction) { this->listenerFunction = listenerFunction; }

private:
	ListenerFunction listenerFunction;
	Rml::ElementDocument* document;
	Rml::Element* parent;
};

#endif /* menuTree_h */ 