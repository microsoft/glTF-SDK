// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Document.h>

#include <queue>
#include <stack>

namespace Microsoft
{
    namespace glTF
    {
        enum TraversalAlgorithm
        {
            DepthFirst,
            BreadthFirst
        };

        namespace Detail
        {
            template<typename Fn>
            void TraverseDepthFirst(const Node& node, const Document& gltfDocument, Fn& fn)
            {
                struct StackValue
                {
                    StackValue(const Node& node, const Node* nodeParent = nullptr) :
                        m_node(node),
                        m_nodeParent(nodeParent),
                        m_childIndex(0UL),
                        m_childCount(node.children.size())
                    {
                    }

                    bool HasChildren() const
                    {
                        return m_childIndex < m_childCount;
                    }

                    const Node& m_node;
                    const Node* const m_nodeParent;

                    size_t m_childIndex;
                    const size_t m_childCount;

                    bool m_visited = false;
                };

                typedef std::stack<StackValue> Stack;

                Stack stack;
                stack.emplace(node);

                do
                {
                    auto& currentNodeValue = stack.top();
                    auto& currentNode = currentNodeValue.m_node;

                    if (!currentNodeValue.m_visited)
                    {
                        currentNodeValue.m_visited = true;
                        fn(currentNode, currentNodeValue.m_nodeParent);
                    }

                    if (currentNodeValue.HasChildren())
                    {
                        stack.emplace(gltfDocument.nodes.Get(currentNode.children[currentNodeValue.m_childIndex++]), &currentNode);
                    }
                    else
                    {
                        stack.pop();
                    }
                } while (!stack.empty());
            }

            template<typename Fn>
            void TraverseBreadthFirst(const Node& node, const Document& gltfDocument, Fn& fn)
            {
                struct QueueValue
                {
                    QueueValue(const Node& node, const Node* nodeParent = nullptr) :
                        m_node(node),
                        m_nodeParent(nodeParent)
                    {
                    }

                    const Node& m_node;
                    const Node* const m_nodeParent;
                };

                typedef std::queue<QueueValue> Queue;

                Queue queue;
                queue.emplace(node);

                do
                {
                    auto& currentNodeValue = queue.front();
                    auto& currentNode = currentNodeValue.m_node;

                    fn(currentNode, currentNodeValue.m_nodeParent);

                    for (const auto& childId : currentNode.children)
                    {
                        queue.emplace(gltfDocument.nodes[childId], &currentNode);
                    }

                    queue.pop();
                } while (!queue.empty());
            }

            // Used for tag dispatching - call TraverseDepthFirst or TraverseBreadthFirst without an if-statement
            template<TraversalAlgorithm>
            struct TraversalAlgorithmTag {};

            template<typename Fn>
            void TraverseNode(TraversalAlgorithmTag<DepthFirst>, const Node& node, const Document& gltfDocument, Fn& fn)
            {
                TraverseDepthFirst(node, gltfDocument, fn);
            }

            template<typename Fn>
            void TraverseNode(TraversalAlgorithmTag<BreadthFirst>, const Node& node, const Document& gltfDocument, Fn& fn)
            {
                TraverseBreadthFirst(node, gltfDocument, fn);
            }
        }

        const size_t DefaultSceneIndex = std::numeric_limits<size_t>::max();

        template<TraversalAlgorithm Algorithm = DepthFirst, typename Fn>
        void Traverse(const Document& gltfDocument, size_t sceneIndex, Fn&& fn)
        {
            // The fn parameter is a forwarding reference (can be a rvalue or lvalue). This copy is necessary
            // as TraverseNode's fn parameter is a non-const reference (which can only be bound to an lvalue)
            Fn fnCopy = std::forward<Fn>(fn);

            const Scene& scene = (sceneIndex == DefaultSceneIndex) ?
                gltfDocument.GetDefaultScene() :
                gltfDocument.scenes[sceneIndex];

            for (const auto& nodeId : scene.nodes)
            {
                Detail::TraverseNode(Detail::TraversalAlgorithmTag<Algorithm>(), gltfDocument.nodes.Get(nodeId), gltfDocument, fnCopy);
            }
        }
    }
}
