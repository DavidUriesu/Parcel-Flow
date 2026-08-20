#include <QApplication>
#include <vector>

#include "repository.h"
#include "service.h"
#include "gui.h"

int main(int argc, char* argv[]) {
    QApplication application{ argc, argv };

    Repository repository{ "agents.txt", "parcels.txt" };
    Service service{ repository };

    std::vector<AgentGUI*> agentWindows;

    for (const Agent& agent : service.getAgents()) {
        AgentGUI* window = new AgentGUI{ service, agent };
        agentWindows.push_back(window);
        window->show();
    }

    DashboardGUI* dashboard = new DashboardGUI{ service };
    dashboard->show();

    MapWidget* map = new MapWidget{ service };
    map->show();

    int result = application.exec();

    service.saveParcels();

    for (AgentGUI* window : agentWindows) {
        delete window;
    }

    delete dashboard;
    delete map;

    return result;
}