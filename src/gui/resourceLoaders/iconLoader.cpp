#include <QImageReader>
#include <QDebug>

#include "iconLoader.h"

TilesetLoader::TilesetLoader(QObject* parent)
	: QObject(parent), tileWidth(0), tileHeight(0) { }

bool TilesetLoader::loadTileset(const QString& filePath, int tileWidth, int tileHeight) {
	QImageReader reader(filePath);
	if (!reader.canRead()) {
		qWarning() << "Failed to load tileset image:" << filePath;
		return false;
	}

	this->tilesetImage = QPixmap::fromImage(reader.read());
	if (this->tilesetImage.isNull()) {
		qWarning() << "Failed to load tileset image as QPixmap:" << filePath;
		return false;
	}

	this->tileWidth = tileWidth;
	this->tileHeight = tileHeight;

	return true;
}

void TilesetLoader::mapTileToKey(const QString& key, int row, int col) {
	if (this->tilesetImage.isNull() || tileWidth <= 0 || tileHeight <= 0) {
		qWarning() << "Tileset image not loaded or invalid dimensions.";
		return;
	}

	int x = col * tileWidth;
	int y = row * tileHeight;

	if (x + tileWidth > tilesetImage.width() || y + tileHeight > tilesetImage.height()) {
		qWarning() << "Tile coordinates out of bounds for key:" << key;
		return;
	}

	QPixmap tile = tilesetImage.copy(x, y, tileWidth, tileHeight);
	tileMap.insert(key, tile);
}

QPixmap TilesetLoader::getTile(const QString& key) const {
	return tileMap.value(key, QPixmap());
}
