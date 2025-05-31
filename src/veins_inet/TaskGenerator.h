// TaskGenerator.h
#ifndef TASK_GENERATOR_H
#define TASK_GENERATOR_H

#include "GraphProcessor.h"
#include <random>
#include <string>
#include <vector>

/// A class to generate routing tasks, compute shortest paths (via A* with h=0, i.e. Dijkstra),
/// and test if a bipartite matching exists between sources and destinations.
class TaskGenerator {
public:
    struct TimeWindow {
        double earliness;
        double tardiness;
    };

    struct Task {
        std::string dest;
        TimeWindow window;
    };

    /// Construct with an existing GraphProcessor (built on your SUMO net)
    explicit TaskGenerator(GraphProcessor& gp);

    /// Generate N random destination tasks with time windows.
    /// Requires GraphProcessor to expose a getAllJunctions() -> vector<string>.
    std::vector<Task> generateTasks(int N);

    /// Return shortest‐path length from `source` to `dest` using A* with h=0 (i.e. Dijkstra).
    /// Returns -1 if unreachable.
    double shortestPathLength(const std::string& source,
                              const std::string& dest);

    /// Given equal‐length lists of sources and destinations, returns true if there is a perfect
    /// one‐to‐one matching where each source can reach its matched destination.
    bool canMatch(const std::vector<std::string>& sources,
                  const std::vector<std::string>& dests);
    std::pair<std::vector<std::string>, std::vector<std::string>> pickSourceDest(int N);

private:
    GraphProcessor& gp_;
    std::mt19937      rng_;

    bool bpm(int u,
             std::vector<bool>& seen,
             std::vector<int>& matchR,
             const std::vector<std::vector<bool>>& adjMatrix);
};

#endif // TASK_GENERATOR_H
