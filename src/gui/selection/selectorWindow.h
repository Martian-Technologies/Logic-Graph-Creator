#ifndef selectorWindow_h
#define selectorWindow_h

#include <QWidget>
#include <QListWidgetItem>

#include "backend/container/block/blockDataManager.h"

namespace Ui {
	class Selector;
};

class SelectorWindow : public QWidget {
	Q_OBJECT
public:
	SelectorWindow(const BlockDataManager* blockDataManager, QWidget* parent = nullptr);
	~SelectorWindow();
	
	void updateToolModeOptions(const std::vector<std::string>* modes);
	void updateBlockList();

private:
	void updateSelected();
	void updateSelectedMode(QListWidgetItem *current, QListWidgetItem *previous);

	const BlockDataManager* blockDataManager;
	Ui::Selector* ui;

signals:
	void selectedBlockChange(BlockType blockType);
	void selectedToolChange(std::string tool);
	void selectedModeChange(std::string mode);
};

#endif /* selectorWindow_h */
