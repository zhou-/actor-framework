
#include "caf/io/all.hpp"
#include "caf/all.hpp"


using atomPing = caf::atom_constant<caf::atom("ping")>;
using atomPong = caf::atom_constant<caf::atom("pong")>;


class ClientConfig : public caf::actor_system_config {
public:
  uint16_t servPort = 9000;
  uint16_t servCount = 1;
  uint16_t clientPort = 7000;

  ClientConfig() {
    load<caf::io::middleman>();
    opt_group{custom_options_, "server"}
    .add(servPort, "serv-port,P", "set server port")
    .add(servCount, "serv-count,c", "set server port count");
    opt_group{custom_options_, "client"}
    .add(clientPort, "cli-port,p", "set client port");
  }
};

caf::behavior connectandping(caf::event_based_actor* self, uint16_t myP, uint16_t port) {
  auto& mm = self->system().middleman();
  auto pres = mm.publish<caf::actor>(self, myP, nullptr, true);
  if (!pres) {
    std::cout << "Unable to expose port " << myP << std::endl;
    return {};
  }
  myP = *pres;
  auto remote = mm.remote_actor("localhost", port);
  if (remote) {
    self->send(*remote, atomPing::value, myP);
    std::cout << "PING (" << myP << "->" << port << ")" << std::endl;
  } else {
    std::cout << "connection failed for remote_actor at " << port << std::endl;
    return {};
  }
  return {
      [=](atomPong) {
        std::cout << "OK " << myP << "->" << port << std::endl;
        self->quit(caf::exit_reason::normal);
      },
      [=](caf::error& e) {
        std::cout << caf::to_string(e) << std::endl;
      }
  };
}

void caf_main(caf::actor_system& system, const ClientConfig& cfg) {
  uint16_t port = cfg.servPort;
  uint16_t count = cfg.servCount;
  uint16_t myP = cfg.clientPort;
  for (uint16_t i = 0; i < count; i++)
    system.spawn(connectandping, myP + i, port + i);
}

CAF_MAIN()
