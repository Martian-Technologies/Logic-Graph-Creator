#include "menuTree.h"
#include "menuTreeListener.h"

MenuTree::MenuTree(Rml::ElementDocument* document, Rml::Element* parent) : document(document), parent(parent) {
	Rml::ElementPtr rootList = document->CreateElement("ul");
	rootList->SetClass("menutree", true);
	parent->AppendChild(std::move(rootList));
}

Rml::Element* MenuTree::addPath(const std::vector<std::string>& path) {
	// walk down the ul/li structure of the menutree using our path
	std::string id;
	Rml::Element* current = parent;
	for (int i = 0; i < path.size(); i++) {
		std::string item = path[i];
		item[0] = toupper(item[0]);
		id += (id.empty() ? "" : "/") + item;
		//identify the next list to search through, or if it's not there, make one
		Rml::ElementList elements;
		current->GetElementsByTagName(elements, "ul");
		if (elements.empty()) {
			Rml::ElementPtr newList = document->CreateElement("ul");
			//all ul's besides the root one are labeled for sublist collapse capability
			newList->SetClass("collapsed", false);
			current->AppendChild(std::move(newList));
			current->GetElementsByTagName(elements, "ul");
		}
		current = elements[0];

		//try to find the next element in the path
		Rml::Element* nextItem = current->GetElementById(id + "-menu");
		//if the next element in the path doesn't exist:
		//1. create a new li with the name/id matching missing path item
		//2. append child: current->li, then set current to li
		if (nextItem) {
			current = nextItem;
		} else {
			Rml::ElementPtr newItem = document->CreateElement("li");
			newItem->SetId(id + "-menu");
			newItem->SetInnerRML(item);
			//classes must be assigned to elements before they are appended to the DOM
			//all parents besides the root are labeled for sublist collapse capability
			if (i < path.size() - 1) {
				newItem->SetClass("parent", true);
				newItem->AddEventListener("click", new MenuTreeListener(&listenerFunction));
			} else {
				newItem->AddEventListener("click", new MenuTreeListener(&listenerFunction));
			}
			current->AppendChild(std::move(newItem));
			current = current->GetElementById(id + "-menu");
		}
	}
	// logInfo("added menu path " + path.back());
	return current;
}

void MenuTree::clear(const std::vector<std::string>& path) {
	Rml::Element* current = parent;
	std::string id;
	for (int i = 0; i < path.size(); i++) {
		std::string item = path[i];
		item[0] = toupper(item[0]);
		id += (id.empty() ? "" : "/") + item;
		//identify the next list to search through, or if it's not there, make one
		Rml::ElementList elements;
		current->GetElementsByTagName(elements, "ul");
		if (elements.empty()) return;
		current = elements[0];
		Rml::Element* nextItem = current->GetElementById(id + "-menu");
		if (!nextItem) return;
		current = nextItem;
	}
	Rml::ElementList elements;
	current->GetElementsByTagName(elements, "ul");
	if (elements.empty()) return;
	current = elements[0];
	Rml::Element* par = current->GetParentNode();
	par->RemoveChild(current);
}
