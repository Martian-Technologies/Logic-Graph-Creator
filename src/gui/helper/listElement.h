#ifndef menuTree_h
#define menuTree_h

#include <RmlUi/Core.h>

class ListElement {
public:
	typedef std::function<void*(const std::string&, Rml::Element*, void*)> GeneratorFunction;
	ListElement(Rml::ElementDocument* document, Rml::Element* parent, GeneratorFunction generatorFunction);

	// Items
	void setItems(const std::vector<std::string>& items);
	void appendItem(const std::string& item);
	void insertItem(unsigned int index, const std::string& item);
	void removeIndex(unsigned int index);
	void getIndex(unsigned int index);

	// Get root element
	Rml::Element* getRoot() const { return root; }
	
private:
	GeneratorFunction generatorFunction;
	Rml::ElementDocument* document;
	Rml::Element* root;

	std::vector<std::pair<std::string, void*>> listItems;
};

#endif /* menuTree_h */ 