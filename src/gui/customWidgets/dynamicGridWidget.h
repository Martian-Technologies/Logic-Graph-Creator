#ifndef dynamicGridWidget_h
#define dynamicGridWidget_h

#include <QGridLayout>
#include <QWidget>

#include <vector>

class DynamicGridWidget : public QWidget {
	Q_OBJECT
public:
	explicit DynamicGridWidget(QWidget* parent = nullptr, int minSize = 100);

	// Function to add a widget to the grid
	void addWidget(QWidget* widget);

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	int minSize;
	QGridLayout* gridLayout;
	std::vector<QWidget*> childWidgets; // List of added widgets

	void updateGrid();
};


#endif /* dynamicGridWidget_h */
