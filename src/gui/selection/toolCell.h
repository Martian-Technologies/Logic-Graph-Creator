#ifndef toolCell_h
#define toolCell_h

#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QToolButton>
#include <QDropEvent>
#include <QString>

#include "gui/resourceLoaders/iconLoader.h"

class ToolCell : public QToolButton {
	Q_OBJECT
public:
	ToolCell(QWidget* parent = nullptr, int size = 48);

signals:
	// Signal emitted when a new item is accepted
	void itemAccepted(const QString& itemText);

protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	
	IconLoader iconLoader;
};

#endif /* toolCell_h */
