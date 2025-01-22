#include <QImageReader>
#include <QDebug>

#include "iconLoader.h"

IconLoader::IconLoader() {
	int logo = loadTileset(":/toolAndBlockIcons/defaultIcon.png");
	if (logo != -1) {
		setIcon("State Changer", logo, QRect(0, 0, 256, 256));
	}

	int blocks = loadTileset(":/toolAndBlockIcons/logicTiles.png");
	if (blocks != -1) {
		setIcon("And", blocks, QRect(256 * 2, 0, 256, 256));
		setIcon("Or", blocks, QRect(256 * 3, 0, 256, 256));
		setIcon("Xor", blocks, QRect(256 * 4, 0, 256, 256));
		setIcon("Nand", blocks, QRect(256 * 5, 0, 256, 256));
		setIcon("Nor", blocks, QRect(256 * 6, 0, 256, 256));
		setIcon("Xnor", blocks, QRect(256 * 7, 0, 256, 256));
		setIcon("Button", blocks, QRect(256 * 8, 0, 256, 256));
		setIcon("Tick Button", blocks, QRect(256 * 9, 0, 256, 256));
		setIcon("Switch", blocks, QRect(256 * 10, 0, 256, 256));
		setIcon("Constant", blocks, QRect(256 * 11, 0, 256, 256));
		setIcon("Light", blocks, QRect(256 * 12, 0, 256, 256));
	}

	int tools = loadTileset(":/toolAndBlockIcons/toolIcons.png");
	if (tools != -1) {
		setIcon("Single Place", tools, QRect(0, 0, 256, 256));
		setIcon("Area Place", tools, QRect(256, 0, 256, 256));
		setIcon("Move", tools, QRect(256 * 2, 0, 256, 256));
		setIcon("Simple", tools, QRect(256, 256, 256, 256));
		setIcon("Tensor", tools, QRect(0, 256, 256, 256));

	}
}

int IconLoader::loadTileset(const std::string& path) {
	QPixmap pixmap = QPixmap(path.c_str());
	if (pixmap.isNull()) {
		qWarning() << "Failed to load icon tileset:" << path;
		return -1;
	}

	tilesets.push_back(pixmap);

	return tilesets.size() - 1;
}

void IconLoader::setIcon(const std::string& name, int index, const QRect& rect) {
	nameToIcon[name] = { index, rect };
}

QIcon IconLoader::getIcon(const std::string& key) const {
	auto iter = nameToIcon.find(key);
	if (iter == nameToIcon.end()) return QIcon();
	return QIcon(tilesets[iter->second.first].copy(iter->second.second));
}
