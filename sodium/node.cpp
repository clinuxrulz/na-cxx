#include "node.h"
#include <stdint.h>
#include <bacon_gc/gc.h>
#include <functional>
#include <vector>

namespace sodium {

    struct Source {
        bacon_gc::Gc<Node> target;
        std::function<std::function<void()>()> register__;
        bool registered;
        std::function<void()> deregister_;

        void register_() {
            if (!registered) {
                registered = true;
                deregister_ = register__();
            }
        }

        void deregister() {
            if (registered) {
                registered = false;
                deregister_();
            }
        }
    };

    struct Node {
        uint32_t id;
        uint32_t rank;
        std::vector<bacon_gc::Gc<Source>> sources;
        std::vector<bacon_gc::GcWeak<Node>> targets;
        bool visited;

        void ensure_bigger_than(uint32_t limit) {
            if (rank > limit || visited) {
                return;
            }
            visited = true;
            rank = limit + 1;
            for (std::vector<bacon_gc::GcWeak<Node>>::iterator it = targets.begin(); it != targets.end(); ++it) {
                bacon_gc::Gc<Node> target = it->lock();
                if (target) {
                    target.value().ensure_bigger_than(rank);
                }
            }
            visited = false;
        }
    };

} // end namespace sodium

namespace bacon_gc {
    template<>
    struct Trace<sodium::Source> {
        template <class F>
        static void trace(const sodium::Source& source, F&& k) {
            Trace<Gc<sodium::Node>>::trace(source.target, k);
        }
    };

    template<>
    struct Finalize<sodium::Source> {
        static void finalize(sodium::Source& source) {
            source.deregister();
        }
    };
}
