//
// Created by baifeng on 2021/12/7.
//

#ifndef SDL2_UI_MINI_SPAN_TREE_H
#define SDL2_UI_MINI_SPAN_TREE_H

#include "common/types.h"
#include "delabella.h"

namespace dungeon {
    typedef struct Edge {
        int id;
        int start;
        int end;
        int weight;
        Edge():id(0), start(0), end(0), weight(0) {}
    } Edge;

    typedef std::vector<Edge> EdgeGraph;
    typedef std::vector<Edge*> EdgeGraphNoCopy;
    typedef std::vector<mge::Vector2i> UVArray;
    typedef std::vector<int> WeightArray;
    typedef std::vector<int> VertexArray;
    typedef std::vector<VertexArray> VertexTree;

    EdgeGraph create_edge_graph(UVArray const& lines, WeightArray const& weights);
    EdgeGraphNoCopy mini_span_tree_kruskal(EdgeGraph& edges, int vertexCount);
    EdgeGraph create_edge_graph_with_delaunay_triangulate(IDelaBella* idb, std::function<double(int start, int end)> const& cost);
}

#endif //SDL2_UI_MINI_SPAN_TREE_H
