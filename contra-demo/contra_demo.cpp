#include <fstream>
#include <iomanip>
#include <iostream>
#include <deque>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>

#include <arbor/context.hpp>
#include <arbor/load_balance.hpp>
#include <arbor/load_balance.hpp>
#include <arbor/simulation.hpp>
#include <arbor/version.hpp>

#include "ring_recipe.hpp"
#include "thread_forwarding_sampler.hpp"

struct usage_error {};

struct options {
    int num_cells = 10;
    double sim_time = 1000;
};

options parse_options(int argc, char** argv);

int main(int argc, char** argv) {
    try {
        options opt = parse_options(argc, argv);

        auto context = arb::make_context();

        // Print a banner with information about hardware configuration
        // and run-time options.

        std::cout
            << "run-time configuration:\n"
            << "    gpu:      " << (has_gpu(context)? "yes": "no") << '\n'
            << "    threads:  " << num_threads(context) << '\n'
            << "    mpi:      " << (has_mpi(context)? "yes": "no") << '\n'
            << "    ranks:    " << num_ranks(context) << '\n'
            << "\n"
            << "simulation parameters:\n"
            << "    cells:    " << opt.num_cells << '\n'
            << "    duration: " << opt.sim_time << '\n';

        // Create an instance of our recipe.
        auto recipe = make_ring_recipe(opt.num_cells);

        auto decomp = arb::partition_load_balance(*recipe, context);

        // Construct the model.
        arb::simulation sim(*recipe, decomp, context);

        // Set up the probe that will measure voltage in the cell.

        // The schedule for sampling is 10 samples every 1 ms.
        auto sched = arb::regular_schedule(0.1);

        // Data object for storing traces data

        traces_type traces;
        // locking tools and signals for the thread communication
        std::mutex queue_mutex;
        std::condition_variable wake_up;
        bool quit;

        // Start the thread that will process the posted data
        std::thread worker(publisher, std::ref(traces), std::ref(queue_mutex), std::ref(wake_up), std::ref(quit));

        // Now attach the sampler at probe_id, with sampling schedule sched,
        // Connect the thread_forwarding_sampler that will push all data on a mutex guarded vector
        sim.add_sampler(arb::all_probes, sched,
            thread_forwarding_sampler(traces, queue_mutex, wake_up));

        std::cout << "running simulation\n";

        // Run the simulation for 100 ms, with time steps of 0.025 ms.
        sim.run(opt.sim_time, 0.025);

        {
            // TODO: this doesn't look quite right... check.
            std::lock_guard<std::mutex> guard(queue_mutex);
            wake_up.notify_one();
            quit = true;
        }
        worker.join();

    }
    catch (usage_error& e) {
        std::cerr << "usage: contra-demo [-n number-of-cells] [-t simulation-time-ms]\n";
        return 1;
    }
    catch (std::exception& e) {
        std::cerr << "exception caught in ring miniapp:\n" << e.what() << "\n";
        return 1;
    }

    return 0;
}

// Simple command-line option parsing.

template <typename V>
V parse(const char* arg) {
    if (!arg) throw usage_error{};

    V v;
    std::istringstream s(arg);
    s >> v;

    if (!s) throw usage_error{};
    return v;
}

options parse_options(int argc, char** argv) {
    options opt;

    char** argp = &argv[1];
    while (*argp) {
        if (**argp!='-') break;

        switch ((*argp)[1]) {
        case 'n':
            opt.num_cells = parse<int>(*++argp);
            break;
        case 't':
            opt.sim_time = parse<double>(*++argp);
            break;
        default:
            throw usage_error{};
        }
        ++argp;
    }

    if (*argp) throw usage_error{};
    return opt;
}

