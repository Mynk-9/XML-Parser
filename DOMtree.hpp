//    Copyright 2020 Mayank Mathur (mynk-9 at Github)

//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at

//        http://www.apache.org/licenses/LICENSE-2.0

//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#ifndef DOM_PARSER_DOM_TREE
#define DOM_PARSER_DOM_TREE

#include <string>
#include <vector>
#include <queue>

#include "DOMnode.hpp"

namespace dom_parser
{
    class DOMtree
    {
    private:
        std::vector<DOMnode> nodes;
        int nodes_counter = 0;

        std::queue<DOMnodeUID> vacantUIDs;
        DOMnode deletedNode = DOMnode("", -1, -1);

        /**
        * @brief   Generates a new DOMnodeUID.
        */
        inline DOMnodeUID generateUID()
        {
            ++nodes_counter;

            if (nodes_counter == 1) // added to fix a possible bug in which multiple root DOM
                return 0;           // elements could occur if once root node is deleted.
            if (vacantUIDs.empty())
                return nodes_counter - 1;

            DOMnodeUID uid = vacantUIDs.front();
            vacantUIDs.pop();
            return uid;
        }

        /**
         * @brief   Checks existance of a node with given UID.
         * @param   dom_parser::DOMnodeUID node     The node UID.
         * @return  true or false accordingly
         */
        inline bool checkNodeExistance(DOMnodeUID node)
        {
            return (node < nodes.size() && nodes[node].getUID() != -1);
        }

    public:
        /**
         * @brief   Constructor of empty tree. @a Depriciated @a method - beware
         *          of undefined behaviour when used this contructor without proper
         *          knowledge.
         */
        DOMtree() {}

        /**
         * @brief   Constructor of the tree with an initial root node.
         * @param   std::string rootName    Name of the root node.
         */
        DOMtree(std::string root)
        {
            DOMnode _root(root, generateUID(), -1);
            nodes.push_back(_root);
        }

        /**
         * @brief   Adds a node within the tree.
         * @param   dom_parser::DOMnodeUID parent   Parent node UID.
         * @param   std::string            tagName  Tag name of the node.
         * @return  DOMnodeID   if node added succefully
         *          -1          if parent does not exist
         */
        DOMnodeUID addNode(DOMnodeUID parent, std::string tagName)
        {
            if (!checkNodeExistance(parent))
                return -1;

            DOMnodeUID UID = generateUID();
            DOMnode node(tagName, UID, parent);

            if (UID < nodes.size())    // If a vacant space if filled then use [] operator
                nodes[UID] = node;     // otherwise push_back to the end of the vector.
            else                       // Condition added to make sure that UID and iterator
                nodes.push_back(node); // position is consistent.

            nodes[parent].addChild(UID);

            return UID;
        }

        /**
         * @brief   Adds a inner-data node within the tree under another node.
         * @param   dom_parser::DOMnodeUID parent   Parent node UID.
         * @param   std::string            data     inner-data
         * @return  DOMnodeID   if node added succefully
         *          -1          if parent does not exist
         */
        DOMnodeUID addInnerDataNode(DOMnodeUID parent, std::string data)
        {
            if (!checkNodeExistance(parent))
                return -1;

            DOMnodeUID UID = generateUID();
            DOMnode node(UID, parent, data);

            if (UID < nodes.size())    // If a vacant space if filled then use [] operator
                nodes[UID] = node;     // otherwise push_back to the end of the vector.
            else                       // Condition added to make sure that UID and iterator
                nodes.push_back(node); // position is consistent.

            nodes[parent].addChild(UID);

            return UID;
        }

        /**
         * @brief   Returns a reference to the node with given UID.
         * @param  dom_parser::DOMnodeUID  node    UID of the node.
         */
        inline DOMnode &getNode(DOMnodeUID node)
        {
            return nodes[node];
        }

        /**
         * @brief   Moves a whole subtree from one parent node to another.
         * @param   dom_parser::DOMnodeUID subtree_root     Subtree root node UID.
         * @param   dom_parser::DOMnodeUID new_parent       New parent node of the subtree.
         * @return  true    if moving is successful
         *          false   if moving is unsuccessful due to problem in input.
         */
        bool moveSubtree(DOMnodeUID subtree_root, DOMnodeUID new_parent)
        {
            if (!checkNodeExistance(subtree_root) || !checkNodeExistance(new_parent))
                return false;
            if (subtree_root == 0)
                return false;
            if (subtree_root == new_parent)
                return false;

            std::vector<DOMnodeUID> ancestors = getAncestorList(new_parent);
            for (DOMnodeUID ancestor : ancestors)
                if (ancestor == subtree_root)
                    return false;

            DOMnodeUID old_parent = nodes[subtree_root].getParent();
            nodes[old_parent].removeChild(subtree_root);
            nodes[new_parent].addChild(subtree_root);
            nodes[subtree_root].setParent(new_parent);
            return true;
        }

        /**
         * @brief   Deletes the subtree with the given node as root.
         *          Deletes the single node if no child nodes present.
         * @param   dom_parser::DOMnodeUID subtree_root Subtree root node.
         */
        void deleteSubtree(DOMnodeUID subtree_root)
        {
            if (!checkNodeExistance(subtree_root))
                return;

            DOMnodeUID current_node; // = subtree_root;
            std::queue<DOMnodeUID> node_queue;
            node_queue.push(subtree_root);

            while (!node_queue.empty())
            {
                current_node = node_queue.front();

                for (DOMnodeUID node : nodes[current_node].getChildrenUID())
                    node_queue.push(node);
                node_queue.pop();

                vacantUIDs.push(current_node);
                nodes[current_node] = deletedNode;
                nodes_counter--;
            }
        }

        /**
         * @brief   Returns std::vector of ancestors of the given node.
         * @param   dom_parser::DOMnodeUID node     The node UID.
         */
        std::vector<DOMnodeUID> getAncestorList(DOMnodeUID node)
        {
            std::vector<DOMnodeUID> ancestorList;
            DOMnodeUID node_uid = node;
            while (node_uid != 0)
            {
                node_uid = nodes[node_uid].getParent();
                ancestorList.push_back(node_uid);
            }
            return ancestorList;
        }

        /**
         * @brief   Operator overload for =
         * */
        void operator=(const DOMtree &tree)
        {
            this->nodes = tree.nodes;
            this->nodes_counter = tree.nodes_counter;
            this->vacantUIDs = tree.vacantUIDs;
        }
    };

} // namespace dom_parser

#endif