#ifndef menuTree_h
#define menuTree_h

#include <RmlUi/Core.h>

class MenuTree {
public:
  MenuTree(Rml::ElementDocument* document, Rml::Element* parent);
  Rml::Element* addPath(const std::vector<std::string>& path);

private:
  Rml::ElementDocument* document;
  Rml::Element* parent;
};

#endif /* menuTree_h */ 