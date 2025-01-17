#ifndef hotbarWindow_h
#define hotbarWindow_h

#include <QDockWidget>
#include <QToolButton>

#include <vector>
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
	void updateSelected(int index, bool state);
	void selectBlock(BlockType blockType, int index);
	void selectTool(std::string tool, int index);

	int selectedBlockIndex = -1;
	int selectedToolIndex = -1;

	std::vector<std::string> values;
	std::vector<QToolButton*> buttons;
	DynamicGridWidget* grid;
	Ui::Hotbar* ui;

signals:
	void selectedBlockChange(BlockType blockType);
	void selectedToolChange(std::string tool);
};

#endif /* hotbarWindow_h */
