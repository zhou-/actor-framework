#include "caf/all.hpp"
#include "caf/io/all.hpp"

using namespace caf;

using atomPing = atom_constant<atom("ping")>;
using atomPong = atom_constant<atom("pong")>;

namespace {

class ServerConfig : public actor_system_config {
public:
  uint16_t servPort = 9000;
  uint16_t portCount = 1;

  ServerConfig() {
    load<io::middleman>();
    opt_group{custom_options_, "server"}
    .add(servPort, "port,p", "set server port")
    .add(portCount, "port-count,c", "set server port count");
  }
};

std::atomic<size_t> count;

} // namespace <anonymous>

behavior portlistener(event_based_actor* self, uint16_t id, uint16_t port) {
  auto pres =
    self->system().middleman().publish<actor>(self, port, nullptr, false);
  if (!pres) {
    std::cout << "Unable to expose port " << port << std::endl;
    return {};
  }
  std::cout << "Listening on " << *pres << std::endl;
  return {
    [=](atomPing, uint16_t dport) {
      auto c = ++count;
      std::cout << "On " << port << ": "
                << " connection from " << dport << " (" << c << ")"
                << std::endl;
      auto remote = self->system().middleman().remote_actor("localhost", dport);
      if (remote) {
        self->send(*remote, atom("pong"));
      } else {
        std::cout << "ERROR, cannot connect to (port : " << dport << ")"
                  << std::endl;
      }
      if (c == 8)
        self->quit();
    },
    [=](const error& err) {
      std::cout << "ERROR : " << to_string(err) << std::endl;
    }
  };
};

void caf_main(actor_system& system, const ServerConfig& cfg) {
  for (uint16_t i = 0; i < cfg.portCount; i++)
    system.spawn(portlistener, i, cfg.servPort + i);
}

CAF_MAIN()
