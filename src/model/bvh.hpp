#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "../utils/sort.hpp"

#include <vector>
#include <algorithm>
#include <stack>

namespace nugiEngine {
  const glm::vec3 eps(0.0001f);

  // Axis-aligned bounding box.
  struct Aabb {
    alignas(16) glm::vec3 min = {FLT_MAX, FLT_MAX, FLT_MAX};
    alignas(16) glm::vec3 max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    int longestAxis() {
      float x = abs(max[0] - min[0]);
      float y = abs(max[1] - min[1]);
      float z = abs(max[2] - min[2]);

      int longest = 0;
      if (y > x && y > z) {
        longest = 1;
      }

      if (z > x && z > y) {
        longest = 2;
      }

      return longest;
    }

    int randomAxis() {
      return rand() % 3;
    }
  };

  // Utility structure to keep track of the initial triangle index in the triangles array while sorting.
  struct TriangleBoundBox {
    uint32_t index;
    Triangle t;
  };

  // Intermediate BvhNode structure needed for constructing Bvh.
  struct BvhItemBuild {
    Aabb box;
    int index = -1; // index refers to the index in the final array of nodes. Used for sorting a flattened Bvh.
    int leftNodeIndex = -1;
    int rightNodeIndex = -1;
    std::vector<TriangleBoundBox> objects;

    BvhNode getGpuModel() {
      bool leaf = leftNodeIndex == -1 && rightNodeIndex == -1;

      BvhNode node{};
      node.minimum = box.min;
      node.maximum = box.max;
      node.leftNode = leftNodeIndex;
      node.rightNode = rightNodeIndex;

      if (leaf) {
        node.objIndex = objects[0].index;
      }

      return node;
    }
  };

  bool nodeCompare(BvhItemBuild &a, BvhItemBuild &b) {
    return a.index < b.index;
  }

  Aabb surroundingBox(Aabb box0, Aabb box1) {
    return {glm::min(box0.min, box1.min), glm::max(box0.max, box1.max)};
  }

  Aabb objectBoundingBox(Triangle &t) {
    // Need to add eps to correctly construct an AABB for flat objects like planes.
    return {glm::min(glm::min(t.point0, t.point1), t.point2) - eps, glm::max(glm::max(t.point0, t.point1), t.point2) + eps};
  }

  Aabb objectListBoundingBox(std::vector<TriangleBoundBox> &objects) {
    Aabb tempBox;
    Aabb outputBox;
    bool firstBox = true;

    for (auto &object : objects) {
      tempBox = objectBoundingBox(object.t);
      outputBox = firstBox ? tempBox : surroundingBox(outputBox, tempBox);
      firstBox = false;
    }

    return outputBox;
  }

  inline bool boxCompare(Triangle &a, Triangle &b, int axis) {
    Aabb boxA = objectBoundingBox(a);
    Aabb boxB = objectBoundingBox(b);

    return boxA.min[axis] < boxB.min[axis];
  }

  bool boxXCompare(TriangleBoundBox a, TriangleBoundBox b) {
    return boxCompare(a.t, b.t, 0);
  }

  bool boxYCompare(TriangleBoundBox a, TriangleBoundBox b) {
    return boxCompare(a.t, b.t, 1);
  }

  bool boxZCompare(TriangleBoundBox a, TriangleBoundBox b) {
    return boxCompare(a.t, b.t, 2);
  }

  // Since GPU can't deal with tree structures we need to create a flattened BVH.
  // Stack is used instead of a tree.
  std::vector<BvhNode> createBvh(const std::vector<TriangleBoundBox> &srcObjects) {
    std::vector<BvhItemBuild> intermediate;
    int nodeCounter = 0;
    std::stack<BvhItemBuild> nodeStack;

    BvhItemBuild root;
    root.index = nodeCounter;
    root.objects = srcObjects;
    nodeCounter++;
    nodeStack.push(root);

    while (!nodeStack.empty()) {
      BvhItemBuild currentNode = nodeStack.top();
      nodeStack.pop();

      currentNode.box = objectListBoundingBox(currentNode.objects);

      int axis = currentNode.box.randomAxis();
      auto comparator = (axis == 0)   ? boxXCompare
                        : (axis == 1) ? boxYCompare
                                      : boxZCompare;

      size_t objectSpan = currentNode.objects.size();
      std::sort(currentNode.objects.begin(), currentNode.objects.end(), comparator);

      if (objectSpan <= 1) {
        intermediate.push_back(currentNode);
        continue;
      } else {
        auto mid = objectSpan / 2;

        BvhItemBuild leftNode;
        leftNode.index = nodeCounter;
        for (int i = 0; i < mid; i++) {
          leftNode.objects.push_back(currentNode.objects[i]);
        }

        nodeCounter++;
        nodeStack.push(leftNode);

        BvhItemBuild rightNode;
        rightNode.index = nodeCounter;
        for (int i = mid; i < objectSpan; i++) {
          rightNode.objects.push_back(currentNode.objects[i]);
        }

        nodeCounter++;
        nodeStack.push(rightNode);

        currentNode.leftNodeIndex = leftNode.index;
        currentNode.rightNodeIndex = rightNode.index;
        intermediate.push_back(currentNode);
      }
    }

    std::sort(intermediate.begin(), intermediate.end(), nodeCompare);

    std::vector<BvhNode> output;
    output.reserve(intermediate.size());

    for (int i = 0; i < intermediate.size(); i++) {
      output.emplace_back(intermediate[i].getGpuModel());
    }

    return output;
  }
}// namespace nugiEngine 
