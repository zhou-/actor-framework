/******************************************************************************
 *                Example for manually pushing into a stream.                 *
 ******************************************************************************/

#include <iostream>
#include <vector>

#include "caf/all.hpp"

using std::endl;
using namespace caf;

namespace {

struct source_state {
  // -- member types -----------------------------------------------------------

  using downstream_manager_type = broadcast_downstream_manager<int>;

  using manager_type = stream_source<downstream_manager_type>;

  // -- constants --------------------------------------------------------------

  static const char* name;

  // -- constructors, destructors, and assignment operators --------------------

  source_state(scheduled_actor* self) : self(self) {
    // nop
  }

  void init() {
    manager = attach_continuous_stream_source(
      self,
      [](unit_t&) {
        // nop
      },
      [](unit_t&, downstream<int>&, size_t) {
        // nop
      },
      [](const unit_t&) {
        // Run continuously until the actor terminates.
        return false;
      });
  }

  // -- memeber variables ------------------------------------------------------

  intrusive_ptr<manager_type> manager;

  scheduled_actor* self;
};

const char* source_state::name = "source";

// Receives integers from a generator and pushes them into a stream.
behavior int_source(stateful_actor<source_state>* self) {
  self->state.init();
  return behavior(
    [=](join_atom) {
      // Add the sender of this message to the stream.
      return self->state.manager->add_outbound_path();
    },
    [=](add_atom, int x) {
      auto& mgr = *self->state.manager;
      if (mgr.out().num_paths() == 0) {
        // No one is listening yet. Drop `x`.
        aout(self) << "Warning: no sink connected yet! Dropping new input.\n";
      } else if (mgr.out().stalled()) {
        // Listeners are too slow to consume data in time. Drop with warning.
        aout(self) << "Warning: source stalled! Dropping new input.\n";
      } else {
        // Push into the output buffer.
        mgr.out().push(x);
        // Tell the stream manager to try producing more batches.
        mgr.push();
      }
    });
}

// Receives and prints integers. Optionally sleeps on each integer to provoke
// dropping at the source.
behavior int_sink(event_based_actor* self) {
  auto sleep_duration = get_or(self->system().config(), "sleep-duration",
                               timespan{0});
  return behavior([=](stream<int> in) {
    // Create a stream manager for implementing a stream sink. Once more, we
    // have to provide three functions: Initializer, Consumer, Finalizer.
    return attach_stream_sink(
      self,
      // Our input source.
      in,
      // Initializer. Here, we store all values we receive. Note that streams
      // are potentially unbound, so this is usually a bad idea outside small
      // examples like this one.
      [](unit_t&) {
        // nop
      },
      // Consumer. Takes individual input elements as `val` and stores them
      // in our history.
      [=](unit_t&, int x) {
        std::this_thread::sleep_for(sleep_duration);
        aout(self) << "int_sink received: " << x << endl;
      },
      // Finalizer. Allows us to run cleanup code once the stream terminates.
      [=](unit_t&, const error& err) {
        if (err) {
          aout(self) << "int_sink aborted with error: " << err << endl;
        } else {
          aout(self) << "int_sink finalized\n";
        }
      });
  });
}

// Generates an integer every 100ms.
behavior int_generator(event_based_actor* self, actor dst) {
  self->delayed_send(self, std::chrono::milliseconds(100), tick_atom::value);
  return behavior([=](tick_atom) {
    self->send(dst, add_atom::value, 42);
    self->delayed_send(self, std::chrono::milliseconds(100), tick_atom::value);
  });
}

struct config : actor_system_config {
  config() {
    opt_group{custom_options_, "global"}
      .add<timespan>("sleep-duration,s",
                     "configured an artificial delay in the sink");
  }
};

void caf_main(actor_system& sys, const config&) {
  // Spin up source and sink.
  auto src = sys.spawn(int_source);
  auto snk = sys.spawn(int_sink);
  // Connect sink and source.
  anon_send(snk * src, join_atom::value);
  // Spin up generator.
  sys.spawn(int_generator, src);
}

} // namespace

CAF_MAIN()
