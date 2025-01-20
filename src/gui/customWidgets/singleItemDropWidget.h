#ifndef singleItemDropWidget_h
#define singleItemDropWidget_h

#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QDropEvent>
#include <QWidget>
#include <QString>

class SingleItemDropWidget : public QWidget {
	Q_OBJECT
public:
	explicit SingleItemDropWidget(QWidget* parent = nullptr);

signals:
	// Signal emitted when a new item is accepted
	void itemAccepted(const QString& itemText);

protected:
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	// void mousePressEvent(QMouseEvent* event) override;

};

#endif /* singleItemDropWidget_h */
