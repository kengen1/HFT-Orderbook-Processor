#include <map>
#include <vector>
#include <list>

struct Order {
    uint64_t order_id;
    uint64_t volume;
    int32_t price; // Consider using a fixed-point representation for price if necessary

    // Constructor for convenience
    Order(uint64_t id, uint64_t vol, int32_t pr) : order_id(id), volume(vol), price(pr) {}
};

class OrderBook {
private:
    // Maps from price to a list of orders at that price
    // Using list to efficiently add/remove orders without invalidating iterators
    std::map<int32_t, std::list<Order>> bids;
    std::map<int32_t, std::list<Order>, std::greater<int32_t>> asks;

public:
    void addOrder(const Order& order, bool isBid);
    void updateOrder(uint64_t order_id, uint64_t new_volume, int32_t new_price, bool isBid);
    void deleteOrder(uint64_t order_id, bool isBid);
    void executeOrder(uint64_t order_id, uint64_t executed_volume, bool isBid);
    std::vector<std::pair<int32_t, uint64_t>> getDepthSnapshot(bool isBid, size_t levels);
};

void OrderBook::addOrder(const Order& order, bool isBid) {
    if (isBid) {
        // Handle buy order
        auto& ordersAtPrice = bids[order.price];
        ordersAtPrice.emplace_back(order);
    } else {
        // Handle sell order
        auto& ordersAtPrice = asks[order.price];
        ordersAtPrice.emplace_back(order);
    }
}
