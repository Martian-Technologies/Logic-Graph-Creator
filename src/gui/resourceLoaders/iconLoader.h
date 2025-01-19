#ifndef tilesetLoader_h
#define tilesetLoader_h

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QMap>

class TilesetLoader : public QObject {
	Q_OBJECT

public:
	explicit TilesetLoader(QObject* parent = nullptr);

	bool loadTileset(const QString& filePath, int tileWidth, int tileHeight);
	QPixmap getTile(const QString& key) const;
	void mapTileToKey(const QString& key, int row, int col);

private:
	QPixmap tilesetImage;
	QMap<QString, QPixmap> tileMap;
	int tileWidth;
	int tileHeight;
};

#endif /* tilesetLoader_h */
