#include "kdtree.hpp"

namespace lumina {

kdtree::kdtree(std::vector<vec3f32> src) {
    // empty source -> empty tree
    if(src.size() == 0) {
        points_ = {};
        height_ = 0;
        return;
    }

    // calculate height of kD-tree
    height_ = std::floor(std::log2(src.size()));
    // number of elements = 2^(height + 1) - 1
    points_.resize(std::pow(2, height_ + 1) - 1, SENTRY_);

    using iter_t = std::vector<vec3f32>::iterator;
    // (iter_begin, iter_end, height, index)
    std::stack<std::tuple<iter_t, iter_t, u32, u32>> s{};
    s.push({src.begin(), src.end(), 0, 0});

    while(!s.empty()) {
        auto [b, e, h, i] = s.top();
        s.pop();
        
        auto d = std::distance(b, e);

        auto axis = h % 3;
        std::nth_element(b, b + d / 2, e, [&](const auto& a, const auto& b) { return a[axis] < b[axis]; });

        points_[i] = *(b + d / 2);

        if(b + d / 2 + 1 != e) {
            s.push({b + d / 2 + 1, e, h + 1, 2 * i + 2});
        }
        if(b != b + d / 2) {
            s.push({b, b + d / 2, h + 1, 2 * i + 1});
        }
    }
}

vec3f32 kdtree::nn_search(const vec3f32& query) const {
    auto min_d = F32_MAX;
    vec3f32 result{};

    // (height, index)
    std::stack<std::pair<u32, u32>> s{};
    s.push({0, 0});

    while(!s.empty()) {
        auto [h, i] = s.top();
        s.pop();

        auto axis = h % 3;

        auto d = distance(query, points_[i]);
        if(d < min_d) {
            min_d = d;
            result = points_[i];
        }

        std::cout << std::format("{} = {}, d = {}", i, points_[i], distance(query, points_[i])) << std::endl;

        if(h < height_) {
            if(query[axis] >= points_[i][axis]) {
                s.push({h + 1, 2 * i + 2});
            }
            if(query[axis] <= points_[i][axis]) {
                s.push({h + 1, 2 * i + 1});
            }
        }
    }

    return result;
}

}