#ifndef selectorWindow_h
#define selectorWindow_h

#include <QWidget>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

#include "backend/container/block/blockDataManager.h"

namespace Ui {
	class Selector;
};

class SelectorWindow : public QWidget {
	Q_OBJECT
public:
	SelectorWindow(const BlockDataManager* blockDataManager, DataUpdateEventManager* dataUpdateEventManager, QWidget* parent = nullptr);
	~SelectorWindow();
	
	void updateToolModeOptions(const std::vector<std::string>* modes);
	void updateBlockList();
	void removeOtherThan(QTreeWidgetItem* start, const std::set<QTreeWidgetItem*>& toKeep);

private:
	void updateSelected();
	void updateSelectedMode(QListWidgetItem *current, QListWidgetItem *previous);

	const BlockDataManager* blockDataManager;
	Ui::Selector* ui;
	DataUpdateEventManager::DataUpdateEventReceiver dataUpdateEventReceiver;

signals:
	void selectedBlockChange(std::string blockPath);
	void selectedToolChange(std::string tool);
	void selectedModeChange(std::string mode);
};

#endif /* selectorWindow_h */
