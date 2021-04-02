#include "JackHelper.hpp"

bool mck::jack::GetInputPorts(jack_client_t *client, std::vector<std::string> &connection)
{
    connection.clear();
    const char *type = "audio";
    const char **cons = jack_get_ports(client, NULL, "audio", JackPortIsOutput);

    if (cons)
    {
        const char **con = cons;
        for (; *con; con++)
        {
            jack_port_t *port = jack_port_by_name(client, *con);
            if (jack_port_is_mine(client, port) == 0) {
                connection.push_back(std::string(*con));
            }
        }
        jack_free(cons);
    }
    return true;
}

bool mck::jack::GetOutputPorts(jack_client_t *client, std::vector<std::string> &connection)
{
    connection.clear();
    const char *type = "audio";
    const char **cons = jack_get_ports(client, NULL, "audio", JackPortIsInput);
    if (cons)
    {
        const char **con = cons;
        for (; *con; con++)
        {
            jack_port_t *port = jack_port_by_name(client, *con);
            if (jack_port_is_mine(client, port) == 0) {
                connection.push_back(std::string(*con));
            }
        }
        jack_free(cons);
    }
    return true;
}

bool mck::jack::NewConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connections)
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
    if (connections.size() != tmp.size())
    {
        return true;
    }
    std::sort(connections.begin(), connections.end());
    std::sort(tmp.begin(), tmp.end());
    for (unsigned i = 0; i < tmp.size(); i++)
    {
        if (connections[i] != tmp[i])
        {
            return true;
        }
    }

    return false;
}
bool mck::jack::GetConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connections)
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

bool mck::jack::SetConnection(jack_client_t *client, jack_port_t *port, std::string &connection, bool isInput)
{
    if (client == nullptr || port == nullptr)
    {
        return false;
    }

    if (connection == "")
    {
        if (jack_port_connected(port) > 0)
        {
            jack_port_disconnect(client, port);
        }
        return true;
    }

    bool ret = true;
    const char *portName = jack_port_name(port);
    bool connect = true;
    const char **cons = jack_port_get_all_connections(client, port);
    if (cons)
    {
        const char **con = cons;
        for (; *con; con++)
        {
            if (std::string(*con) != connection)
            {
                jack_disconnect(client, portName, *con);
            }
            else
            {
                connect = false;
            }
        }
        jack_free(cons);
    }

    std::vector<std::string> newCons;

    if (connect)
    {
        if (isInput)
        {
            ret &= (jack_connect(client, connection.c_str(), portName) == 0);
        }
        else
        {
            ret &= (jack_connect(client, portName, connection.c_str()) == 0);
        }
    }

    return ret;
}

bool mck::jack::SetConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connections, bool isInput)
{
    if (client == nullptr || port == nullptr)
    {
        return false;
    }
    if (jack_port_connected(port) > 0)
    {
        jack_port_disconnect(client, port);
    }
    const char *name = jack_port_name(port);
    bool ret = true;

    std::vector<std::string> newCons;

    for (auto &c : connections)
    {
        if (c == "")
        {
            continue;
        }

        if (isInput)
        {
            ret &= (jack_connect(client, c.c_str(), name) == 0);
        }
        else
        {
            ret &= (jack_connect(client, name, c.c_str()) == 0);
        }

        newCons.push_back(c);
    }

    connections = newCons;

    return ret;
}