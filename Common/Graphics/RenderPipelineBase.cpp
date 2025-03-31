#include "pch.h"
#include "RenderPipelineBase.h"
#include <ranges>

using namespace NCL::Rendering;

void RenderPipeline::RebuildDAG() {
    std::vector<std::vector<int>> adjacency(passes.size());
    std::vector<int> inDegree(passes.size(), 0);

    // We'll also produce a local array for storing topological order as indices
    std::vector<int> sortedIndices;
    sortedIndices.reserve(passes.size());

    // Gather producers for each resource type
    std::unordered_map<std::type_index, int> producerForResource;   

    //for (int i = 0; i < (int)passes.size(); i++) {
    //    auto dep = passes[i]->DeclareDependencies();
    //    for (auto& rtype : dep.producedResources) {
    //        // If multiple passes produce the same type, warn or handle as you see fit
    //        producerForResource[rtype] = i;
    //    }
    //}
    // 
    //// Build adjacency edges using resource matches 
    //for (int i = 0; i < (int)passes.size(); i++) {
    //    auto dep = passes[i]->DeclareDependencies();
    //    for (auto& rtype : dep.requiredResources) {
    //        auto it = producerForResource.find(rtype);
    //        if (it != producerForResource.end()) {
    //            int producerIndex = it->second;
    //            // producer -> i
    //            adjacency[producerIndex].push_back(i);
    //            inDegree[i]++;
    //        }
    //        // else it's an external resource: no edge
    //    }
    //}

    // Kahn's Algorithm for topological sort
    auto zeroRange = std::views::iota(0, (int)passes.size())
        | std::views::filter([&](int i) { return inDegree[i] == 0; });
    std::queue<int> zeroQueue;
    for (int idx : zeroRange) {
        zeroQueue;
    }

    while (!zeroQueue.empty()) {
        int current = zeroQueue.front();
        zeroQueue.pop();
        sortedIndices.push_back(current);

        for (int neighbor : adjacency[current]) {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0) {
                zeroQueue.push(neighbor);
            }
        }
    }

    if ((int)sortedIndices.size() < (int)passes.size()) {
        throw std::runtime_error("Cycle or resource conflict detected!");
    }

    // Store final pass pointers in sortedPasses for the pipeline to use
    sortedPasses.clear();
    for (int idx : sortedIndices) {
        sortedPasses.push_back(passes[idx]);
    }
}