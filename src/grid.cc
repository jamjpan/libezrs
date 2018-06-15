// MIT License
//
// Copyright (c) 2018 the Cargo authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <algorithm>
#include <iostream> /* debug */
#include <vector>

#include "libcargo/cargo.h" /* bbox(), node2pt() */
#include "libcargo/classes.h"
#include "libcargo/distance.h"
#include "libcargo/grid.h"
#include "libcargo/types.h"

namespace cargo {

Grid::Grid(int n)
{
    x_dim_ = (Cargo::bbox().upper_right.lng - Cargo::bbox().lower_left.lng) / n;
    y_dim_ = (Cargo::bbox().upper_right.lat - Cargo::bbox().lower_left.lat) / n;
    data_.resize(n * n, {});
    n_ = n;
}

void Grid::insert(const Vehicle& veh)
{
    MutableVehicle mveh(veh);
    insert(mveh);
}

void Grid::insert(const MutableVehicle& mveh)
{
    // Pushes back a copy
    data_.at(hash(Cargo::node2pt(mveh.last_visited_node()))).push_back(mveh);
}

std::vector<MutableVehicle> Grid::within_about(const DistanceDouble& d,
                                        const NodeId& node)
{
    std::vector<MutableVehicle> res;
    int offset_x = std::ceil(metersTolngdegs(d, Cargo::node2pt(node).lat)/x_dim_);
    int offset_y = std::ceil(metersTolatdegs(d)/y_dim_);
    int base_x = hash_x(Cargo::node2pt(node));
    int base_y = hash_y(Cargo::node2pt(node));
    // i,j must be positive, and less than n_
    for (int j = std::max(0, base_y - offset_y); j <= std::min(base_y + offset_y, n_-1); ++j)
        for (int i = std::max(0, base_x - offset_x); i <= std::min(base_x + offset_x, n_-1); ++i) {
            int k = i+j*n_;
            res.insert(res.end(), data_.at(k).begin(), data_.at(k).end());
        }
    return res;
}

void Grid::refresh(const MutableVehicle& mveh)
{
    // Presumably veh.last_visited_node() hasn't changed since veh
    // was indexed... MutableVehicle does not allow to change it
    int i = hash(Cargo::node2pt(mveh.last_visited_node()));
    for (auto& mv : data_.at(i))
        if (mv.id() == mveh.id())
            mv = mveh;
}

void Grid::clear()
{
    data_.clear();
    data_.resize(n_*n_, {});
}

int Grid::hash(const Point& coord)
{
    // x nor y can be greater than n_
    return std::min(hash_x(coord), n_ - 1) +
           std::min(hash_y(coord), n_ - 1) * n_;
}

int Grid::hash_x(const Point& coord)
{
    return (int)std::floor((coord.lng - Cargo::bbox().lower_left.lng) / x_dim_);
}

int Grid::hash_y(const Point& coord)
{
    return (int)std::floor((coord.lat - Cargo::bbox().lower_left.lat) / y_dim_);
}

} // namespace cargo
