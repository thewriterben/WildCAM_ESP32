#ifndef BLOCKCHAIN_MERKLE_TREE_H
#define BLOCKCHAIN_MERKLE_TREE_H

#include <Arduino.h>
#include <vector>

/**
 * @brief Efficient Merkle Tree implementation for blockchain data verification
 * 
 * Optimized for ESP32 memory constraints while providing cryptographic proof
 * of data integrity for wildlife monitoring transactions.
 */
class MerkleTree {
public:
    /**
     * @brief Merkle tree node structure
     */
    struct Node {
        String hash;                // Node hash value
        bool isLeaf;               // True if leaf node
        uint16_t leftChild;        // Index of left child (-1 if none)
        uint16_t rightChild;       // Index of right child (-1 if none)
        uint16_t parent;           // Index of parent node (-1 if root)
        
        Node() : hash(""), isLeaf(false), leftChild(0xFFFF), rightChild(0xFFFF), parent(0xFFFF) {}
        Node(const String& h, bool leaf = true) : hash(h), isLeaf(leaf), leftChild(0xFFFF), rightChild(0xFFFF), parent(0xFFFF) {}
    };

    /**
     * @brief Merkle proof structure for verification
     */
    struct MerkleProof {
        String targetHash;              // Hash being proven
        std::vector<String> siblings;   // Sibling hashes in path to root
        std::vector<bool> isLeftSibling; // Direction indicators
        String rootHash;               // Expected root hash
        bool isValid;                  // Proof validity flag
        
        MerkleProof() : targetHash(""), rootHash(""), isValid(false) {}
    };

public:
    MerkleTree();
    ~MerkleTree();

    /**
     * @brief Build merkle tree from list of data hashes
     * @param dataHashes Vector of SHA-256 hashes (leaf nodes)
     * @return true if tree built successfully
     */
    bool buildTree(const std::vector<String>& dataHashes);

    /**
     * @brief Get the root hash of the tree
     * @return Root hash string, empty if tree not built
     */
    String getRootHash() const;

    /**
     * @brief Generate merkle proof for a specific data hash
     * @param targetHash Hash to generate proof for
     * @return Merkle proof structure
     */
    MerkleProof generateProof(const String& targetHash) const;

    /**
     * @brief Verify a merkle proof against root hash
     * @param proof Merkle proof to verify
     * @return true if proof is valid
     */
    bool verifyProof(const MerkleProof& proof) const;

    /**
     * @brief Add a new leaf to existing tree (rebuilds tree)
     * @param dataHash New data hash to add
     * @return true if successfully added
     */
    bool addLeaf(const String& dataHash);

    /**
     * @brief Update existing leaf hash (rebuilds tree)
     * @param oldHash Hash to replace
     * @param newHash New hash value
     * @return true if successfully updated
     */
    bool updateLeaf(const String& oldHash, const String& newHash);

    /**
     * @brief Check if tree contains a specific hash
     * @param hash Hash to search for
     * @return true if hash is in tree
     */
    bool containsHash(const String& hash) const;

    /**
     * @brief Get all leaf hashes in the tree
     * @return Vector of leaf hashes
     */
    std::vector<String> getLeafHashes() const;

    /**
     * @brief Get tree depth (number of levels)
     * @return Tree depth, 0 if empty
     */
    uint8_t getTreeDepth() const;

    /**
     * @brief Get number of leaf nodes
     * @return Leaf count
     */
    uint16_t getLeafCount() const;

    /**
     * @brief Clear the tree
     */
    void clear();

    /**
     * @brief Check if tree is empty
     * @return true if no nodes in tree
     */
    bool isEmpty() const { return nodes_.empty(); }

    /**
     * @brief Export tree structure to JSON
     * @return JSON representation of tree
     */
    String toJSON() const;

    /**
     * @brief Import tree structure from JSON
     * @param json JSON tree representation
     * @return true if successfully imported
     */
    bool fromJSON(const String& json);

    /**
     * @brief Get memory usage statistics
     * @param nodeCount Number of nodes in tree
     * @param memoryBytes Approximate memory usage
     */
    void getMemoryStats(uint16_t& nodeCount, size_t& memoryBytes) const;

    /**
     * @brief Validate tree structure integrity
     * @return true if tree structure is valid
     */
    bool validateTree() const;

private:
    std::vector<Node> nodes_;      // All tree nodes (breadth-first order)
    uint16_t rootIndex_;           // Index of root node
    uint8_t treeDepth_;            // Current tree depth
    bool isBuilt_;                 // True if tree is built and valid

    /**
     * @brief Find index of node with specific hash
     * @param hash Hash to search for
     * @param leafOnly True to search only leaf nodes
     * @return Node index, 0xFFFF if not found
     */
    uint16_t findNodeIndex(const String& hash, bool leafOnly = false) const;

    /**
     * @brief Build internal tree structure from leaf nodes
     * @return true if successful
     */
    bool buildInternalNodes();

    /**
     * @brief Calculate parent hash from two child hashes
     * @param leftHash Left child hash
     * @param rightHash Right child hash
     * @return Combined parent hash
     */
    String calculateParentHash(const String& leftHash, const String& rightHash) const;

    /**
     * @brief Get path from leaf to root
     * @param leafIndex Index of leaf node
     * @param path Vector to store path indices
     * @return true if path found
     */
    bool getPathToRoot(uint16_t leafIndex, std::vector<uint16_t>& path) const;

    /**
     * @brief Validate node relationships
     * @param nodeIndex Node to validate
     * @return true if node relationships are valid
     */
    bool validateNode(uint16_t nodeIndex) const;

    /**
     * @brief Calculate tree depth for given number of leaves
     * @param leafCount Number of leaf nodes
     * @return Required tree depth
     */
    uint8_t calculateRequiredDepth(uint16_t leafCount) const;

    /**
     * @brief Check if index is valid
     * @param index Node index to check
     * @return true if index is valid
     */
    bool isValidIndex(uint16_t index) const {
        return index != 0xFFFF && index < nodes_.size();
    }
};

#endif // BLOCKCHAIN_MERKLE_TREE_H