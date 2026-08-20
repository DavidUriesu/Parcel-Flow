#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include "service.h"

class AgentGUI : public QWidget, public Observer {
private:
    Service& service;
    Agent agent;

    QTableWidget* parcelsTable;
    QComboBox* streetsCombo;
    QPushButton* deliverButton;

    void initGUI();
    void connectSignals();
    void populateComboBox();
    void populateTable();

    int getSelectedRow() const;
    Parcel getSelectedParcel() const;

    void deliverParcel();

public:
    AgentGUI(Service& service, const Agent& agent, QWidget* parent = nullptr);
    ~AgentGUI() override;

    void update() override;
};

class DashboardGUI : public QWidget, public Observer {
private:
    Service& service;

    QTableWidget* parcelsTable;

    QLineEdit* recipientEdit;
    QLineEdit* streetEdit;
    QLineEdit* numberEdit;
    QLineEdit* xEdit;
    QLineEdit* yEdit;

    QPushButton* addButton;

    void initGUI();
    void connectSignals();
    void populateTable();

    void addParcel();

public:
    DashboardGUI(Service& service, QWidget* parent = nullptr);
    ~DashboardGUI() override;

    void update() override;
};

class MapWidget : public QWidget, public Observer {
private:
    Service& service;

protected:
    void paintEvent(QPaintEvent*) override;

public:
    MapWidget(Service& service, QWidget* parent = nullptr);
    ~MapWidget() override;

    void update() override;
};
