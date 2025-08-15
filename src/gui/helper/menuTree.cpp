#include "menuTree.h"
#include "menuTreeListener.h"
#include "eventPasser.h"

#include "util/algorithm.h"

MenuTree::MenuTree(Rml::ElementDocument* document, Rml::Element* parent, bool clickableName, bool startOpen) : document(document), parent(parent), clickableName(clickableName), startOpen(startOpen) {
	parent->SetClass("menutree", true);
}

void MenuTree::setPaths(const std::vector<std::string>& paths) {
	std::vector<std::vector<std::string>> vecPaths(paths.size());
	for (unsigned int i = 0; i < vecPaths.size(); i++) {
		stringSplitInto(paths[i], '/', vecPaths[i]);
	}
	setPaths(vecPaths, parent);
}


void MenuTree::setPaths(const std::vector<std::vector<std::string>>& paths, Rml::Element* current) {
	Rml::ElementList elements;
	current->GetElementsByTagName(elements, "div");
	if (elements.empty()) {
		Rml::ElementPtr newDiv = document->CreateElement("div");
		elements.push_back(current->AppendChild(std::move(newDiv)));
	}
	Rml::Element* div = elements[0];

	if (paths.empty()) {
		// remove class
		current->SetClass("parent", false);
		// remove ul
		elements.clear();
		div->GetElementsByTagName(elements, "ul");
		if (!elements.empty()) div->RemoveChild(elements[0]);
		// remove span
		elements.clear();
		current->GetElementsByTagName(elements, "span");
		if (!elements.empty()) current->RemoveChild(elements[0]);
		return;
	}
	std::map<std::string, std::vector<std::vector<std::string>>> pathsByRoot;
	for (const auto& path : paths) {
		if (path.size() == 1) {
			pathsByRoot[path[0]];
		} else {
			auto& pathVec = pathsByRoot[path[0]].emplace_back(path.size() - 1);
			for (unsigned int i = 1; i < path.size(); i++) {
				pathVec[i - 1] = path[i];
			}
		}
	}

	Rml::Element* listElement;
	elements.clear();
	div->GetElementsByTagName(elements, "ul");
	if (elements.empty()) {
		Rml::ElementPtr newList = document->CreateElement("ul");
		current->SetClass("collapsed", !startOpen && current != parent);
		listElement = div->AppendChild(std::move(newList));
		current->SetClass("parent", true);
		if (current != parent) {
			Rml::ElementPtr arrow = document->CreateElement("span");
			arrow->SetInnerRML(">");
			arrow->AddEventListener("click", new MenuTreeListener());
			current->InsertBefore(std::move(arrow), div);
		}
	} else {
		listElement = elements[0];
		std::vector<Rml::Element*> children;
		for (unsigned int i = 0; i < listElement->GetNumChildren(); i++) {
			children.push_back(listElement->GetChild(i));
		}
		for (auto element : children) {
			getPath(element);
			const std::string& id = element->GetId();
			unsigned int index = id.size() - 5;
			while (index != 0 && id[index] != '/') index--;
			auto iter = pathsByRoot.find(id.substr(index + (index != 0), id.size() - 5 - index - (index != 0)));
			if (iter == pathsByRoot.end()) {
				listElement->RemoveChild(element);
				continue;
			}
			setPaths(iter->second, element);
			pathsByRoot.erase(iter);
		}
	}
	std::string pathStr = getPath(current);
	if (!(pathStr.empty())) pathStr += "/";
	for (auto iter : pathsByRoot) {
		Rml::ElementPtr newItem = document->CreateElement("li");
		// add listener
		if (!clickableName) newItem->AddEventListener("click", new MenuTreeListener(false, &listenerFunction));
		else {
			newItem->AddEventListener("click", new EventPasser(
				[this](Rml::Event& event) {
					event.StopPropagation();
					Rml::Element* target = event.GetCurrentElement();
					if (listenerFunction)
						listenerFunction(target->GetId().substr(0, target->GetId().size() - 5));
				}
			));
		}
		// set id
		newItem->SetId(pathStr + iter.first + "-menu");
		// create div for text
		Rml::ElementPtr newDiv = document->CreateElement("div");
		newDiv->AppendChild(std::move(document->CreateTextNode(iter.first)));
		newItem->AppendChild(std::move(newDiv));
		Rml::Element* newItem2 = listElement->AppendChild(std::move(newItem));
		if (!iter.second.empty()) {
			setPaths(iter.second, newItem2);
		}
	}
}

std::string MenuTree::getPath(Rml::Element* item) {
	if (item == parent) return "";
	return item->GetId().substr(0, item->GetId().size() - 5);
}