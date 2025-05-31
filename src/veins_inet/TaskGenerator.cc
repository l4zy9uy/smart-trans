// TaskGenerator.cpp
#include "TaskGenerator.h"
#include <algorithm>
#include <stdexcept>

// Constructor
TaskGenerator::TaskGenerator(GraphProcessor& gp)
  : gp_(gp),
    rng_(std::random_device{}())
{}

// 1) Generate N tasks with random time‐windows
std::vector<TaskGenerator::Task> TaskGenerator::generateTasks(int N) {
    auto all = gp_.getAllJunctions();
    if ((int)all.size() < N)
        throw std::runtime_error("Not enough junctions to generate tasks");

    std::shuffle(all.begin(), all.end(), rng_);
    std::uniform_real_distribution<double> distStart(0.0, 50.0);
    std::uniform_real_distribution<double> distSpan(0.0, 50.0);

    std::vector<Task> tasks;
    tasks.reserve(N);
    for (int i = 0; i < N; ++i) {
        double e = distStart(rng_);
        double span = distSpan(rng_);
        tasks.push_back(Task{
            .dest = all[i],
            .window = TimeWindow{ e, e + span }
        });
    }
    return tasks;
}

// 2) Shortest path via A* (h=0)
double TaskGenerator::shortestPathLength(const std::string& source,
                                         const std::string& dest)
{
    return gp_.dijkstra(source, dest);
}

// 3) Bipartite matching test
bool TaskGenerator::canMatch(const std::vector<std::string>& sources,
                             const std::vector<std::string>& dests)
{
    int N = sources.size();
    if ((int)dests.size() != N)
        throw std::invalid_argument("sources and dests must have same size");

    // Build reachability matrix
    std::vector<std::vector<bool>> reach(N, std::vector<bool>(N));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            reach[i][j] = (shortestPathLength(sources[i], dests[j]) >= 0);
        }
    }

    std::vector<int> matchR(N, -1);
    for (int u = 0; u < N; ++u) {
        std::vector<bool> seen(N, false);
        if (!bpm(u, seen, matchR, reach))
            return false;
    }
    return true;
}

bool TaskGenerator::bpm(int u,
                        std::vector<bool>& seen,
                        std::vector<int>& matchR,
                        const std::vector<std::vector<bool>>& adjMatrix)
{
    int N = adjMatrix.size();
    for (int v = 0; v < N; ++v) {
        if (adjMatrix[u][v] && !seen[v]) {
            seen[v] = true;
            if (matchR[v] < 0 || bpm(matchR[v], seen, matchR, adjMatrix)) {
                matchR[v] = u;
                return true;
            }
        }
    }
    return false;
}

std::pair<std::vector<std::string>, std::vector<std::string>>
TaskGenerator::pickSourceDest(int N) {
    auto all = gp_.getAllJunctions();
    if (static_cast<int>(all.size()) < 2 * N) {
        throw std::runtime_error("Not enough junctions for picking sources/dests");
    }
    std::shuffle(all.begin(), all.end(), rng_);

    std::vector<std::string> sources(all.begin(), all.begin() + N);
    std::vector<std::string> dests  (all.begin() + N, all.begin() + 2 * N);
    return {sources, dests};
}