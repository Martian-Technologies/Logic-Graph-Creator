#include "MenuTree.h"

#include <RmlUi/Core.h>

MenuTree::MenuTree(Rml::ElementDocument* document, Rml::Element* parent): document(document), parent(parent) {
  Rml::ElementPtr rootList = document->CreateElement("ul");
  rootList->SetClass("menutree", true);
  parent->AppendChild(std::move(rootList));
}

Rml::Element* MenuTree::addPath(std::vector<std::string> path) {
  // walk down the ul/li structure of the menutree using our path
  Rml::Element* current = parent;
  for(int i = 0; i < path.size(); i++) {
    //identify the next list to search through, or if it's not there, make one
    if(!current->HasChildNodes()) {
      Rml::ElementPtr newList = document->CreateElement("ul");
      //all ul's besides the root one are labeled for sublist collapse capability
      newList->SetClass("collapsed", true);
      logInfo("labeled sublist of node as collapseable: " + path[i]);
      current->AppendChild(std::move(newList));
    }
    current = current->GetFirstChild();

    //try to find the next element in the path
    bool foundPath = false;
    for(int c = 0; c < current->GetNumChildren(); c++) {
      if(current->GetChild(c)->GetId() == path[i] + "-menu") {
        current = current->GetChild(c);
        foundPath = true;
        break;
      }
    }
    //if the next element in the path doesn't exist:
    //1. create a new li with the name/id matching missing path item
    //2. append child: current->li, then set current to li
    if(!foundPath) {
      Rml::ElementPtr newItem = document->CreateElement("li");
      newItem->SetId(path[i] + "-menu");
      newItem->SetInnerRML(path[i]);
      //classes must be assigned to elements before they are appended to the DOM
      //all parents besides the root are labeled for sublist collapse capability
      if(i < path.size() - 1) { 
        newItem->SetClass("parent", true);
        logInfo("labeled node as parent: " + path[i]);
      }
      current->AppendChild(std::move(newItem));
      current = current->GetElementById(path[i] + "-menu");
    }
  }
  logInfo("added menu path " + path[path.size() - 1]);
  return current;
}