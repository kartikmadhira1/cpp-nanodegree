#include "route_planner.h"
#include <algorithm>


RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model) {
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;
    start_node = &m_Model.FindClosestNode(start_x,start_y);
    end_node = &m_Model.FindClosestNode(end_x,end_y);
}

std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node){
    std::vector<RouteModel::Node> path_found={};
    distance =0.0f;

    while (current_node != nullptr){
        path_found.push_back(*current_node);
        distance+=m_Model.MetricScale()*current_node->distance(*current_node->parent);        
        current_node = current_node->parent;
    }
    return path_found;
}

void RoutePlanner::AStarSearch(){
    start_node->visited = true;
    open_list.push_back(start_node);
    RouteModel::Node *current_node = nullptr;
    while(open_list.size()>0){
        current_node = NextNode();
        if(current_node->distance(*end_node)==0){
            m_Model.path = ConstructFinalPath(end_node);
            return;
        }
        else{
            AddNeighbors(current_node);
        }
    }
}
float RoutePlanner::CalculateHValue(const RouteModel::Node* curr_node){
    return end_node->distance(*curr_node);
}

RouteModel::Node* RoutePlanner::NextNode(){
    std::sort(open_list.begin(),open_list.end(),[](const auto &first,const auto &second){
      if(first->g_value+first->h_value>second->h_value+second->g_value){
        return true;
      }
      else return false;
    });
    RouteModel::Node *last = open_list.back();
    open_list.pop_back();
    return last;
}


void RoutePlanner::AddNeighbors(RouteModel::Node *node){
    node->FindNeighbors();
    for(auto neighbor:node->neighbors){
        neighbor->parent = node;
        neighbor->g_value = node->g_value + node->distance(*neighbor);
        neighbor->h_value = RoutePlanner::CalculateHValue(neighbor);
        open_list.push_back(neighbor);
        neighbor->visited = true;
    }
}