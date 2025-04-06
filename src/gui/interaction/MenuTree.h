#ifndef MENUTREE_H
#define MENUTREE_H

#include <RmlUi/Core.h>

class MenuTree {
public:
  MenuTree(Rml::ElementDocument* document, Rml::Element* parent);
  Rml::Element* addPath(std::vector<std::string>);

private:
  Rml::ElementDocument* document;
  Rml::Element* parent;
};

#endif /* MENUTREE_H */ 