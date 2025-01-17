#ifndef hotbarWindow_h
#define hotbarWindow_h

#include <QDockWidget>

#include <string>

#include "backend/container/block/blockDefs.h"

class DynamicGridWidget;

namespace Ui {
	class Hotbar;
};

class HotbarWindow : public QDockWidget {
	Q_OBJECT
public:
	HotbarWindow(QWidget* parent = nullptr);
	~HotbarWindow();
	
private:
	void updateSelected();

	DynamicGridWidget* grid;
	Ui::Hotbar* ui;

signals:
	void selectedBlockChange(BlockType blockType);
	void selectedToolChange(std::string tool);
};

#endif /* hotbarWindow_h */
