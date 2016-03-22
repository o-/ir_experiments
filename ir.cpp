#include "ir.h"

void Value::print(std::ostream& o) {
  switch(t) {
    case Type::Int:
      o << v.i;
  }
}

Value Value::operator + (const Value & o) {
  if (t == Type::Int && o.t == Type::Int)
    return v.i + o.v.i;

  assert(false);
  return -1;
}

void Node::printGraph(std::ostream& o, std::string name) {
  o << "digraph " << name << " {\n";
  o << "label=\"" << name << "\";\nlabelloc=top;\nlabeljust=left;";
  std::vector<Node*> done;
  print(o, done);
  o << "}\n";
}

void Node::print(std::ostream& o, std::vector<Node*>& done) {
  if (std::find(done.begin(), done.end(), this) != done.end())
    return;

  done.push_back(this);

  id(o);
  o << " [label=\"";
  label(o);
  o << "\"];\n";

  for (auto n : val_in) {
    n->print(o, done);
    id(o);
    o << " -> ";
    n->id(o);
    o << ";\n";
  }

  for (auto n : syntax_in) {
    n->print(o, done);
    id(o);
    o << " -> ";
    n->id(o);
    o << " [style=dashed];\n";
  }
}

void Fun::print(std::ostream& o, std::vector<Node*>& done) {
  if (std::find(done.begin(), done.end(), this) != done.end())
    return;
  Node::print(o, done);

  e->print(o);
  id(o);
  o << " -> ";
  e->id(o);
  o << " [style=dotted];\n";
}

void Arg::print(std::ostream& o, std::vector<Node*>& done) {
  if (std::find(done.begin(), done.end(), this) != done.end())
    return;
  Node::print(o, done);

  if (e) {
    e->print(o);
    id(o);
    o << " -> ";
    e->id(o);
    o << " [style=dotted];\n";
  }
}

void Env::print(std::ostream& o) {
  id(o);
  o << " [label=\"";
  label(o);
  o << "\", shape=box];\n";
}
