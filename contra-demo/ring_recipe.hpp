#pragma once

#include <memory>

#include <arbor/recipe.hpp>

std::unique_ptr<arb::recipe> make_ring_recipe(unsigned num_cells, float min_delay = 10.f);
