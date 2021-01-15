// Copyright 2021 Roger Peralta Aranibar
#ifndef SOURCE_PERSISTENCE_PARTIAL_DIRECTED_GRAPH_HPP_
#define SOURCE_PERSISTENCE_PARTIAL_DIRECTED_GRAPH_HPP_

#include <utility>
#include <limits>

namespace ADE {
namespace Persistence {

template <typename Type>
class PartialNode {
 public:
  typedef Type data_type;

  PartialNode() : data_(nullptr), forward_(nullptr), out_ptrs_size_(0), current_back_pointer_size_(0),
                  prev_version(nullptr), corresponding_version(0), last_graph_version(nullptr){}

  PartialNode(data_type const& data, std::size_t const& out_ptrs_size, std::size_t const& in_ptrs_size, unsigned int *g_version)
                    : data_(new data_type(data)), out_ptrs_size_(out_ptrs_size),
                      in_ptrs_size_(in_ptrs_size), current_back_pointer_size_(0),
                      prev_version(nullptr), corresponding_version(std::numeric_limits<unsigned int>::max()),
                      last_graph_version(g_version) {
    forward_ = new PartialNode<Type>*[out_ptrs_size]();
    backward_ = new PartialNode<Type>*[in_ptrs_size]();
  }

  data_type get_data(unsigned int version) {
    return search_version(version).data_;
  }

  bool set_data(data_type const& data) {
    PartialNode* old_version = new PartialNode<Type>(data, out_ptrs_size_, in_ptrs_size_, last_graph_version);
    old_version->corresponding_version = last_graph_version;//close this node as a old version node
    ++(*last_graph_version);
    for (size_t i(0); i < in_ptrs_size_; ++i)
      old_version->backward_[i] = backward_[i];
    for (size_t i(0); i < out_ptrs_size_; ++i)
      old_version->forward_[i] = forward_[i];
    old_version->prev_version = prev_version;
    prev_version = old_version;
    return true;
  }

  PartialNode* insert_vertex(std::size_t const& position,
                             data_type const& data) {
    if (out_ptrs_size_ < position)
      throw std::out_of_range("Insert position is out of edges range.");
    PartialNode* next_node_ptr = forward_[position];
    if (next_node_ptr->in_ptrs_size_ < next_node_ptr->current_back_pointer_size_)
      throw std::out_of_range("Position out of second argument node.");
    PartialNode* new_node = new PartialNode(data, out_ptrs_size_, in_ptrs_size_, last_graph_version);
    new_node->forward_[position] = next_node_ptr;
    next_node_ptr->backward_[next_node_ptr->current_back_pointer_size_] = new_node;
    forward_[position] = new_node;
    new_node->backward_[0] = this;
    return forward_[position];
  }

  bool update_edge(std::size_t const& position, PartialNode* v) {
    if (out_ptrs_size_ < position)
      throw std::out_of_range("Position out of first argument node.");
    forward_[position] = v;
    if(v->in_ptrs_size_ < v->current_back_pointer_size_)
      throw std::out_of_range("Position out of second argument node.");
    v->backward_[v->current_back_pointer_size_++] = this;
    return true;
  }

  PartialNode& operator[](
      std::pair<std::size_t, unsigned int> const& position_version) const {
    if (out_ptrs_size_ < id)
      throw std::out_of_range("Index out of node edges range.");
    if (!forward_[id])
      throw std::logic_error("Access to null reference.");
    return *search_version(position_version.second)->forward_[position_version.first];
  }

  PartialNode* search_version(unsigned int version) {
    PartialNode* current_version_node = this;//latest version
    PartialNode* chosen_node_JIC = current_version_node;
    while (current_version_node) {
      if (current_version_node->corresponding_version > version) {
        chosen_node_JIC = current_version_node;
        current_version_node = current_version_node->prev_version;
      }
      else if (current_version_node->corresponding_version == version)
        return current_version_node;
      else
        break;
    }
    return chosen_node_JIC;
  }

  data_type* data_;
  std::size_t out_ptrs_size_;
  PartialNode** forward_;

  std::size_t in_ptrs_size_;
  std::size_t current_modifications_size_;
  std::size_t current_back_pointer_size_;

  PartialNode** backward_;

  PartialNode* prev_version;
  unsigned int corresponding_version;
  unsigned int *last_graph_version;
};

template <typename Type, typename Node>
class PartialDirectedGraph {
 public:
  typedef Type data_type;

  PartialDirectedGraph(data_type const data, std::size_t const& out_ptrs_size,
                       std::size_t const& in_ptrs_size) {}

  Node* get_root_ptr(unsigned int const& version) {
    assert(version <= *current_version_);
    return root_ptr_->search_version(version);
  }

  Node get_root(unsigned int const& version) {
    assert(version <= *current_version_);
    return *(root_ptr_->search_version(version));
  }

  Node* insert_vertex(data_type const data, Node* u, std::size_t position) {
    ++(*current_version_);
    return nullptr;
  }

  bool add_edge(Node* u, Node* v, std::size_t position) {
    ++(*current_version_);
    u->update_edge(position, v);
    return true;
  }

 protected:
  unsigned int* current_version_;
  std::size_t in_ptrs_size_;

  Node* root_ptr_;
  std::size_t out_ptrs_size_;
};

}  // namespace Persistence
}  // namespace ADE

#endif  // SOURCE_PERSISTENCE_PARTIAL_DIRECTED_GRAPH_HPP_
