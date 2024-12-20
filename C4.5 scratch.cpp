#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <set>
#include <algorithm>

struct Node {
    bool isLeaf;
    int label;  // for leaf nodes
    int attribute; // for internal nodes
    std::map<int, Node*> children;

    Node() : isLeaf(false), label(-1), attribute(-1) {}
};

class C45Classifier {
public:
    Node* buildTree(const std::vector<std::vector<int>>& data, const std::vector<int>& labels, const std::set<int>& attributes) {
        Node* node = new Node();
        
        // Check if all labels are the same (make leaf)
        if (std::all_of(labels.begin(), labels.end(), [&](int l) { return l == labels[0]; })) {
            node->isLeaf = true;
            node->label = labels[0];
            return node;
        }

        // If no more attributes to split on, make leaf with majority label
        if (attributes.empty()) {
            node->isLeaf = true;
            node->label = majorityLabel(labels);
            return node;
        }

        // Find best attribute based on gain ratio
        int bestAttr = bestAttribute(data, labels, attributes);
        node->attribute = bestAttr;

        // Split data by attribute values and build subtrees
        std::map<int, std::vector<std::vector<int>>> subsets;
        std::map<int, std::vector<int>> subLabels;

        for (int i = 0; i < data.size(); ++i) {
            int val = data[i][bestAttr];
            subsets[val].push_back(data[i]);
            subLabels[val].push_back(labels[i]);
        }

        std::set<int> newAttributes = attributes;
        newAttributes.erase(bestAttr);

        for (auto& pair : subsets) {
            int attrVal = pair.first;
            node->children[attrVal] = buildTree(pair.second, subLabels[attrVal], newAttributes);
        }
        
        return node;
    }

    int classify(const std::vector<int>& instance, Node* node) {
        if (node->isLeaf) return node->label;
        int attrVal = instance[node->attribute];
        if (node->children.count(attrVal)) {
            return classify(instance, node->children[attrVal]);
        } else {
            return -1; // handle missing branches (for simplicity)
        }
    }

private:
    double entropy(const std::vector<int>& labels) {
        std::map<int, int> freq;
        for (int label : labels) freq[label]++;
        double ent = 0.0;
        for (auto [labels, count] : freq) {
            double p = (double)count / labels.size();
            ent -= p * std::log2(p);
        }
        return ent;
    }

    double gainRatio(const std::vector<std::vector<int>>& data, const std::vector<int>& labels, int attr) {
        double baseEntropy = entropy(labels);
        std::map<int, std::vector<int>> subsets;
        for (int i = 0; i < data.size(); ++i) {
            int val = data[i][attr];
            subsets[val].push_back(labels[i]);
        }

        double splitInfo = 0.0, newEntropy = 0.0;
        for (auto& [val, subLabels] : subsets) {
            double weight = (double)subLabels.size() / labels.size();
            newEntropy += weight * entropy(subLabels);
            splitInfo -= weight * std::log2(weight);
        }
        double gain = baseEntropy - newEntropy;
        return (splitInfo == 0) ? 0 : gain / splitInfo;
    }

    int bestAttribute(const std::vector<std::vector<int>>& data, const std::vector<int>& labels, const std::set<int>& attributes) {
        double bestGainRatio = -1;
        int bestAttr = -1;
        for (int attr : attributes) {
            double gr = gainRatio(data, labels, attr);
            if (gr > bestGainRatio) {
                bestGainRatio = gr;
                bestAttr = attr;
            }
        }
        return bestAttr;
    }

    int majorityLabel(const std::vector<int>& labels) {
        std::map<int, int> freq;
        for (int label : labels) freq[label]++;
        return std::max_element(freq.begin(), freq.end(),
                                [](const auto& a, const auto& b) { return a.second < b.second; })->first;
    }
};

int main() {
    // Example dataset: [attribute1, attribute2, ...], label
    std::vector<std::vector<int>> data = {
        {1, 0, 0}, {1, 1, 1}, {0, 0, 0}, {0, 1, 1}, {1, 0, 1}
    };
    std::vector<int> labels = {0, 1, 0, 1, 1};
    std::set<int> attributes = {0, 1, 2};  // attribute indices

    C45Classifier classifier;
    Node* root = classifier.buildTree(data, labels, attributes);

    // Test instance
    std::vector<int> testInstance = {1, 0, 0};
    int prediction = classifier.classify(testInstance, root);
    std::cout << "Prediction: " << prediction << std::endl;

    return 0;
}
