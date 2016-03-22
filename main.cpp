#include "ir.h"

void optimize(Node* c, int steam) {
  c->printGraph(std::cout, "unopt");

  for (int i = 0; i < steam; ++i) {
    c = c->constantFold();

    c->printGraph(std::cout, "cfold");

    c = c->deprom();

    c->printGraph(std::cout, "deprom");

    c = c->activate();

    c->printGraph(std::cout, "activate");

    c = c->loadElim();

    c->printGraph(std::cout, "loadElim");

    c = c->tryInline();

    c->printGraph(std::cout, "inline");
  }
}

int main() {
  {
    // (function(a) a + 1)(2 + 1)

    Env* e = new Env("e");

    Node* body = new Add(
        new Constant(1), new Arg(0, e));;

    Node* fun = new Fun(body, e);

    std::vector<Node*> args = {
      new Add(new Constant(2), new Constant(1))};

    Node* c = new Call(fun, args);

    optimize(c, 3);
  }

  {
    // (function(a) function() a)(1)()

    Env* e = new Env("e");
    Env* anon = new Env("a");

    Node* anonFun = new Fun(
        new Arg(0, e), anon);

    Node* outerFun = new Fun(
        anonFun, e);

    Node* c = new Call(
        new Call(outerFun, {new Constant(1)}), {});

    optimize(c, 2);
  }

  {
    // (function(z) {
    //     (function(a) {
    //         a
    //     })(
    //         function(b, c) { b + c + d }
    //     )(2, 3 + z)
    // }(1)

    Env* e_inner = new Env("inner");
    Env* e_outer = new Env("outer");
    Env* e_top = new Env("top");

    Node* code =
      new Call(
          new Call(
            new Fun(new Arg(0, e_outer),
              e_outer), {
              new Fun(
                  new Add(
                    new Add(new Arg(1, e_inner), new Arg(1, e_inner)),
                    new Arg(0, e_inner)),
                  e_inner)}), {
          new Constant(2),
          new Add(
              new Arg(0, e_top), new Constant(3))});

    Node* c = new Call(
        new Fun(code, e_top),
        {new Constant(1)});

    optimize(c, 7);
  }
}

