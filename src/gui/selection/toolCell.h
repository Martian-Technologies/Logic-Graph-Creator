#ifndef toolCell_h
#define toolCell_h

#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QDropEvent>
#include <QToolButton>
#include <QString>

class ToolCell : public QToolButton {
	Q_OBJECT
public:
	ToolCell(QWidget* parent = nullptr);

signals:
	// Signal emitted when a new item is accepted
	void itemAccepted(const QString& itemText);

protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;

	std::string value;
};

#endif /* toolCell_h */
