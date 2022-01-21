//
// Created by baifeng on 2021/12/7.
//

#include "mini_span_tree.h"
#include "assert.h"
#include <algorithm>
#include <map>

namespace dungeon {

    EdgeGraph create_edge_graph(UVArray const& lines, WeightArray const& weights) {
        assert(lines.size() == weights.size() && "create_edge_graph fail.");
        EdgeGraph edges;
        edges.resize(lines.size());
        for (int i = 0; i < edges.size(); ++i) {
            auto& edge = edges[i];
            auto& line = lines[i];
            edge.id = i + 1;
            edge.start = line.x;
            edge.end = line.y;
            edge.weight = weights[i];
        }
        return edges;
    }

    bool edge_compare(Edge const& e1, Edge const& e2) {
        return e1.weight < e2.weight;
    }

    bool kruskal_find_tree(int start, int end, VertexTree& tree) {
        int temp_start = -1;
        int temp_end = -1;
        for (int i = 0; i < tree.size(); ++i) {
            auto& array = tree[i];
            auto array_begin = array.begin();
            auto array_end = array.end();
            if (std::find(array_begin, array_end, start) != array_end) {
                temp_start = i;
            }
            if (std::find(array_begin, array_end, end) != array_end) {
                temp_end = i;
            }
        }
        if (temp_start != temp_end) {
            auto& end_array = tree[temp_end];
            auto& start_array = tree[temp_start];
            for (int i = 0; i < end_array.size(); ++i) {
                start_array.push_back(end_array[i]);
            }
            end_array.clear();
            return true;
        }
        return false;
    }

    EdgeGraphNoCopy mini_span_tree_kruskal(EdgeGraph& edges, int vertexCount) {
        EdgeGraphNoCopy ret;
        VertexTree tree(vertexCount);
        for (int i = 0; i < vertexCount; ++i) {
            tree[i].push_back(i);
        }
        ret.reserve(vertexCount - 1);
        std::sort(edges.begin(), edges.end(), edge_compare);
        for (int i = 0; i < edges.size(); ++i) {
            auto& edge = edges[i];
            if (kruskal_find_tree(edge.start, edge.end, tree)) {
                ret.push_back(&edge);
            }
        }
        return ret;
    }

    EdgeGraph create_edge_graph_with_delaunay_triangulate(IDelaBella* idb, std::function<double(int start, int end)> const& cost) {
        UVArray uvarr;
        WeightArray weights;
        std::map<int, bool> tag;
        int const verts = idb->GetNumOutputVerts();
        int const index_scale = idb->GetNumInputPoints() << 8;
        if (verts >= 1) {
            // 保存每条边
            const DelaBella_Triangle* dela = idb->GetFirstDelaunayTriangle();
            int tris = verts / 3;
            for (int i = 0; i < tris; i++) {
                auto v0 = dela->v[0];
                auto v1 = dela->v[1];
                auto v2 = dela->v[2];
                int key0 = std::min(v0->i, v1->i) * index_scale + std::max(v0->i, v1->i);
                if (not tag[key0]) {
                    auto first = v0->i < v1->i ? v0 : v1;
                    auto second = v1->i > v0->i ? v1 : v0;
                    tag[key0] = true;
                    uvarr.push_back({first->i, second->i});
                }
                int key1 = std::min(v1->i, v2->i) * index_scale + std::max(v1->i, v2->i);
                if (not tag[key1]) {
                    auto first = v2->i < v1->i ? v2 : v1;
                    auto second = v1->i > v2->i ? v1 : v2;
                    tag[key1] = true;
                    uvarr.push_back({first->i, second->i});
                }
                int key2 = std::min(v2->i, v0->i) * index_scale + std::max(v2->i, v0->i);
                if (not tag[key2]) {
                    auto first = v0->i < v2->i ? v0 : v2;
                    auto second = v2->i > v0->i ? v2 : v0;
                    tag[key2] = true;
                    uvarr.push_back({first->i, second->i});
                }
                dela = dela->next;
            }
            // 计算每条边的权重
            weights.reserve(uvarr.size());
            for (int i = 0; i < uvarr.size(); ++i) {
                auto& uv = uvarr[i];
                weights.push_back(cost(uv.x, uv.y));
            }
            // 生成连通图
            return create_edge_graph(uvarr, weights);
        }
        return {};
    }
}