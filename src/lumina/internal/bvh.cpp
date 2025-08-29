#include "bvh.hpp"

namespace lumina {

bvh::bvh(const std::vector<vec3f32>& vertices, const std::vector<vec3u32>& indices) {
    std::vector<u32> index_indices(indices.size());
    std::iota(index_indices.begin(), index_indices.end(), 0);

    std::queue<std::pair<std::vector<u32>, u32>> build_queue{};
    build_queue.push({index_indices, 0});

    u32 axis{};

    nodes_.push_back({});

    while(!build_queue.empty()) {
        auto [idxs, node_idx] = build_queue.front();
        build_queue.pop();

        std::sort(idxs.begin(), idxs.end(), [&](const auto& a, const auto& b) {
            triangle t_a = {vertices[indices[a].x], vertices[indices[a].y], vertices[indices[a].z]};
            triangle t_b = {vertices[indices[b].x], vertices[indices[b].y], vertices[indices[b].z]};

            return t_a.centroid()[axis] < t_b.centroid()[axis];
        });

        std::vector<u32> left_idxs(idxs.data(), idxs.data() + idxs.size() / 2);
        std::vector<u32> right_idxs(idxs.data() + idxs.size() / 2, idxs.data() + idxs.size());
        aabb left_box{};
        aabb right_box{};
        for(const auto& i : left_idxs) {
            left_box += aabb({vertices[indices[i].x], vertices[indices[i].y], vertices[indices[i].z]});
        }
        for(const auto& i : right_idxs) {
            right_box += aabb({vertices[indices[i].x], vertices[indices[i].y], vertices[indices[i].z]});
        }

        nodes_[node_idx].left_box  = left_box;
        nodes_[node_idx].right_box = right_box;

        if(left_idxs.size() == 1) {
            nodes_[node_idx].left_index = -left_idxs[0];
        }
        else {
            auto left_node_idx = s32(nodes_.size());
            nodes_[node_idx].left_index = left_node_idx;
            build_queue.push({left_idxs, left_node_idx});
            nodes_.push_back({});
        }
        if(right_idxs.size() == 1) {
            nodes_[node_idx].right_index = -right_idxs[0];
        }
        else {
            auto right_node_idx = s32(nodes_.size());
            nodes_[node_idx].right_index = right_node_idx;
            build_queue.push({right_idxs, right_node_idx});
            nodes_.push_back({});
        }

        axis = (axis + 1) % 3;
    }
}

std::optional<std::pair<u32, f32>> bvh::trace(const std::vector<vec3f32>& vertices, const std::vector<vec3u32>& indices, const ray& r, f32 t_max) const {
    std::queue<s32> idxs{};

    idxs.push(0);

    f32 t = t_max;
    u32 i = U32_MAX;

    u32 count{};

    while(!idxs.empty()) {
        auto current_idx = idxs.front();
        idxs.pop();

        if(intersect(r, nodes_[current_idx].left_box)) {
            // internal
            if(nodes_[current_idx].left_index > 0) {
                idxs.push(nodes_[current_idx].left_index);
            }
            // leaf
            else {
                auto tri_idx = -nodes_[current_idx].left_index;
                auto curr_t = intersect(r, {vertices[indices[tri_idx].x], vertices[indices[tri_idx].y], vertices[indices[tri_idx].z]});
                if(curr_t) {
                    if(*curr_t < t) {
                        t = *curr_t;
                        i = tri_idx;
                    }
                }
            }
        }
        if(intersect(r, nodes_[current_idx].right_box)) {
            // internal
            if(nodes_[current_idx].right_index > 0) {
                idxs.push(nodes_[current_idx].right_index);
            }
            // leaf
            else {
                auto tri_idx = -nodes_[current_idx].right_index;
                auto curr_t = intersect(r, {vertices[indices[tri_idx].x], vertices[indices[tri_idx].y], vertices[indices[tri_idx].z]});
                if(curr_t) {
                    if(*curr_t < t) {
                        t = *curr_t;
                        i = tri_idx;
                    }
                }
            }
        }
    }

    if(i == U32_MAX) {
        return std::nullopt;
    }
    else {
        return {{i, t}};
    }
}

}