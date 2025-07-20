#include "parsedCircuit.h"

void CircuitValidator::validate() {
	bool isValid = true;

	isValid = isValid && validateBlockData();
	isValid = isValid && validateBlockTypes();
	isValid = isValid && setBlockPositionsInt();
	isValid = isValid && handleInvalidConnections();
	isValid = isValid && setOverlapsUnpositioned();
	isValid = isValid && handleUnpositionedBlocks();

	parsedCircuit.valid = isValid;
}

bool CircuitValidator::validateBlockData() {
	Vector size = parsedCircuit.getSize();
	if (parsedCircuit.getSize().dx == 0)
		size.dx = 1;
	if (parsedCircuit.getSize().dy == 0)
		size.dy = 1;
	for (auto& port : parsedCircuit.getConnectionPorts()) {
		size.extentToFit(port.positionOnBlock + Vector(1));
	}
	parsedCircuit.setSize(size);
	return true;
}

bool CircuitValidator::validateBlockTypes() {
	for (std::pair<const block_id_t, ParsedCircuit::BlockData>& p : parsedCircuit.blocks) {
		if (p.second.type == BlockType::NONE) {
			logWarning("Found a NONE type block, converting to buffer", "CircuitValidator");
			p.second.type = BlockType::JUNCTION;
			//return false;
		}
	}
	return true;
}

