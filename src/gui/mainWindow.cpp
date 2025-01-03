#include <QPushButton>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>
#include <QShortcut>
#include <QJsonDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QDragEnterEvent>
#include <QMimeData>

#include <iostream>
#include <memory>

#include "logicGridWindow.h"
#include "ui_mainWindow.h"
#include "mainWindow.h"
#include "gpu1.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), blockContainerManager(), evaluator(nullptr) {
    ui->setupUi(this);

    setWindowTitle(tr("Logic Graph Creator"));
    setWindowIcon(QIcon(":/gateIcon.ico"));
    setAcceptDrops(true);

    id = blockContainerManager.createNewContainer();
    std::shared_ptr<BlockContainerWrapper> blockContainerWrapper = blockContainerManager.getContainer(id);

    // makeGPU1(blockContainerWrapper.get());

    evaluator = std::make_shared<Evaluator>(blockContainerWrapper);

    LogicGridWindow* logicGridWindow = new LogicGridWindow(this);
    logicGridWindow->setBlockContainer(blockContainerWrapper);
    logicGridWindow->setEvaluator(evaluator);
    logicGridWindow->setSelector(ui->selectorTreeWidget);

    connect(ui->StartSim, &QPushButton::clicked, this, &MainWindow::setSimState);
    connect(ui->UseSpeed, &QCheckBox::stateChanged, this, &MainWindow::simUseSpeed);
    connect(ui->Speed, &QDoubleSpinBox::valueChanged, this, &MainWindow::setSimSpeed);

    QVBoxLayout* layout = new QVBoxLayout(ui->gridWindow);
    layout->addWidget(logicGridWindow);

    QShortcut* saveShortcut = new QShortcut(QKeySequence("Ctrl+S"), this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::save);
}

void MainWindow::setSimState(bool state) {
    evaluator->setPause(!state);
}

void MainWindow::simUseSpeed(bool state) {
    evaluator->setUseTickrate(state);
    // evaluator->setPause(!state);
}

void MainWindow::setSimSpeed(double speed) {
    evaluator->setTickrate(std::round(speed * 60));
    // evaluator->setPause(!state);
}

void saveJsonToFile(const QJsonObject& jsonObject);

void MainWindow::save() {
    // std::cout << "save" << std::endl;
    BlockContainerWrapper* blockContainerWrapper = blockContainerManager.getContainer(id).get();
    Difference difference = blockContainerWrapper->getBlockContainer()->getCreationDifference();
    const auto modifications = difference.getModifications();
    QJsonObject modificationsJson;
    QJsonArray placeJson;
    QJsonArray connectJson;
    for (const auto& modification : modifications) {
        const auto& [modificationType, modificationData] = modification;
        switch (modificationType) {
        case Difference::PLACE_BLOCK:
        {
            QJsonObject placement;
            const auto& [position, rotation, blockType] = std::get<Difference::block_modification_t>(modificationData);
            placement["x"] = position.x;
            placement["y"] = position.y;
            placement["r"] = (char)rotation;
            placement["t"] = (char)blockType;
            placeJson.push_back(placement);
            break;
        }
        case Difference::CREATED_CONNECTION:
        {
            QJsonObject connection;
            const auto& [outputPosition, inputPosition] = std::get<Difference::connection_modification_t>(modificationData);
            connection["ox"] = outputPosition.x;
            connection["oy"] = outputPosition.y;
            connection["iy"] = inputPosition.y;
            connection["ix"] = inputPosition.x;
            connectJson.push_back(connection);
            break;
        }
        default:
            throw std::invalid_argument("save: invalid modificationType");
        }
    }
    modificationsJson["place"] = placeJson;
    modificationsJson["connect"] = connectJson;
    saveJsonToFile(modificationsJson);
}

void saveJsonToFile(const QJsonObject& jsonObject) {
    // Convert JSON object to QJsonDocument
    QJsonDocument jsonDoc(jsonObject);

    // Open a save file dialog
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save JSON File", "", "JSON Files (*.json);;All Files (*)");

    if (fileName.isEmpty()) {
        QMessageBox::information(nullptr, "No File Selected", "No file was selected to save the JSON.");
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Error", "Could not open the file for writing.");
        return;
    }

    // Write JSON to the file
    file.write(jsonDoc.toJson());
    file.close();

    QMessageBox::information(nullptr, "Success", "JSON file saved successfully!");
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    // Accept the drag if it contains a file
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    // Get the list of URLs from the event
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    // Load the first file in the list
    QString filePath = urls.first().toLocalFile();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Invalid file path.");
        return;
    }

    load(filePath);
}

void MainWindow::load(const QString& filePath) {
    // open file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open JSON file.");
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    // validate data
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    if (!jsonDoc.isObject()) {
        qWarning("Invalid JSON format: Expected an object");
        return;
    }

    QJsonObject data = jsonDoc.object();
    if (!data["place"].isArray() || !data["connect"].isArray()) {
        qWarning("Invalid JSON format: Expected an object->array");
        return;
    }

    // load container
    BlockContainerWrapper* container = blockContainerManager.getContainer(id).get();

    // place blocks
    QJsonArray placeJson = data["place"].toArray();
    for (const QJsonValue& value : placeJson) {
        if (!value.isObject()) {
            qWarning("Invalid JSON format: Expected an object->array->object");
            return;
        }
        QJsonObject placement = value.toObject();
        if (!(placement["x"].isDouble() && placement["y"].isDouble() && placement["r"].isDouble() && placement["t"].isDouble())) {
            qWarning("Invalid JSON format: Expected an object->array->object->'x','y','r','t'");
            return;
        }
        container->tryInsertBlock(Position(placement["x"].toInt(), placement["y"].toInt()), (Rotation)(placement["r"].toInt()), (BlockType)(placement["t"].toInt()));
    }

    // connect blocks
    QJsonArray connectJson = data["connect"].toArray();
    for (const QJsonValue& value : connectJson) {
        if (!value.isObject()) {
            qWarning("Invalid JSON format: Expected an object->array->object.");
            return;
        }
        QJsonObject connection = value.toObject();
        if (!(connection["ox"].isDouble() && connection["oy"].isDouble() && connection["ix"].isDouble() && connection["iy"].isDouble())) {
            qWarning("Invalid JSON format: Expected an object->array->object->'ox','oy','ix','iy'");
            return;
        }
        container->tryCreateConnection(Position(connection["ox"].toInt(), connection["oy"].toInt()), Position(connection["ix"].toInt(), connection["iy"].toInt()));
    }

}
