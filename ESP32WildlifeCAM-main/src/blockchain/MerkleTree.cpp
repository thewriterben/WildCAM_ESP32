#include "MerkleTree.h"
#include "HashingService.h"
#include "../utils/logger.h"
#include <ArduinoJson.h>

// Static hashing service for merkle tree operations
static HashingService merkleHashingService;

MerkleTree::MerkleTree() : rootIndex_(0xFFFF), treeDepth_(0), isBuilt_(false) {
    if (!merkleHashingService.isInitialized()) {
        merkleHashingService.initialize();
    }
}

MerkleTree::~MerkleTree() {
    clear();
}

bool MerkleTree::buildTree(const std::vector<String>& dataHashes) {
    if (dataHashes.empty()) {
        Logger::log("MerkleTree: Cannot build tree with empty data hashes");
        return false;
    }

    clear();

    // Add leaf nodes
    for (const String& hash : dataHashes) {
        if (hash.length() != 64) {
            Logger::log("MerkleTree: Invalid hash length: " + String(hash.length()));
            return false;
        }
        nodes_.push_back(Node(hash, true));
    }

    // Build internal nodes
    if (!buildInternalNodes()) {
        Logger::log("MerkleTree: Failed to build internal nodes");
        return false;
    }

    isBuilt_ = true;
    Logger::log("MerkleTree: Built tree with " + String(getLeafCount()) + 
                " leaves and depth " + String(treeDepth_));
    return true;
}

String MerkleTree::getRootHash() const {
    if (!isBuilt_ || rootIndex_ == 0xFFFF || rootIndex_ >= nodes_.size()) {
        return "";
    }
    return nodes_[rootIndex_].hash;
}

MerkleTree::MerkleProof MerkleTree::generateProof(const String& targetHash) const {
    MerkleProof proof;
    proof.targetHash = targetHash;
    proof.rootHash = getRootHash();

    if (!isBuilt_ || targetHash.length() != 64) {
        return proof;
    }

    // Find the leaf node with target hash
    uint16_t leafIndex = findNodeIndex(targetHash, true);
    if (leafIndex == 0xFFFF) {
        Logger::log("MerkleTree: Target hash not found in tree");
        return proof;
    }

    // Get path from leaf to root
    std::vector<uint16_t> pathToRoot;
    if (!getPathToRoot(leafIndex, pathToRoot)) {
        Logger::log("MerkleTree: Failed to get path to root");
        return proof;
    }

    // Build proof by collecting sibling hashes
    for (size_t i = 0; i < pathToRoot.size() - 1; i++) {
        uint16_t currentIndex = pathToRoot[i];
        uint16_t parentIndex = pathToRoot[i + 1];
        
        if (!isValidIndex(currentIndex) || !isValidIndex(parentIndex)) {
            continue;
        }

        const Node& parent = nodes_[parentIndex];
        
        // Find sibling
        uint16_t siblingIndex = 0xFFFF;
        if (parent.leftChild == currentIndex) {
            siblingIndex = parent.rightChild;
            proof.isLeftSibling.push_back(false); // Current is left, sibling is right
        } else if (parent.rightChild == currentIndex) {
            siblingIndex = parent.leftChild;
            proof.isLeftSibling.push_back(true);  // Current is right, sibling is left
        }

        if (isValidIndex(siblingIndex)) {
            proof.siblings.push_back(nodes_[siblingIndex].hash);
        }
    }

    proof.isValid = !proof.siblings.empty();
    return proof;
}

bool MerkleTree::verifyProof(const MerkleProof& proof) const {
    if (!proof.isValid || proof.siblings.size() != proof.isLeftSibling.size()) {
        return false;
    }

    String currentHash = proof.targetHash;

    // Reconstruct root hash using proof
    for (size_t i = 0; i < proof.siblings.size(); i++) {
        String siblingHash = proof.siblings[i];
        bool isLeftSibling = proof.isLeftSibling[i];

        if (isLeftSibling) {
            // Sibling is on left, current is on right
            currentHash = calculateParentHash(siblingHash, currentHash);
        } else {
            // Sibling is on right, current is on left
            currentHash = calculateParentHash(currentHash, siblingHash);
        }
    }

    return currentHash.equalsIgnoreCase(proof.rootHash);
}

bool MerkleTree::addLeaf(const String& dataHash) {
    if (dataHash.length() != 64) {
        return false;
    }

    // Get current leaf hashes
    std::vector<String> currentLeaves = getLeafHashes();
    currentLeaves.push_back(dataHash);

    // Rebuild tree
    return buildTree(currentLeaves);
}

