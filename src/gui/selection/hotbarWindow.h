#ifndef hotbarWindow_h
#define hotbarWindow_h

#include <QDockWidget>
#include <QToolButton>

#include "backend/container/block/blockDefs.h"
#include "toolCell.h"

class DynamicGridWidget;

namespace Ui {
	class Hotbar;
};

class HotbarWindow : public QDockWidget {
	Q_OBJECT
public:
	HotbarWindow(QWidget* parent = nullptr);
	~HotbarWindow();
	void setItem(int index, std::string name);

private:
	void updateSelected(int index, bool state);
	void selectBlock(int index, BlockType blockType);
	void selectTool(int index, std::string tool);

	int selectedBlockIndex = -1;
	int selectedToolIndex = -1;

	std::vector<std::string> values;
	std::vector<ToolCell*> buttons;
	DynamicGridWidget* grid;
	Ui::Hotbar* ui;

signals:
	void selectedBlockChange(BlockType blockType);
	void selectedToolChange(std::string tool);
};

#endif /* hotbarWindow_h */
