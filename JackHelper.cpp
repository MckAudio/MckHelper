#include "JackHelper.h"

void mck::to_json(nlohmann::json &j, const Connection &c)
{
    j["name"] = c.name;
}
void mck::from_json(const nlohmann::json &j, Connection &c)
{
    c.name = j.at("name").get<std::string>();
}

bool mck::NewConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connections)
{
    std::vector<std::string> tmp;
    if (client == nullptr || port == nullptr)
    {
        return false;
    }
    const char **cons = jack_port_get_all_connections(client, port);
    if (cons)
    {
        const char **con = cons;
        for (; *con; con++)
        {
            tmp.push_back(std::string(*con));
        }
        jack_free(cons);
    }

    // Compare connections
    if (connections.size() != tmp.size()) {
        return true;
    }
    std::sort(connections.begin(), connections.end());
    std::sort(tmp.begin(), tmp.end());
    for (unsigned i = 0; i < tmp.size(); i++)
    {
        if (connections[i] != tmp[i]) {
            return true;
        }
    }

    return false;
}
bool mck::GetConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connections)
{
    connections.clear();

    if (client == nullptr || port == nullptr)
    {
        return false;
    }
    const char **cons = jack_port_get_all_connections(client, port);
    if (cons)
    {
        const char **con = cons;
        for (; *con; con++)
        {
            connections.push_back(std::string(*con));
        }
        jack_free(cons);
    }

    return true;
}

bool mck::SetConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connections, bool isInput)
{
    if (client == nullptr || port == nullptr)
    {
        return false;
    }
    if (jack_port_connected(port) > 0)
    {
        jack_port_disconnect(client, port);
    }
    const char* name = jack_port_name(port);
    bool ret = true;

    for (auto &c : connections)
    {
        if (isInput) {
            ret &= (jack_connect(client, c.c_str(), name) == 0);
        } else {
            ret &= (jack_connect(client, name, c.c_str()) == 0);
        }
    }

    return ret;
}