bool CircuitValidator::setBlockPositionsInt() {
	parsedCircuit.makePositionsRelative();

	for (auto& [id, block] : parsedCircuit.blocks) {
		if (!isIntegerPosition(block.pos)) {
			FPosition oldPosition = block.pos;
			block.pos.x = std::floor(block.pos.x);
			block.pos.y = std::floor(block.pos.y);
			if (block.pos.x < parsedCircuit.minPos.dx) parsedCircuit.minPos.dx = block.pos.x;
			if (block.pos.y < parsedCircuit.minPos.dy) parsedCircuit.minPos.dy = block.pos.y;
			if (block.pos.x > parsedCircuit.maxPos.dx) parsedCircuit.maxPos.dx = block.pos.x;
			if (block.pos.y > parsedCircuit.maxPos.dy) parsedCircuit.maxPos.dy = block.pos.y;

			logInfo("Converted block id={} position from {} to {}", "CircuitValidator", id, oldPosition.toString(), block.pos.toString());
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

		ParsedCircuit::ConnectionData reversePair(conn.inputBlockId, conn.inputId, conn.outputBlockId, conn.outputId);

		if (--connectionCounts[reversePair] < 0) {
			parsedCircuit.connections.push_back(reversePair);
			logInfo("Added reciprocated connection between: ({} {}) and ({} {})", "CircuitValidator", conn.inputBlockId, conn.inputId, conn.outputBlockId, conn.outputId);
			connectionCounts[reversePair] = 0;
		}
		++i;
	}

	// check all remaining connections were found
	for (const auto& [pair, count] : connectionCounts) {
		if (count != 0) {
			logWarning("Invalid connection handling, connection frequency: ({} {}) {}", "CircuitValidator", pair.outputBlockId, pair.inputBlockId, count);
			return false;
		}
	}

	return true;
}

bool CircuitValidator::setOverlapsUnpositioned() {
	for (auto& [id, block] : parsedCircuit.blocks) {
		if (block.pos.x == std::numeric_limits<float>::max() ||
			block.pos.y == std::numeric_limits<float>::max()) {
			continue;
		}

		Position intPos(static_cast<int>(block.pos.x), static_cast<int>(block.pos.y));

		BlockData* blockData = blockDataManager->getBlockData(block.type);
		if (!blockData) {
			logError("Could not find block type data for block type: {}", "CircuitValidator", (unsigned int)block.type);
		}

		std::vector<Position> takenPositions;
		bool hasOverlap = false;
		for (auto iter = (blockData->getSize(block.rotation) - Vector(1)).iter(); iter; iter++) {
			Position checkPos(intPos + *iter);
			if (occupiedPositions.count(checkPos)) {
				hasOverlap = true;
				break;
			} else {
				takenPositions.push_back(checkPos);
			}
		}

		if (hasOverlap) {
			// set the block position as undefined
			logInfo("Found overlapped block position at {} --> {}, setting to undefined position",
				   "CircuitValidator",
				   block.pos.toString(),
				   intPos.toString());
			block.pos.x = std::numeric_limits<float>::max();
			block.pos.y = std::numeric_limits<float>::max();
		} else {
			// mark all positions as occupied
			occupiedPositions.insert(takenPositions.begin(), takenPositions.end());
		}
	}

	return true;
}


// iterative dfs
template <class PreVisit, class PostVisit>
void depthFirstSearch(const std::unordered_map<block_id_t, std::vector<block_id_t>>& adj, block_id_t start, std::unordered_set<block_id_t>& visited, PreVisit preVisit, PostVisit postVisit) {
	std::stack<block_id_t> stack;
	stack.push(start);
	visited.insert(start);
	preVisit(start);
	while (!stack.empty()) {
		block_id_t node = stack.top();
		bool hasUnvisited = false;
		if (adj.count(node)) {
			for (block_id_t neighbor : adj.at(node)) {
				if (visited.count(neighbor)) continue;
				visited.insert(neighbor);
				preVisit(neighbor);
				stack.push(neighbor);
				hasUnvisited = true;
				break;
			}
		}

		if (!hasUnvisited) {
			stack.pop();
			postVisit(node);
		}
	}
}

// SCC META GRAPH TOPOLOGICAL SORT
bool CircuitValidator::handleUnpositionedBlocks() {
	// Separate components so that we can place down disconnected components independently
	std::unordered_map<block_id_t, std::vector<block_id_t>> undirectedAdj;
	undirectedAdj.reserve(parsedCircuit.blocks.size());
	for (const ParsedCircuit::ConnectionData& conn : parsedCircuit.connections) {
		block_id_t u = conn.inputBlockId;
		block_id_t v = conn.outputBlockId;
		undirectedAdj[u].push_back(v);
		undirectedAdj[v].push_back(u);
	}

	// find cc's by dfs
	std::vector<std::unordered_set<block_id_t>> components;
	std::unordered_map<block_id_t, int> blockToComponent;
	std::unordered_set<block_id_t> visited;
	for (const auto& [id, block] : parsedCircuit.blocks) {
		if (!visited.count(id)) {
			components.push_back({});
			depthFirstSearch(undirectedAdj, id, visited, [&](block_id_t node) {components.back().insert(node); blockToComponent[node] = components.size() - 1;}, [&](block_id_t) { });
		}
	}

	// preprocess connected component connections
	std::vector<std::unordered_map<block_id_t, std::vector<block_id_t>>> componentAdjs(components.size());
	for (const ParsedCircuit::ConnectionData& conn : parsedCircuit.connections) {
		if (blockDataManager->isConnectionInput(parsedCircuit.blocks.at(conn.outputBlockId).type, conn.outputId)) {
			int cc = blockToComponent[conn.inputBlockId];
			componentAdjs[cc][conn.inputBlockId].push_back(conn.outputBlockId);
		}
	}

	// precompute input connections for each block for layer placement
	std::unordered_map<block_id_t, std::vector<block_id_t>> inputConnections;
	for (const auto& conn : parsedCircuit.connections) {
		inputConnections[conn.inputBlockId].push_back(conn.outputBlockId);
	}

	const int componentSpacing = 5;
	int currentYOffset = 0;
	for (int ccIndex = 0; ccIndex < componentAdjs.size(); ++ccIndex) {
		const std::unordered_map<block_id_t, std::vector<block_id_t>>& adj = componentAdjs[ccIndex];
		// logInfo("Parsing CC {}", "", ccIndex);

		// find SCC metagraph DAG for topological sort using kosaraju's
		// gather stack that shows decreasing post visit numbers
		std::stack<block_id_t> postNumber; // highest post number node is a part of a source SCC, run on G^R
		visited.clear();
		for (block_id_t id : components[ccIndex]) {
			if (!visited.count(id)) {
				depthFirstSearch(adj, id, visited, [&](block_id_t) { }, [&](block_id_t node) {postNumber.push(node);});
			}
		}

		// make reverse graph in adjacency list
		std::unordered_map<block_id_t, std::vector<block_id_t>> revAdj;
		for (const auto& [u, adjs] : adj) {
			for (block_id_t v : adjs) {
				revAdj[v].push_back(u);
			}
		}

		// run dfs from source SCC's in reverse graph, sink SCCs of regular
		std::vector<std::vector<block_id_t>> sccs;
		visited.clear();
		while (!postNumber.empty()) {
			block_id_t node = postNumber.top();
			postNumber.pop();
			if (!visited.count(node)) {
				std::vector<block_id_t> revCC;
				depthFirstSearch(adj, node, visited, [&](block_id_t node) {revCC.push_back(node);}, [&](block_id_t) { });
				sccs.push_back(revCC);
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
		for (int sccIdx : sccOrder) {
			for (block_id_t id : sccs[sccIdx]) {
				int maxLayer = 0;
				auto it = inputConnections.find(id);
				if (it != inputConnections.end()) {
					for (block_id_t outputBlock : it->second) {
						maxLayer = std::max(maxLayer, layers[outputBlock] + 1);
					}
				}
				layers[id] = maxLayer;
			}
		}

		// place blocks in layers
		const int xSpacing = 2; // x spacing between layers
		std::unordered_map<int, int> layerYcounter;
		int maxYPlaced = parsedCircuit.minPos.dy;
		for (int sccIndex : sccOrder) {
			for (block_id_t id : sccs[sccIndex]) {
				ParsedCircuit::BlockData& block = parsedCircuit.blocks[id];
				if (block.pos.x != std::numeric_limits<float>::max() &&
					block.pos.y != std::numeric_limits<float>::max()) {
					continue;
				}

				// check block dimensions in case of custom block
				BlockData* blockData = blockDataManager->getBlockData(block.type);
				if (!blockData) {
					logError("Could not find block type data for block type: {}", "CircuitValidator", (unsigned int)block.type);
				}
				Vector blockSize = blockData->getSize(block.rotation);

				const int layer = layers[id];
				const int x = static_cast<int>(parsedCircuit.minPos.dx) + (layer - 1) * xSpacing;

				// find first available Y position in current layer column
				int y;
				if (layerYcounter.find(x) != layerYcounter.end()) {
					y = layerYcounter[x];
				} else {
					y = static_cast<int>(parsedCircuit.minPos.dy) + currentYOffset;
				}

				std::vector<Position> takenPositions;
				bool canPlace;
				do {
					canPlace = true;
					takenPositions.clear();
					std::vector<Position> takenPositions;
					canPlace = true;
					for (auto iter = (blockSize - Vector(1)).iter(); iter; iter++) {
						Position checkPos(Position(x, y) + *iter);
						if (occupiedPositions.count(checkPos)) {
							canPlace = false;
							break;
						} else {
							takenPositions.push_back(checkPos);
						}
					}
					if (!canPlace) ++y;
				} while (!canPlace);

				// mark the found positions
				occupiedPositions.insert(takenPositions.begin(), takenPositions.end());

				block.pos = FPosition(static_cast<float>(x), static_cast<float>(y));
				layerYcounter[x] = y + blockSize.dy;

				float blockMaxX = x + blockSize.dx - 1;
				float blockMaxY = y + blockSize.dy - 1;
				parsedCircuit.minPos.dx = std::fmin(parsedCircuit.minPos.dx, static_cast<float>(x));
				parsedCircuit.minPos.dy = std::fmin(parsedCircuit.minPos.dy, static_cast<float>(y));
				parsedCircuit.maxPos.dx = std::fmax(parsedCircuit.maxPos.dx, blockMaxX);
				parsedCircuit.maxPos.dy = std::fmax(parsedCircuit.maxPos.dy, blockMaxY);

				maxYPlaced = std::max(maxYPlaced, static_cast<int>(blockMaxY));
			}
		}
		currentYOffset = componentSpacing + maxYPlaced;
	}

	return true;
}
