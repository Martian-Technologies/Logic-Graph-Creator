#include "validateCircuit.h"
#include <stack>

void CircuitValidator::validate() {
    bool isValid = true;

    isValid = isValid && validateDependencies();

    isValid = isValid && setBlockPositionsInt();
    isValid = isValid && handleInvalidConnections();
    isValid = isValid && setOverlapsUnpositioned();
    isValid = isValid && handleUnpositionedBlocks();

    parsedCircuit.valid = isValid;
}

bool CircuitValidator::validateDependencies() {
    int numImports = 0;

    // Setup offset for possibly merging dependencies
    Vector offset(parsedCircuit.maxPos.dx + (parsedCircuit.blocks.empty()?0:3), parsedCircuit.minPos.dy);

    for (auto& [depName, depCircuit] : parsedCircuit.dependencies) {
        // validate the dependency as a circuit itself
        CircuitValidator depValidator(*depCircuit);
        if (!depCircuit->valid) {
            logError("Dependency circuit validation failed for " + depName);
            parsedCircuit.valid = false;
            return false;
        }
        logInfo("Dependency circuit validation success for " + depName);
    }

    logInfo("File dependency size: " + std::to_string(parsedCircuit.dependencies.size()));

    return true;
}

bool CircuitValidator::setBlockPositionsInt() {
    for (auto& [id, block] : parsedCircuit.blocks) {
        if (!isIntegerPosition(block.pos)) {
            FPosition oldPosition = block.pos;
            block.pos.x = std::floor(block.pos.x);
            block.pos.y = std::floor(block.pos.y);
            if (block.pos.x < parsedCircuit.minPos.dx) parsedCircuit.minPos.dx = block.pos.x;
            if (block.pos.y < parsedCircuit.minPos.dy) parsedCircuit.minPos.dy = block.pos.y;
            if (block.pos.x > parsedCircuit.maxPos.dx) parsedCircuit.maxPos.dx = block.pos.x;
            if (block.pos.y > parsedCircuit.maxPos.dy) parsedCircuit.maxPos.dy = block.pos.y;

            logInfo("Converted block id=" + std::to_string(id) +
                        " position from " + oldPosition.toString() + " to " + block.pos.toString(),
                    "CircuitValidator");
        }
    }
    return true;
}

bool CircuitValidator::handleInvalidConnections() {
    // map to connection frequencies
    std::unordered_map<ParsedCircuit::ConnectionData, int, ConnectionHash> connectionCounts;
    
    // count the connections
    for (auto& conn : parsedCircuit.connections) {
        ++connectionCounts[conn];
    }

    int i = 0;
    while (i < (int)parsedCircuit.connections.size()) {
        ParsedCircuit::ConnectionData& conn = parsedCircuit.connections[i];

        ParsedCircuit::ConnectionData reversePair{
            .outputBlockId = conn.inputBlockId,
            .outputId = conn.inputId,
            .inputBlockId = conn.outputBlockId,
            .inputId = conn.outputId,
        };

        if(--connectionCounts[reversePair] < 0){
            parsedCircuit.connections.push_back(reversePair);
            logInfo("Added reciprocated connection between: (" +
                        std::to_string(conn.inputBlockId) + ' ' +
                        std::to_string(conn.outputBlockId) + ") and (" +
                        std::to_string(reversePair.inputBlockId) + ' ' +
                        std::to_string(reversePair.outputBlockId) + ")",
                    "CircuitValidator");
            connectionCounts[reversePair] = 0;
        }
        ++i;
    }

    // check all remaining connections were found
    for (const auto& [pair, count] : connectionCounts) {
        if (count != 0){
            logWarning("Invalid connection handling, connection frequency: (" +
                         std::to_string(pair.outputBlockId) + ' ' +
                         std::to_string(pair.inputBlockId) + ") " +
                         std::to_string(count),
                     "CircuitValidator");
            return false;
        }
    }

    return true;
}

