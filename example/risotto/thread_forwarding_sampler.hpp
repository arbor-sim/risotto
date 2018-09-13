#pragma once

/*
* Simple(st?) implementation of a recorder of scalar
* trace data from a cell probe, with some metadata.
*/

#include <stdexcept>
#include <type_traits>
#include <vector>
#include <iostream>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <arbor/common_types.hpp>
#include <arbor/sampling.hpp>
#include <arbor/util/any_ptr.hpp>

using arb::cell_gid_type;
using arb::cell_lid_type;
using arb::cell_size_type;
using arb::cell_member_type;
using arb::cell_kind;
using arb::time_type;
using arb::cell_probe_address;
using arb::mc_cell;
using arb::section_kind;


using traces_type = std::vector<std::tuple< arb::cell_gid_type, arb::cell_lid_type, arb::time_type, double>>;


class thread_forwarding_sampler {
public:
    explicit thread_forwarding_sampler(traces_type &traces, std::mutex& mutex,
        std::condition_variable& wake_up) : traces_(traces) , mutex_(mutex), wake_up_(wake_up)
    {}

    void operator()(cell_member_type probe_id, arb::probe_tag tag, std::size_t n,
        const arb::sample_record* recs) {

        {
            // Take the lock
            std::lock_guard<std::mutex> guard(mutex_);


            // For all samples n in the current batch
            for (std::size_t i = 0; i < n; ++i) {
                // TODO: Do we need to check this every single time?
                //
                if (auto p = arb::util::any_cast<const double*>(recs[i].data)) {
                    // TODO: put *ps in an array and push that to the other side.
                    traces_.push_back({ probe_id.gid, probe_id.index, recs[i].time, *p });
                }
                else {
                    throw std::runtime_error("unexpected sample type in printing_sampler");
                }
            }
        }
        //Tell the other side to wake up outside of the lock
        wake_up_.notify_one();
    }

private:
    traces_type & traces_;
    std::mutex& mutex_;
    std::condition_variable& wake_up_;
};