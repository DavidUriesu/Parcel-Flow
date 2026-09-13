#include "gui.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QPainter>
#include <stdexcept>

AgentGUI::AgentGUI(Service& service, const Agent& agent, QWidget* parent)
    : QWidget{ parent }, service{ service }, agent{ agent } {
    initGUI();
    populateComboBox();
    populateTable();
    connectSignals();

    service.addObserver(this);
}

AgentGUI::~AgentGUI() {
    service.removeObserver(this);
}

void AgentGUI::initGUI() {
    setWindowTitle(QString::fromStdString(agent.getName()));
    resize(700, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout{ this };

    QLabel* areaLabel = new QLabel{
        QString::fromStdString(
            "Service area: center (" +
            std::to_string(agent.getCenterX()) + ", " +
            std::to_string(agent.getCenterY()) + "), radius " +
            std::to_string(agent.getRadius()))
    };

    mainLayout->addWidget(areaLabel);

    QLabel* comboLabel = new QLabel{ "Street:" };
    streetsCombo = new QComboBox{};

    mainLayout->addWidget(comboLabel);
    mainLayout->addWidget(streetsCombo);

    parcelsTable = new QTableWidget{};
    parcelsTable->setColumnCount(6);
    parcelsTable->setHorizontalHeaderLabels({
        "Tracking", "Recipient", "Street", "Number", "X", "Y"
    });
    parcelsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    parcelsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    parcelsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    parcelsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(parcelsTable);

    deliverButton = new QPushButton{ "Deliver" };
    mainLayout->addWidget(deliverButton);
}

void AgentGUI::populateComboBox() {
    streetsCombo->clear();
    streetsCombo->addItem("All streets");

    for (const std::string& street : service.getAllStreets()) {
        streetsCombo->addItem(QString::fromStdString(street));
    }
}

void AgentGUI::populateTable() {
    parcelsTable->clearContents();
    parcelsTable->setRowCount(0);

    std::string selectedStreet = streetsCombo->currentText().toStdString();
    std::vector<Parcel> parcels = service.getParcelsForAgent(agent, selectedStreet);

    for (int i = 0; i < parcels.size(); i++) {
        parcelsTable->insertRow(i);

        parcelsTable->setItem(i, 0, new QTableWidgetItem{ QString::fromStdString(parcels[i].getTrackingNumber()) });
        parcelsTable->setItem(i, 1, new QTableWidgetItem{ QString::fromStdString(parcels[i].getRecipient()) });
        parcelsTable->setItem(i, 2, new QTableWidgetItem{ QString::fromStdString(parcels[i].getStreet()) });
        parcelsTable->setItem(i, 3, new QTableWidgetItem{ QString::fromStdString(parcels[i].getNumber()) });
        parcelsTable->setItem(i, 4, new QTableWidgetItem{ QString::number(parcels[i].getX()) });
        parcelsTable->setItem(i, 5, new QTableWidgetItem{ QString::number(parcels[i].getY()) });
    }
}

void AgentGUI::connectSignals() {
    QObject::connect(streetsCombo, &QComboBox::currentTextChanged, this, [this]() {
        populateTable();
        });

    QObject::connect(deliverButton, &QPushButton::clicked, this, [this]() {
        deliverParcel();
        });
}

int AgentGUI::getSelectedRow() const {
    return parcelsTable->currentRow();
}

Parcel AgentGUI::getSelectedParcel() const {
    int row = getSelectedRow();
    std::string selectedStreet = streetsCombo->currentText().toStdString();
    std::vector<Parcel> parcels = service.getParcelsForAgent(agent, selectedStreet);

    return parcels[row];
}

void AgentGUI::deliverParcel() {
    int row = getSelectedRow();

    if (row < 0) {
        QMessageBox::warning(this, "Error", "No parcel selected.");
        return;
    }

    Parcel parcel = getSelectedParcel();
    try {
        service.deliverParcel(parcel.getTrackingNumber());
    }
    catch (const std::exception& error) {
        QMessageBox::critical(this, "Database error", QString::fromUtf8(error.what()));
    }
}

void AgentGUI::update() {
    populateComboBox();
    populateTable();
}

DashboardGUI::DashboardGUI(Service& service, QWidget* parent)
    : QWidget{ parent }, service{ service } {
    initGUI();
    populateTable();
    connectSignals();

    service.addObserver(this);
}

DashboardGUI::~DashboardGUI() {
    service.removeObserver(this);
}

void DashboardGUI::initGUI() {
    setWindowTitle("Dashboard");
    resize(800, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout{ this };

    parcelsTable = new QTableWidget{};
    parcelsTable->setColumnCount(8);
    parcelsTable->setHorizontalHeaderLabels({
        "Tracking", "Recipient", "Street", "Number", "X", "Y", "Agent", "Delivered"
    });
    parcelsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    parcelsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(parcelsTable);

    recipientEdit = new QLineEdit{};
    recipientEdit->setPlaceholderText("Recipient");

    streetEdit = new QLineEdit{};
    streetEdit->setPlaceholderText("Street");

    numberEdit = new QLineEdit{};
    numberEdit->setPlaceholderText("Number");

    xEdit = new QLineEdit{};
    xEdit->setPlaceholderText("X");

    yEdit = new QLineEdit{};
    yEdit->setPlaceholderText("Y");

    addButton = new QPushButton{ "Add parcel" };

    mainLayout->addWidget(recipientEdit);
    mainLayout->addWidget(streetEdit);
    mainLayout->addWidget(numberEdit);
    mainLayout->addWidget(xEdit);
    mainLayout->addWidget(yEdit);
    mainLayout->addWidget(addButton);
}

void DashboardGUI::populateTable() {
    parcelsTable->clearContents();
    parcelsTable->setRowCount(0);

    std::vector<Parcel> parcels = service.getParcels();

    for (int i = 0; i < parcels.size(); i++) {
        parcelsTable->insertRow(i);

        QTableWidgetItem* trackingItem = new QTableWidgetItem{ QString::fromStdString(parcels[i].getTrackingNumber()) };
        QTableWidgetItem* recipientItem = new QTableWidgetItem{ QString::fromStdString(parcels[i].getRecipient()) };
        QTableWidgetItem* streetItem = new QTableWidgetItem{ QString::fromStdString(parcels[i].getStreet()) };
        QTableWidgetItem* numberItem = new QTableWidgetItem{ QString::fromStdString(parcels[i].getNumber()) };
        QTableWidgetItem* xItem = new QTableWidgetItem{ QString::number(parcels[i].getX()) };
        QTableWidgetItem* yItem = new QTableWidgetItem{ QString::number(parcels[i].getY()) };
        QTableWidgetItem* agentItem = new QTableWidgetItem{
            QString::fromStdString(service.getAssignedAgentName(parcels[i]))
        };
        QTableWidgetItem* deliveredItem = new QTableWidgetItem{ parcels[i].isDelivered() ? "true" : "false" };

        if (parcels[i].isDelivered()) {
            trackingItem->setBackground(Qt::green);
            recipientItem->setBackground(Qt::green);
            streetItem->setBackground(Qt::green);
            numberItem->setBackground(Qt::green);
            xItem->setBackground(Qt::green);
            yItem->setBackground(Qt::green);
            agentItem->setBackground(Qt::green);
            deliveredItem->setBackground(Qt::green);
        }

        parcelsTable->setItem(i, 0, trackingItem);
        parcelsTable->setItem(i, 1, recipientItem);
        parcelsTable->setItem(i, 2, streetItem);
        parcelsTable->setItem(i, 3, numberItem);
        parcelsTable->setItem(i, 4, xItem);
        parcelsTable->setItem(i, 5, yItem);
        parcelsTable->setItem(i, 6, agentItem);
        parcelsTable->setItem(i, 7, deliveredItem);
    }
}

void DashboardGUI::connectSignals() {
    QObject::connect(addButton, &QPushButton::clicked, this, [this]() {
        addParcel();
        });
}

void DashboardGUI::addParcel() {
    std::string recipient = recipientEdit->text().trimmed().toStdString();
    std::string street = streetEdit->text().trimmed().toStdString();
    std::string number = numberEdit->text().trimmed().toStdString();

    bool xIsValid;
    bool yIsValid;
    int x = xEdit->text().trimmed().toInt(&xIsValid);
    int y = yEdit->text().trimmed().toInt(&yIsValid);

    if (!xIsValid || !yIsValid) {
        QMessageBox::warning(this, "Invalid parcel", "Coordinates must be whole numbers.");
        return;
    }

    try {
        service.addParcel(recipient, street, number, x, y);
    }
    catch (const std::invalid_argument& error) {
        QMessageBox::warning(this, "Invalid parcel", QString::fromUtf8(error.what()));
        return;
    }
    catch (const std::exception& error) {
        QMessageBox::critical(this, "Database error", QString::fromUtf8(error.what()));
        return;
    }

    recipientEdit->clear();
    streetEdit->clear();
    numberEdit->clear();
    xEdit->clear();
    yEdit->clear();
}

void DashboardGUI::update() {
    populateTable();
}

MapWidget::MapWidget(Service& service, QWidget* parent)
    : QWidget{ parent }, service{ service } {
    setWindowTitle("Map");
    resize(700, 500);

    service.addObserver(this);
}

MapWidget::~MapWidget() {
    service.removeObserver(this);
}

void MapWidget::paintEvent(QPaintEvent*) {
    QPainter painter{ this };

    painter.setPen(Qt::black);
    painter.setBrush(Qt::blue);

    std::vector<Parcel> parcels = service.getUndeliveredParcels();

    for (const Parcel& parcel : parcels) {
        int drawX = parcel.getX() * 10 + 20;
        int drawY = parcel.getY() * 10 + 20;

        painter.drawEllipse(drawX, drawY, 14, 14);
        painter.drawText(drawX + 18, drawY + 12, QString::fromStdString(parcel.getRecipient()));
    }
}

void MapWidget::update() {
    repaint();
}