bool MerkleTree::updateLeaf(const String& oldHash, const String& newHash) {
    if (oldHash.length() != 64 || newHash.length() != 64) {
        return false;
    }

    // Get current leaf hashes and replace old with new
    std::vector<String> currentLeaves = getLeafHashes();
    bool found = false;

    for (String& hash : currentLeaves) {
        if (hash.equalsIgnoreCase(oldHash)) {
            hash = newHash;
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    // Rebuild tree
    return buildTree(currentLeaves);
}

bool MerkleTree::containsHash(const String& hash) const {
    return findNodeIndex(hash, true) != 0xFFFF;
}

std::vector<String> MerkleTree::getLeafHashes() const {
    std::vector<String> leafHashes;
    
    for (const Node& node : nodes_) {
        if (node.isLeaf) {
            leafHashes.push_back(node.hash);
        }
    }
    
    return leafHashes;
}

uint8_t MerkleTree::getTreeDepth() const {
    return treeDepth_;
}

uint16_t MerkleTree::getLeafCount() const {
    uint16_t count = 0;
    for (const Node& node : nodes_) {
        if (node.isLeaf) {
            count++;
        }
    }
    return count;
}

void MerkleTree::clear() {
    nodes_.clear();
    rootIndex_ = 0xFFFF;
    treeDepth_ = 0;
    isBuilt_ = false;
}

String MerkleTree::toJSON() const {
    DynamicJsonDocument doc(4096);
    
    doc["isBuilt"] = isBuilt_;
    doc["treeDepth"] = treeDepth_;
    doc["rootIndex"] = rootIndex_;
    doc["nodeCount"] = nodes_.size();
    
    if (isBuilt_) {
        doc["rootHash"] = getRootHash();
    }
    
    JsonArray nodesArray = doc.createNestedArray("nodes");
    for (size_t i = 0; i < nodes_.size(); i++) {
        JsonObject nodeObj = nodesArray.createNestedObject();
        nodeObj["index"] = i;
        nodeObj["hash"] = nodes_[i].hash;
        nodeObj["isLeaf"] = nodes_[i].isLeaf;
        nodeObj["leftChild"] = (nodes_[i].leftChild == 0xFFFF) ? -1 : nodes_[i].leftChild;
        nodeObj["rightChild"] = (nodes_[i].rightChild == 0xFFFF) ? -1 : nodes_[i].rightChild;
        nodeObj["parent"] = (nodes_[i].parent == 0xFFFF) ? -1 : nodes_[i].parent;
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool MerkleTree::fromJSON(const String& json) {
    DynamicJsonDocument doc(4096);
    
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        return false;
    }
    
    clear();
    
    isBuilt_ = doc["isBuilt"];
    treeDepth_ = doc["treeDepth"];
    rootIndex_ = doc["rootIndex"];
    
    JsonArray nodesArray = doc["nodes"];
    nodes_.resize(nodesArray.size());
    
    for (size_t i = 0; i < nodesArray.size(); i++) {
        JsonObject nodeObj = nodesArray[i];
        
        nodes_[i].hash = nodeObj["hash"].as<String>();
        nodes_[i].isLeaf = nodeObj["isLeaf"];
        nodes_[i].leftChild = (nodeObj["leftChild"] == -1) ? 0xFFFF : nodeObj["leftChild"];
        nodes_[i].rightChild = (nodeObj["rightChild"] == -1) ? 0xFFFF : nodeObj["rightChild"];
        nodes_[i].parent = (nodeObj["parent"] == -1) ? 0xFFFF : nodeObj["parent"];
    }
    
    return validateTree();
}

void MerkleTree::getMemoryStats(uint16_t& nodeCount, size_t& memoryBytes) const {
    nodeCount = nodes_.size();
    memoryBytes = sizeof(MerkleTree) + (nodes_.size() * sizeof(Node));
    
    // Add estimated string memory usage
    for (const Node& node : nodes_) {
        memoryBytes += node.hash.length();
    }
}

bool MerkleTree::validateTree() const {
    if (!isBuilt_) {
        return !nodes_.empty(); // Empty tree is valid if not built
    }
    
    if (nodes_.empty() || rootIndex_ >= nodes_.size()) {
        return false;
    }
    
    // Validate each node
    for (uint16_t i = 0; i < nodes_.size(); i++) {
        if (!validateNode(i)) {
            return false;
        }
    }
    
    return true;
}

// Private method implementations

uint16_t MerkleTree::findNodeIndex(const String& hash, bool leafOnly) const {
    for (uint16_t i = 0; i < nodes_.size(); i++) {
        if (nodes_[i].hash.equalsIgnoreCase(hash)) {
            if (!leafOnly || nodes_[i].isLeaf) {
                return i;
            }
        }
    }
    return 0xFFFF;
}

bool MerkleTree::buildInternalNodes() {
    if (nodes_.empty()) {
        return false;
    }
    
    uint16_t leafCount = getLeafCount();
    treeDepth_ = calculateRequiredDepth(leafCount);
    
    std::vector<uint16_t> currentLevel;
    
    // Start with leaf nodes
    for (uint16_t i = 0; i < nodes_.size(); i++) {
        currentLevel.push_back(i);
    }
    
    // Build tree level by level
    while (currentLevel.size() > 1) {
        std::vector<uint16_t> nextLevel;
        
        for (size_t i = 0; i < currentLevel.size(); i += 2) {
            uint16_t leftIndex = currentLevel[i];
            uint16_t rightIndex = (i + 1 < currentLevel.size()) ? currentLevel[i + 1] : leftIndex;
            
            // Create parent node
            String leftHash = nodes_[leftIndex].hash;
            String rightHash = nodes_[rightIndex].hash;
            String parentHash = calculateParentHash(leftHash, rightHash);
            
            Node parentNode(parentHash, false);
            parentNode.leftChild = leftIndex;
            parentNode.rightChild = rightIndex;
            
            uint16_t parentIndex = nodes_.size();
            nodes_.push_back(parentNode);
            
            // Update child nodes to point to parent
            nodes_[leftIndex].parent = parentIndex;
            if (rightIndex != leftIndex) {
                nodes_[rightIndex].parent = parentIndex;
            }
            
            nextLevel.push_back(parentIndex);
        }
        
        currentLevel = nextLevel;
    }
    
    if (!currentLevel.empty()) {
        rootIndex_ = currentLevel[0];
        return true;
    }
    
    return false;
}

String MerkleTree::calculateParentHash(const String& leftHash, const String& rightHash) const {
    return merkleHashingService.calculateCombinedHash(leftHash, rightHash);
}

bool MerkleTree::getPathToRoot(uint16_t leafIndex, std::vector<uint16_t>& path) const {
    if (!isValidIndex(leafIndex) || !nodes_[leafIndex].isLeaf) {
        return false;
    }
    
    path.clear();
    uint16_t currentIndex = leafIndex;
    
    while (currentIndex != 0xFFFF) {
        path.push_back(currentIndex);
        
        if (currentIndex == rootIndex_) {
            break;
        }
        
        currentIndex = nodes_[currentIndex].parent;
        
        // Prevent infinite loops
        if (path.size() > treeDepth_ + 1) {
            return false;
        }
    }
    
    return !path.empty() && path.back() == rootIndex_;
}

bool MerkleTree::validateNode(uint16_t nodeIndex) const {
    if (!isValidIndex(nodeIndex)) {
        return false;
    }
    
    const Node& node = nodes_[nodeIndex];
    
    // Validate hash length
    if (node.hash.length() != 64) {
        return false;
    }
    
    // Validate leaf nodes
    if (node.isLeaf) {
        // Leaf nodes should not have children
        return node.leftChild == 0xFFFF && node.rightChild == 0xFFFF;
    }
    
    // Validate internal nodes
    if (!isValidIndex(node.leftChild)) {
        return false;
    }
    
    // Right child can be same as left for odd number of nodes
    if (node.rightChild != 0xFFFF && !isValidIndex(node.rightChild)) {
        return false;
    }
    
    // Validate parent-child relationships
    if (isValidIndex(node.leftChild)) {
        if (nodes_[node.leftChild].parent != nodeIndex) {
            return false;
        }
    }
    
    if (isValidIndex(node.rightChild) && node.rightChild != node.leftChild) {
        if (nodes_[node.rightChild].parent != nodeIndex) {
            return false;
        }
    }
    
    return true;
}

uint8_t MerkleTree::calculateRequiredDepth(uint16_t leafCount) const {
    if (leafCount <= 1) {
        return 1;
    }
    
    uint8_t depth = 0;
    uint16_t count = leafCount;
    
    while (count > 1) {
        count = (count + 1) / 2;  // Round up division
        depth++;
    }
    
    return depth + 1;  // Add 1 for leaf level
}