bool CircuitValidator::setOverlapsUnpositioned() {
    std::unordered_set<Position> occupiedPositions;

    for (auto& [id, block] : parsedCircuit.blocks) {
        if (block.pos.x == std::numeric_limits<float>::max() || 
            block.pos.y == std::numeric_limits<float>::max()) {
            continue;
        }
        Position intPos(static_cast<int>(block.pos.x), static_cast<int>(block.pos.y));
        if (!occupiedPositions.insert(intPos).second){
            // set the block position as effectively undefined
            logInfo("Found overlapped block position at " + block.pos.toString() + " --> " + intPos.toString() + ", setting to undefined position", "CircuitValidator");
            block.pos.x = std::numeric_limits<float>::max();
            block.pos.y = std::numeric_limits<float>::max();
        }
    }

    return true;
}

// SCC META GRAPH TOPOLOGICAL SORT
bool CircuitValidator::handleUnpositionedBlocks() {
    // Separate components so that we can place down disconnected components independently
    std::unordered_map<block_id_t, std::vector<block_id_t>> undirectedAdj;
    for (const ParsedCircuit::ConnectionData& conn : parsedCircuit.connections) {
        block_id_t u = conn.inputBlockId;
        block_id_t v = conn.outputBlockId;
        undirectedAdj[u].push_back(v);
        undirectedAdj[v].push_back(u);
    }

    // find cc's by dfs
    std::vector<std::unordered_set<block_id_t>> components;
    std::unordered_set<block_id_t> visited;
    std::function<void(block_id_t)> dfsComponent = [&](block_id_t node) {
        visited.insert(node);
        components.back().insert(node);
        for (block_id_t neighbor : undirectedAdj[node]) {
            if (!visited.count(neighbor)) {
                dfsComponent(neighbor);
            }
        }
    };
    for (const auto& [id, block] : parsedCircuit.blocks) {
        if (!visited.count(id)) {
            components.push_back({});
            dfsComponent(id);
        }
    }

    const int componentSpacing = 5;
    int currentYOffset = 0;
    int ccIndex = 0;
    for (const std::unordered_set<block_id_t>& cc: components){
        logInfo("Parsing CC " + std::to_string(++ccIndex));

        // adjacency list
        std::unordered_map<block_id_t, std::vector<block_id_t>> adj;
        for (const auto& conn : parsedCircuit.connections) {
            if (cc.count(conn.inputBlockId) && cc.count(conn.outputBlockId)) {
                block_id_t to = conn.outputBlockId;
                block_id_t from = conn.inputBlockId;
                //logInfo("From: " + std::to_string(from) + "("+std::to_string(conn.outputId)+")" + 
                //        ", To: " + std::to_string(to) + "("+std::to_string(conn.inputId)+")");
                if (isConnectionInput(parsedCircuit.blocks.at(conn.outputBlockId).type, conn.outputId)){
                    adj[from].push_back(to);
                }
            }
        }

        // find SCC metagraph DAG for topological sort using kosaraju's
        std::unordered_map<block_id_t, std::vector<block_id_t>> revAdj;
        std::stack<block_id_t> postNumber; // highest post number node is a part of a source SCC, run on G^R
        std::vector<std::vector<block_id_t>> sccs;

        std::function<void(block_id_t)> dfs =
            [&](block_id_t node) -> void {
                visited.insert(node);
                for (block_id_t v : adj[node]) {
                    if (!visited.count(v)) {
                        dfs(v);
                    }
                }
                postNumber.push(node); // post visit
            };

        std::function<void(block_id_t, std::vector<block_id_t>&)> dfsRev =
            [&](block_id_t node, std::vector<block_id_t>& component) -> void {
                visited.insert(node);
                component.push_back(node);
                for (block_id_t v : revAdj[node]) {
                    if (!visited.count(v)) {
                        dfsRev(v, component);
                    }
                }
            };

        // gather stack that shows decreasing post visit numbers
        visited.clear();
        for (block_id_t id : cc) {
            if (!visited.count(id)) {
                dfs(id);
            }
        }

        // make reverse graph in adjacency list
        for (const auto& [u, adjs] : adj) {
            for (block_id_t v : adjs) {
                revAdj[v].push_back(u);
            }
        }

        // run dfs from source SCC's in reverse graph, sink SCCs of regular
        visited.clear();
        while (!postNumber.empty()) {
            block_id_t node = postNumber.top();
            postNumber.pop();
            if (!visited.count(node)) {
                std::vector<block_id_t> component;
                dfsRev(node, component);
                sccs.push_back(component);
            }
        }

        // Make metagraph and run topo sort
        std::unordered_map<int, std::vector<int>> metaGraph;
        std::unordered_map<block_id_t, int> blockToScc;
        for (int i = 0; i < sccs.size(); ++i) {
            for (block_id_t block : sccs[i]) {
                blockToScc[block] = i;
            }
        }

        std::unordered_map<int, int> sccInDegree;
        for (const auto& [u, adjs] : adj) {
            int sccU = blockToScc[u];
            for (block_id_t v : adjs) {
                int sccV = blockToScc[v];
                if (sccU != sccV) {
                    // add sccV as a new adjacency to sccU
                    metaGraph[sccU].push_back(sccV);
                    ++sccInDegree[sccV]; // SCC gains an incoming edge
                }
            }
        }

        // topological sort via removing source nodes
        std::queue<int> q;
        for (int i = 0; i < sccs.size(); ++i) {
            if (sccInDegree[i] == 0) {
                q.push(i);
            }
        }

        std::vector<int> sccOrder;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            sccOrder.push_back(u);
            for (int v : metaGraph[u]) {
                if (--sccInDegree[v] == 0) {
                    q.push(v);
                }
            }
        }

        if (sccOrder.size() != sccs.size()) {
            logError("Cycle in SCC meta graph. Topological sort failed.", "CircuitValidator");
            return false;
        }

        std::unordered_map<block_id_t, int> layers;
        for (int sccIndex : sccOrder) {
            for (block_id_t id : sccs[sccIndex]) {
                int maxLayer = 0;
                for (const auto& conn : parsedCircuit.connections) {
                    if (conn.inputBlockId == id) {
                        maxLayer = std::max(maxLayer, layers[conn.outputBlockId] + 1);
                    }
                }
                layers[id] = maxLayer;
            }
        }

        // Get occupied positions to avoid overlaps
        std::unordered_set<Position> occupiedPositions;
        for (const auto& [id, block] : parsedCircuit.blocks) {
            if (block.pos.x != std::numeric_limits<float>::max() && 
                    block.pos.y != std::numeric_limits<float>::max()) {
                occupiedPositions.insert(Position(
                            static_cast<int>(block.pos.x),
                            static_cast<int>(block.pos.y)
                            ));
            }
        }

        // place blocks in layers
        const int xSpacing = 2; // x spacing between layers
        std::unordered_map<int, int> layerYcounter;
        int maxYPlaced = parsedCircuit.minPos.dy;
        for (int sccIndex : sccOrder) {
            for (block_id_t id : sccs[sccIndex]){
                ParsedCircuit::BlockData& block = parsedCircuit.blocks[id];
                if (block.pos.x != std::numeric_limits<float>::max() && 
                        block.pos.y != std::numeric_limits<float>::max()) {
                    continue;
                }

                const int layer = layers[id];
                const int x = static_cast<int>(parsedCircuit.minPos.dx) + (layer-1) * xSpacing;

                // find first available Y position in current layer column
                int y = static_cast<int>(parsedCircuit.minPos.dy) + currentYOffset;

                if (layerYcounter.find(x) != layerYcounter.end()) {
                    y = layerYcounter[x];
                }

                while (occupiedPositions.count(Position(x, y))) {
                    ++y;
                }

                maxYPlaced = std::max(maxYPlaced, y);
                block.pos = FPosition(x, y);
                occupiedPositions.insert(Position(x, y));
                layerYcounter[x] = y + 1;

                parsedCircuit.minPos.dx = std::fmin(parsedCircuit.minPos.dx, static_cast<float>(x));
                parsedCircuit.minPos.dy = std::fmin(parsedCircuit.minPos.dy, static_cast<float>(y));
                parsedCircuit.maxPos.dx = std::fmax(parsedCircuit.maxPos.dx, static_cast<float>(x));
                parsedCircuit.maxPos.dy = std::fmax(parsedCircuit.maxPos.dy, static_cast<float>(y));

                //logInfo("Placed block " + std::to_string(id) + " at " + 
                //        block.pos.toString() + " in layer " + std::to_string(layer), 
                //        "CircuitValidator");
            }
        }
        currentYOffset = componentSpacing + maxYPlaced;
    }

    return true;
}
