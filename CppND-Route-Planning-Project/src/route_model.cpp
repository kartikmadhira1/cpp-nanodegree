#include "route_model.h"
#include <iostream>

RouteModel::RouteModel(const std::vector<std::byte> &xml) : Model(xml) {
int idx = 0;
for (Model::Node new_node:this->Nodes()){
        this->m_Nodes.push_back(Node(idx, this , new_node));
        idx++;
    }
CreateNodeToRoadHashmap();
}


void RouteModel::CreateNodeToRoadHashmap(){
    for (const Model::Road &road : Model::Roads()){
        if(road.type!= Model::Road::Type::Footway){
            for(auto node_idx:Ways()[road.way].nodes){
                if(node_to_road.find(node_idx)== node_to_road.end()){
                    node_to_road.insert(std::pair(node_idx,std::vector<const Model::Road *>()));
                }
                node_to_road[node_idx].push_back(&road); 
            }
        }
    }
}

RouteModel::Node * RouteModel::Node::FindNeighbor(std::vector<int> node_indices){
    RouteModel::Node *closest_node = nullptr;
    RouteModel::Node  node;
    for(int index:node_indices){
        node = parent_model->SNodes()[index];
        if( !node.visited && this->distance(node)!=0){
            if(closest_node==nullptr || this->distance(node)<this->distance(*closest_node)){
                closest_node = &parent_model->SNodes()[index];
            }
        }
    }
    return closest_node;
}



void RouteModel::Node::FindNeighbors(){
    for (auto &road:parent_model->node_to_road[this->index]){
    RouteModel::Node* closest = this->FindNeighbor(parent_model->Ways()[road->way].nodes);
            if(closest){
                this->neighbors.push_back(closest);
        }
    }
}


RouteModel::Node &RouteModel::FindClosestNode(float x, float y){
    RouteModel::Node  *temp_node;
    temp_node->x = x;
    temp_node->y = y;
    RouteModel::Node *temp_node_closest = nullptr;

    float min_dist = std::numeric_limits<float>::max();
    int closest_idx;
    for(auto &road: Roads()){
        if(road.type != Model::Road::Type::Footway){
            for (auto each_node : Ways()[road.way].nodes){
                *temp_node_closest = SNodes()[each_node];
                if(temp_node->distance(*temp_node_closest)<min_dist){
                    closest_idx = each_node;
                    min_dist = temp_node->distance(*temp_node_closest);
                }
            }
        }
    }
    return SNodes()[closest_idx];
}
