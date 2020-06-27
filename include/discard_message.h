
#include <queue>
#include <deque>
#include <type_traits>

namespace umsg {
    // Canonical message handler
    template<typename T>
    class discard_message
    {
    public:
        void msg(T c) {
            // handle message here
        }
    };

    // Degenerate notification-only message handler
    template<>
    class discard_message<void>
    {
        void msg() {
        }
    };
}
