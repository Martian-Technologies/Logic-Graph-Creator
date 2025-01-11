#ifndef logicGridWindow_h
#define logicGridWindow_h

#include <QApplication>
#include <QTreeWidget>
#include <QVulkanInstance>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QWidget>
#include <QTimer>

#include "circuitView/circuitView.h"
#include "gpu/vulkanRenderer.h"
#include "util/vec2.h"

class CircuitViewWidget : public QWidget {
	Q_OBJECT
public:
	CircuitViewWidget(QWidget* parent = nullptr);

	// setup
	void createVulkanWindow(VulkanGraphicsView vulkanView, QVulkanInstance* qVulkanInstance);
	void setCircuit(std::shared_ptr<Circuit> circuit);
	void setEvaluator(std::shared_ptr<Evaluator> evaluator);
	void setSelector(QTreeWidget* treeWidget);

	// dont call this func (temporary)
	void updateSelectedItem();	

private:
	CircuitView<VulkanRenderer> circuitView;

	// update loop
	QTimer* updateLoopTimer;
	const float updateInterval = 0.0001f;
	void updateLoop();

	// framerate statistics
	std::list<float> pastFrameTimes;
	const int numTimesInAverage = 20;

	// ui elements
	QTreeWidget* treeWidget;

	// settings (temp)
	bool mouseControls;

	void save();
	void load(const QString& filePath);

	// utility functions
	inline Vec2 pixelsToView(QPointF point) { return Vec2((float)point.x() / (float)rect().width(), (float)point.y() / (float)rect().height()); }
	inline bool insideWindow(const QPoint& point) const { return point.x() >= 0 && point.y() >= 0 && point.x() < size().width() && point.y() < size().height(); }
	inline float getPixelsWidth() { return (float)rect().width(); }
	inline float getPixelsHeight() { return (float)rect().height(); }

protected:
	// events overrides
	void showEvent(QShowEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void enterEvent(QEnterEvent* event) override;
	void leaveEvent(QEvent* event) override;
	bool event(QEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;
};

#endif /* logicGridWindow_h */
