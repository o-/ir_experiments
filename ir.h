#ifndef IR_H
#define IR_H

#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <functional>

class Env {
 public:
  std::string name;
  Env(std::string name) : name(name) {}
  int references = 0;
  void print(std::ostream& o);
  void label(std::ostream& o) { o << "E_" << name; }
  void id(std::ostream& o) { o << "Env_" << this; }
};

class Value {
 public:
  enum class Type {
    Int
  };

  union Val {
    int i;
    Val(int i) : i(i) {}
  };

  Type t;
  Val v;

  Value(int i) : t(Type::Int), v(i) {}

  void print(std::ostream& o);
  Value operator + (const Value & o);
};

class Node {
 public:
  std::vector<Node*> val_in;
  std::vector<Node*> syntax_in;

  Node(std::vector<Node*> val_in, std::vector<Node*> syntax_in) :
      val_in(val_in), syntax_in(syntax_in) {}

  virtual void label(std::ostream& o) = 0;
  void id(std::ostream& o) { o << "Node_" << this; }

  void printGraph(std::ostream& o, std::string name);
  virtual void print(std::ostream& o, std::vector<Node*>& done);

  void foreachValue(std::function<Node*(Node*)> opt) {
    for (int i = 0; i < val_in.size(); ++i)
      val_in[i] = opt(val_in[i]);
  }

  void foreach(std::function<Node*(Node*)> opt) {
    for (int i = 0; i < val_in.size(); ++i)
      val_in[i] = opt(val_in[i]);
    for (int i = 0; i < syntax_in.size(); ++i)
      syntax_in[i] = opt(syntax_in[i]);
  }

  // Optimizations:

  virtual Node* deprom() {
    foreachValue([](Node* i) { return i->deprom(); });
    return this;
  }

  virtual Node* bind(Env* e, std::vector<Node*> a) {
    foreach([e,a](Node* i){ return i->bind(e, a); });
    return this;
  }

  virtual Node* activate() {
    foreachValue([](Node* i) { return i->activate(); });
    return this;
  }

  virtual Node* loadElim() {
    foreachValue([](Node* i){ return i->loadElim(); });
    return this;
  }

  virtual Node* tryInline() {
    foreachValue([](Node* i){ return i->tryInline(); });
    return this;
  }

  virtual Node* constantFold() {
    foreachValue([](Node* i){ return i->constantFold(); });
    return this;
  }
};

class Constant : public Node {
 public:
  Value v;
  Constant(Value v) : Node({}, {}), v(v) {}
  void label(std::ostream& o) { v.print(o); }
};

class Add : public Node {
 public:
  Add(Node* l, Node* r) : Node({l, r}, {}) {}
  void label(std::ostream& o) { o << "+"; }

  virtual Node* constantFold() {
    Constant* l = nullptr;
    Constant* r = nullptr;
    if ((l = dynamic_cast<Constant*>(val_in[0])) &&
        (r = dynamic_cast<Constant*>(val_in[1]))) {
      return new Constant(l->v + r->v);
    };

    return Node::constantFold();
  }
};

class Fun : public Node {
 public:
  Env* e;
  bool activated = false;

  Fun(Node* body, Env* e) : Node({}, {body}), e(e) { }
  void label(std::ostream& o) { o << "fun"; }

  Node* bind(Env* e, std::vector<Node*> a) {
    body(body()->bind(e, a));
    return this;
  }

  void doActivate() {
    if (activated) return;
    activated = true;
    val_in.resize(1);
    val_in[0] = syntax_in[0];
    syntax_in.resize(0);
  }


  Node* body() { return activated ? val_in[0] : syntax_in[0]; }
  void body(Node* b) {
    if (activated) {
      val_in[0] = b;
    } else {
      syntax_in[0] = b;
    }
  }

  void print(std::ostream& o, std::vector<Node*>& done);
};

class Call : public Node {
 public:
  Call(Node* f, std::vector<Node*> args) : Node({f}, args) {}
  void label(std::ostream& o) { o << "()"; }

  Node* deprom() {
    Fun* f = nullptr;
    if ((f = dynamic_cast<Fun*>(val_in[0]))) {
      f->bind(f->e, syntax_in);
    }
    return Node::deprom();
  }

  Node* activate() {
    Fun* f = nullptr;
    if ((f = dynamic_cast<Fun*>(val_in[0]))) {
      f->doActivate();
    }
    return Node::activate();
  }

  Node* tryInline() {
    Fun* f = nullptr;
    if ((f = dynamic_cast<Fun*>(val_in[0]))) {
      if (f->e->references == 0)
        return f->body();
    }
    return Node::tryInline();
  }
};

class Arg : public Node {
 public:
  int i;
  Env* e;
  Arg(int i, Env* e) : Node({}, {}), i(i), e(e) {
    e->references++;
  }
  void label(std::ostream& o) { o << "a_" << i; }

  virtual Node* bind(Env* e_in, std::vector<Node*> a) {
    if (e_in == e) {
      e->references--;
      e = nullptr;
      assert(i < a.size());
      assert(val_in.empty() || val_in[0] == a[i]);
      val_in.resize(1);
      val_in[0] = a[i];
    }
    return this;
  }

  virtual Node* loadElim() {
    if (val_in.empty())
      return this;
    assert(val_in.size() == 1);
    return val_in[0];
  }

  void print(std::ostream& o, std::vector<Node*>& done);
};

class Seq : public Node {
 public:
  Seq(std::vector<Node*> b) : Node(b, {}) {}
  void label(std::ostream& o) { o << "{"; }
};

#endif
