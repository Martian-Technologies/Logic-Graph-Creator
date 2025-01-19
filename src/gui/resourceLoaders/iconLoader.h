#ifndef iconLoader_h
#define iconLoader_h

#include <QPixmap>
#include <QRect>
#include <QIcon>


class IconLoader : public QObject {
public:
	IconLoader();

	QIcon getIcon(const std::string& key) const;

private:
	int loadTileset(const std::string& path);
	void setIcon(const std::string& name, int index, const QRect& rect);

	std::map<std::string, std::pair<int, QRect>> nameToIcon;
	std::vector<QPixmap> tilesets;
};

#endif /* iconLoader_h */
