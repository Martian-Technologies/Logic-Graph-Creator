#include "gpu1.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

BlockType strToType(QString str) {
    if (str == "and") return BlockType::AND;
    else if (str == "or") return BlockType::OR;
    else if (str == "xor") return BlockType::XOR;
    else if (str == "nand") return BlockType::NAND;
    else if (str == "nor") return BlockType::NOR;
    else if (str == "xnor") return BlockType::XNOR;
    else if (str == "switch") return BlockType::SWITCH;
}

void makeGPU1(BlockContainerWrapper* container, bool doConnections) {
   
    // Load JSON data using Qt
    QFile file("scripts/gpu1.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open JSON file.");
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
    if (!jsonDoc.isArray()) {
        qWarning("Invalid JSON format: Expected an array.");
        return;
    }
    QJsonArray data = jsonDoc.array();

    for (const QJsonValue& value : data) {
        QJsonObject block = value.toObject();
        QString blockType = block["type"].toString();

        if (blockType == "gate") {
            QJsonObject position = block["position"].toObject();
            container->tryInsertBlock(Position(position["x"].toInt(), position["y"].toInt()), Rotation::ZERO, strToType(block["mode"].toString()));
        } else if (blockType == "switch") {
            QJsonObject position = block["position"].toObject();
            container->tryInsertBlock(Position(position["x"].toInt(), position["y"].toInt()), Rotation::ZERO, BlockType::SWITCH);
        }
    }

    if (doConnections) {
        for (const QJsonValue& value : data) {
            QJsonObject block = value.toObject();
            QJsonObject position = block["position"].toObject();
            int startX = position["x"].toInt();
            int startY = position["y"].toInt();

            QJsonArray outputs = block["outputs"].toArray();
            for (const QJsonValue& outputValue : outputs) {
                QJsonObject end = outputValue.toObject();
                container->tryCreateConnection(Position(startX, startY), Position(end["x"].toInt(), end["y"].toInt()));
            }
        }
    }
}
