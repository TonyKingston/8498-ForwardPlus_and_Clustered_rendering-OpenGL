#pragma once
#include <array>
#include <variant>
#include "FunctionUtils.h"

namespace NCL {

    /* Very basic stack based pool. Type T should be default constructable. */
    template<typename T, size_t N>
    class SimpleObjectPool {
    public:
        int Create() {
            int index = firstFree;
            firstFree = std::get<int>(elements[index]);
            return index;
        }

        void Destroy(int index) {
            elements[index] = firstFree;
            firstFree = index;
        }
    protected;

        std::array<std::variant<T, int>, N> elements{};
        int firstFree = 0;
    };

 /** Usage Example 
    int main() {
        const size_t PoolSize = 10;
        SimpleObjectPool<int, PoolSize> pool;

        // Create objects in the pool
        int index1 = pool.Create();
        int index2 = pool.Create();
        int index3 = pool.Create();

        std::cout << "Objects created at indices: " << index1 << ", " << index2 << ", " << index3 << std::endl;

        // Destroy an object
        pool.Destroy(index2);
        std::cout << "Object at index " << index2 << " destroyed." << std::endl;

        // Create another object
        int index4 = pool.Create();
        std::cout << "New object created at index: " << index4 << std::endl;

        return 0;
    }

**/

}

