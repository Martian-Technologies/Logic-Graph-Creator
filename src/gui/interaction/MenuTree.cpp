#include "menuTree.h"

#include <RmlUi/Core.h>

MenuTree::MenuTree(Rml::ElementDocument* document, Rml::Element* parent) : document(document), parent(parent) {
	Rml::ElementPtr rootList = document->CreateElement("ul");
	rootList->SetClass("menutree", true);
	parent->AppendChild(std::move(rootList));
}

Rml::Element* MenuTree::addPath(const std::vector<std::string>& path) {
	// walk down the ul/li structure of the menutree using our path
	Rml::Element* current = parent;
	for (int i = 0; i < path.size(); i++) {
		const std::string item = path[i];
		//identify the next list to search through, or if it's not there, make one
		if (!current->HasChildNodes()) {
			Rml::ElementPtr newList = document->CreateElement("ul");
			//all ul's besides the root one are labeled for sublist collapse capability
			newList->SetClass("collapsed", true);
			logInfo("labeled sublist of node as collapseable: " + item);
			current->AppendChild(std::move(newList));
		}
		current = current->GetFirstChild();

		//try to find the next element in the path
		Rml::Element* nextItem = current->GetElementById(item + "-menu");
		//if the next element in the path doesn't exist:
		//1. create a new li with the name/id matching missing path item
		//2. append child: current->li, then set current to li
		if (nextItem) {
			current = nextItem;
		} else {
			Rml::ElementPtr newItem = document->CreateElement("li");
			newItem->SetId(item + "-menu");
			newItem->SetInnerRML(std::string(i*2, '-') + item);
			//classes must be assigned to elements before they are appended to the DOM
			//all parents besides the root are labeled for sublist collapse capability
			if (i < path.size() - 1) {
				newItem->SetClass("parent", true);
				logInfo("labeled node as parent: " + item);
			}
			current->AppendChild(std::move(newItem));
			current = current->GetElementById(item + "-menu");
		}
	}
	logInfo("added menu path " + path.back());
	return current;
}