#pragma once

#include <vector>
#include <string>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <nlohmann/json.hpp>

namespace mck
{
    namespace jack
    {
        bool GetInputPorts(jack_client_t *client, std::vector<std::string> &connection);
        bool GetOutputPorts(jack_client_t *client, std::vector<std::string> &connection);
        bool NewConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connection);
        bool GetConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connections);
        bool SetConnection(jack_client_t *client, jack_port_t *port, std::string &connection, bool isInput);
        bool SetConnections(jack_client_t *client, jack_port_t *port, std::vector<std::string> &connections, bool isInput);
    }
} // namespace mck