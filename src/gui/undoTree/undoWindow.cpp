#include "undoWindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>

UndoWindow::UndoWindow(QWidget* parent) : QDialog(parent), parent(parent) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

	setupUI();
}

void UndoWindow::setupUI() {
	setWindowTitle("Tree History");

	QHBoxLayout* mainLayout = new QHBoxLayout(this);

	// -------------------- Content Area -------------------- 

    QVBoxLayout* branchView = new QVBoxLayout();

	// -------------------- Information List -------------------- 
	QVBoxLayout* historyBar = new QVBoxLayout();
	
	QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	
	QPushButton* setHistory = new QPushButton("Set Branch", this);


	historyBar->addWidget(scrollArea);
	historyBar->addWidget(setHistory);

	// -------------------- To Screen --------------------
	mainLayout->addLayout(historyBar);
	mainLayout->addLayout(branchView);
	setLayout(mainLayout);
}

void UndoWindow::showGates() {

}
void UndoWindow::addGate() {

}
void UndoWindow::removeGate() {

}

void UndoWindow::selectBranch() {

}
void UndoWindow::nextNode() {

}
void UndoWindow::prevNode() {

}
