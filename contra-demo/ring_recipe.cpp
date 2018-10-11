#include <memory>
#include <random>
#include <vector>

#include <arbor/recipe.hpp>
#include <arbor/mc_cell.hpp>
#include <arbor/util/any.hpp>

using namespace arb;

// Parameters used to generate the random cell morphologies.

struct branch_cell_parameters {
    branch_cell_parameters() = default;

    //  Maximum number of levels in the cell (not including the soma)
    unsigned max_depth = 5;

    // The following parameters are described as ranges.
    // The first value is at the soma, and the last value is used on the last level.
    // Values at levels in between are found by linear interpolation.

    std::array<double,2> branch_probs = {1.0, 0.5}; //  Probability of a branch occuring.
    std::array<unsigned,2> compartments = {20, 2};  //  Compartment count on a branch.
    std::array<double,2> lengths = {200, 20};       //  Length of branch in μm.
};

// Helper used to interpolate in branch_cell.

template <typename T>
double interp(const std::array<T,2>& r, unsigned i, unsigned n) {
    double p = i * 1./(n-1);
    double r0 = r[0];
    double r1 = r[1];
    return r[0] + p*(r1-r0);
}

// Generate a single cell in the recipe.

mc_cell branch_cell(arb::cell_gid_type gid, const branch_cell_parameters& params) {
    mc_cell cell;

    // Add soma.
    auto soma = cell.add_soma(12.6157/2.0); // For area of 500 μm².
    soma->rL = 100;
    soma->add_mechanism("hh");

    std::vector<std::vector<unsigned>> levels;
    levels.push_back({0});

    // Standard mersenne_twister_engine seeded with gid.
    std::mt19937 gen(gid);
    std::uniform_real_distribution<double> dis(0, 1);

    double dend_radius = 0.5; // Diameter of 1 μm for each cable.

    unsigned nsec = 1;
    for (unsigned i=0; i<params.max_depth; ++i) {
        // Branch prob at this level.
        double bp = interp(params.branch_probs, i, params.max_depth);
        // Length at this level.
        double l = interp(params.lengths, i, params.max_depth);
        // Number of compartments at this level.
        unsigned nc = std::round(interp(params.compartments, i, params.max_depth));

        std::vector<unsigned> sec_ids;
        for (unsigned sec: levels[i]) {
            for (unsigned j=0; j<2; ++j) {
                if (dis(gen)<bp) {
                    sec_ids.push_back(nsec++);
                    auto dend = cell.add_cable(sec, arb::section_kind::dendrite, dend_radius, dend_radius, l);
                    dend->set_compartments(nc);
                    dend->add_mechanism("pas");
                    dend->rL = 100;
                }
            }
        }
        if (sec_ids.empty()) {
            break;
        }
        levels.push_back(sec_ids);
    }

    // Add spike threshold detector at the soma.
    cell.add_detector({0,0}, 10);

    // Add a synapse to the mid point of the first dendrite.
    cell.add_synapse({1, 0.5}, "expsyn");

    return cell;
}

// Recipe definition.

class ring_recipe: public recipe {
public:
    explicit ring_recipe(unsigned num_cells, unsigned min_delay = 10):
        num_cells_(num_cells),
        min_delay_(min_delay)
    {}

    cell_size_type num_cells() const override {
        return num_cells_;
    }

    util::unique_any get_cell_description(cell_gid_type gid) const override {
        mc_cell c;

        auto soma = c.add_soma(12.6157/2.0);
        soma->add_mechanism("hh");

        c.add_cable(0, section_kind::dendrite, 1.0/2, 1.0/2, 200.0);

        for (auto& seg: c.segments()) {
            if (seg->is_dendrite()) {
                seg->add_mechanism("pas");
                seg->set_compartments(4);
            }
        }

        c.add_detector({0,0}, 10);
        c.add_synapse({1, 0.5}, "expsyn");

        return util::make_unique_any<mc_cell>(c);
    }

    cell_kind get_cell_kind(cell_gid_type gid) const override {
        return cell_kind::cable1d_neuron;
    }

    // Each cell has one spike detector (at the soma).
    cell_size_type num_sources(cell_gid_type gid) const override {
        return 1;
    }


    // The cell has one target synapse, which will be connected to cell gid-1.
    cell_size_type num_targets(cell_gid_type gid) const override {
        return 1;
    }

    std::vector<cell_connection> connections_on(cell_gid_type gid) const override {
        cell_gid_type src = gid? gid-1: num_cells_-1;
        return {cell_connection({src, 0}, {gid, 0}, event_weight_, min_delay_)};
    }

    // Return one event generator on gid 0. This generates a single event that will
    // kick start the spiking.
    std::vector<event_generator> event_generators(cell_gid_type gid) const override {
        std::vector<event_generator> gens;
        if (!gid) {
            gens.push_back(explicit_generator(pse_vector{{{0, 0}, 0.1, 1.0}}));
        }
        return gens;
    }

    // There are two voltage probes per cell, one at the soma and one at the end of
    // the proximal dendritic branch point.
    cell_size_type num_probes(cell_gid_type gid) const override {
        return 2;
    }

    probe_info get_probe(cell_member_type id) const override {
        const auto kind = cell_probe_address::membrane_voltage;

        if (id.index == 0) {
            // Measure at the soma.
            const segment_location loc_soma(0, 0.0);
            return probe_info{id, kind, cell_probe_address{loc_soma, kind}};
        } else {
            // Measure at the dendrite.
            const segment_location loc_dendrite(1, 1.0);
            return probe_info{id, kind, cell_probe_address{loc_dendrite, kind}};
        }
    }

private:
    cell_size_type num_cells_;
    double min_delay_;
    float event_weight_ = 0.01;
};

std::unique_ptr<arb::recipe> make_ring_recipe(unsigned num_cells, float min_delay = 10.f) {
    return std::unique_ptr<recipe>(new ring_recipe(num_cells, min_delay));
}
