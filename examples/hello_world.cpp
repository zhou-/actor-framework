#include <string>
#include <chrono>
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"

using std::cout;
using std::endl;
using std::string;

#define STATE_CLASS(actor_name)                                                \
  struct actor_name##_state {                                                  \
    static const char* name;                                                   \
  };                                                                           \
  const char* actor_name##_state::name = #actor_name

#define ACTOR_TYPE(actor_name) stateful_actor<actor_name##_state>

using namespace caf;

namespace {

auto default_delay = std::chrono::milliseconds(250);

auto worker_delay = std::chrono::milliseconds(25);

using string_set = std::set<std::string>;
using string_vec = std::vector<std::string>;

std::mutex cout_mtx;

inline void print_impl() {
  std::cout << std::endl;
}

template <class T, class... Ts>
void print_impl(const T& x, const Ts&... xs) {
  std::cout << x;
  print_impl(xs...);
}

template <class... Ts>
void print(const Ts&... xs) {
  std::unique_lock<std::mutex> guard(cout_mtx);
  print_impl(xs...);
}

struct config : public actor_system_config {
  string_vec hosts;
  uint16_t port = 0;
  config() {
    opt_group{custom_options_, "global"}
    .add(hosts, "hosts,H", "")
    .add(port, "port,p", "");
  }
  template <class F>
  void for_each_host(F fun) const {
    string_vec tmp;
    for (auto& str : hosts) {
      tmp.clear();
      split(tmp, str, ':', false);
      if (tmp.size() == 2)
        try {
          fun(std::move(tmp.front()),
              static_cast<uint16_t>(std::stoi(tmp.back())));

        } catch (std::exception&) {
          // nop
        }
    }
  }
};

STATE_CLASS(unreliable_worker);

behavior unreliable_worker(ACTOR_TYPE(unreliable_worker) * self, actor master,
                           actor supervisor) {
  self->link_to(master);
  // passing the supervisor around makes sure there is always at least one
  // strong reference and it won't get "garbage collected"
  self->delayed_send(self, worker_delay, ok_atom::value, supervisor);
  return {
    [=](ok_atom, actor) {
      print("kill worker for ", to_string(master));
      self->quit(exit_reason::user_shutdown);
    }
  };
}

STATE_CLASS(worker_supervisor);

behavior worker_supervisor(ACTOR_TYPE(worker_supervisor)* self,
                           actor master) {
  self->monitor(master);
  print("spawn new worker for ", to_string(master));
  self->spawn<monitored>(unreliable_worker, master, self);
  strong_actor_ptr strong_self{self->ctrl()};
  self->set_down_handler([=](scheduled_actor*, down_msg& x) {
    if (x.source == master) {
      print("remote buddy ", to_string(master), " is down!");
      self->quit(x.reason);
      return;
    }
    print("respawn worker for ", to_string(master));
    self->spawn<monitored>(unreliable_worker, master, self);
  });
  return {[] {}};
}

void ignore_exit(scheduled_actor*, exit_msg& msg) {
  print("received exit message from ", to_string(msg.source));
}

STATE_CLASS(server);

behavior server(ACTOR_TYPE(server)* self) {
  print("spanwed server ", to_string(actor_cast<actor>(self)));
  self->set_exit_handler(ignore_exit);
  return {
    [=](actor new_buddy) {
      print("new remote buddy: ", to_string(new_buddy));
      self->spawn(worker_supervisor, new_buddy);
    }
  };
}

behavior connector(event_based_actor* self, actor master,
                   const std::string& host, uint16_t port) {
  auto mm = self->system().middleman().actor_handle();
  self->set_error_handler([=](scheduled_actor*, error&) {
    print("unable to connect to ", host, " on port ", port, ", try again in ",
          to_string(duration{default_delay}));
    self->delayed_send(mm, default_delay, connect_atom::value, host, port);
  });
  self->send(mm, connect_atom::value, host, port);
  return {
    [=](const node_id& nid, const strong_actor_ptr& hdl, const string_set&) {
      if (hdl == nullptr) {
        print("no actor published on host ", host, " and port ", port);
        return;
      }
      print(to_string(nid), " is ", host, ":", port);
      self->send(master, actor_cast<actor>(hdl));
    }
  };
}

void caf_main(actor_system& sys, const config& cfg) {
  auto master = sys.spawn(server);
  auto port = sys.middleman().publish(master, cfg.port);
  while (port != cfg.port) {
    cout << "middleman.open() failed: "
         << sys.render(port.error())
         << " try again in " << to_string(duration{default_delay})
         << endl;
    std::this_thread::sleep_for(default_delay);
    port = sys.middleman().open(cfg.port);
  }
  cout << "successfully opened port " << cfg.port << endl;;
  cfg.for_each_host([&](const std::string& host, uint16_t port) {
    sys.spawn(connector, master, host, port);
  });
  /*
  std::string dummy;
  std::cin >> dummy;
  anon_send_exit(master, exit_reason::kill);
  sys.middleman().close(cfg.port);
  */
}

} // namespace <anonymous>

CAF_MAIN(io::middleman)

