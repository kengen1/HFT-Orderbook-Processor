#include <map>
#include <vector>
#include <list>
#include <unordered_map>
#include <sstream>

struct Order {
    uint64_t order_id;
    uint64_t volume;
    int32_t price; // Consider using a fixed-point representation for price if necessary

    // Constructor for convenience
    Order(uint64_t id, uint64_t vol, int32_t pr) : order_id(id), volume(vol), price(pr) {}
};

// Helper structure to hold order location info
struct OrderLocation {
    int32_t price;
    std::list<Order>::iterator it;
    bool isBid;
};


class OrderBook {
private:
    // Maps from price to a list of orders at that price
    // Using list to efficiently add/remove orders without invalidating iterators
    std::map<int32_t, std::list<Order>, std::greater<int32_t>> bids; // Correct for descending order
    std::map<int32_t, std::list<Order>> asks; // Correct for ascending order (default behavior)
    std::unordered_map<uint64_t, OrderLocation> orderIndex;

public:
    void addOrder(const Order& order, bool isBid);
    void updateOrder(uint64_t order_id, uint64_t new_volume, int32_t new_price);
    void deleteOrder(uint64_t order_id);
    void executeOrder(uint64_t order_id, uint64_t executed_volume);
    std::vector<std::pair<int32_t, uint64_t>> getDepthSnapshot(bool isBid, size_t levels);
    std::string getSnapshotAsString(unsigned int sequenceNo, const std::string& symbol, size_t levels);
};

void OrderBook::addOrder(const Order& order, bool isBid) {
    std::list<Order>* ordersList = nullptr;

    // Determine the correct list (bids or asks) based on the order type and get a reference to it
    if (isBid) {
        ordersList = &bids[order.price];
    } else {
        ordersList = &asks[order.price];
    }

    // Add the new order to the list
    ordersList->push_back(order);

    // Create an iterator to the newly added order. Since we just added the order, it's at the back of the list.
    auto orderIt = std::prev(ordersList->end());

    // Update the orderIndex with the new order's details including its location (iterator) and price
    orderIndex[order.order_id] = {order.price, orderIt, isBid};
}

void OrderBook::updateOrder(uint64_t order_id, uint64_t new_volume, int32_t new_price) {
    if (orderIndex.find(order_id) == orderIndex.end()) {
        std::cerr << "Order ID not found for update: " << order_id << std::endl;
        return;
    }

    // Retrieve the order location info
    OrderLocation loc = orderIndex[order_id];

    // Remove the order from its current list
    if (loc.isBid) {
        bids[loc.price].erase(loc.it);
        if (bids[loc.price].empty()) {
            bids.erase(loc.price);
        }
    } else {
        asks[loc.price].erase(loc.it);
        if (asks[loc.price].empty()) {
            asks.erase(loc.price);
        }
    }

    // If the price has changed, we need to insert the order in the new position
    // Else, we can just update the volume in place
    if (loc.price != new_price) {
        // Remove old index entry
        orderIndex.erase(order_id);

        // Insert order with new price and volume
        Order newOrder(order_id, new_volume, new_price);
        addOrder(newOrder, loc.isBid);

        // Note: The addOrder method must be updated to handle orderIndex updates.
    } else {
        // Update volume directly if the price hasn't changed
        loc.it->volume = new_volume;

        // Update the orderIndex in case of volume change with same price
        orderIndex[order_id] = {new_price, loc.it, loc.isBid};
    }
}

void OrderBook::deleteOrder(uint64_t order_id) {
    // Check if the order exists
    auto it = orderIndex.find(order_id);
    if (it == orderIndex.end()) {
        std::cerr << "Order ID not found for deletion: " << order_id << std::endl;
        return;
    }

    // Retrieve the order location and details
    OrderLocation loc = it->second;

    // Remove the order from its current list
    if (loc.isBid) {
        bids[loc.price].erase(loc.it);
        // If no more orders at this price, remove the price entry
        if (bids[loc.price].empty()) {
            bids.erase(loc.price);
        }
    } else {
        asks[loc.price].erase(loc.it);
        // If no more orders at this price, remove the price entry
        if (asks[loc.price].empty()) {
            asks.erase(loc.price);
        }
    }

    // Remove the order from the index
    orderIndex.erase(it);
}

void OrderBook::executeOrder(uint64_t order_id, uint64_t executed_volume) {
    // Check if the order exists in the orderIndex
    auto it = orderIndex.find(order_id);
    if (it == orderIndex.end()) {
        std::cerr << "Order ID not found for execution: " << order_id << std::endl;
        return;
    }

    // Retrieve the order's location and details
    OrderLocation loc = it->second;

    // Calculate the new volume after execution
    uint64_t newVolume = loc.it->volume > executed_volume ? loc.it->volume - executed_volume : 0;

    if (newVolume > 0) {
        // If the order is still partially filled, update its volume
        loc.it->volume = newVolume;
    } else {
        // If the order is fully executed, remove it from the list
        if (loc.isBid) {
            bids[loc.price].erase(loc.it);
            // If no more orders at this price, remove the price entry
            if (bids[loc.price].empty()) {
                bids.erase(loc.price);
            }
        } else {
            asks[loc.price].erase(loc.it);
            // If no more orders at this price, remove the price entry
            if (asks[loc.price].empty()) {
                asks.erase(loc.price);
            }
        }

        // Remove the order from the orderIndex
        orderIndex.erase(it);
    }
}

std::vector<std::pair<int32_t, uint64_t>> OrderBook::getDepthSnapshot(bool isBid, size_t levels) {
    std::vector<std::pair<int32_t, uint64_t>> snapshot;

    // Since bids and asks are of different types due to their sorting criteria,
    // we handle them separately within their respective conditional blocks.
    if (isBid) {
        // Process bids
        size_t count = 0;
        for (const auto& priceOrders : bids) {
            if (count++ >= levels) break;
            uint64_t aggregatedVolume = 0;
            for (const auto& order : priceOrders.second) {
                aggregatedVolume += order.volume;
            }
            snapshot.emplace_back(priceOrders.first, aggregatedVolume);
        }
    } else {
        // Process asks
        size_t count = 0;
        for (const auto& priceOrders : asks) {
            if (count++ >= levels) break;
            uint64_t aggregatedVolume = 0;
            for (const auto& order : priceOrders.second) {
                aggregatedVolume += order.volume;
            }
            snapshot.emplace_back(priceOrders.first, aggregatedVolume);
        }
    }

    return snapshot;
}

std::string OrderBook::getSnapshotAsString(unsigned int sequenceNo, const std::string& symbol, size_t levels) {
    auto bidsSnapshot = getDepthSnapshot(true, levels);
    auto asksSnapshot = getDepthSnapshot(false, levels);

    std::stringstream ss;

    // Building the string
    ss << sequenceNo << ", " << symbol << ", [";
    for (const auto& bid : bidsSnapshot) {
        ss << "(" << bid.first << ", " << bid.second << "), ";
    }
    // Remove trailing comma and space if any bids were added
    if (!bidsSnapshot.empty()) {
        ss.seekp(-2, ss.cur); // Go back two characters to remove the last ", "
        ss << "], [";
    } else {
        ss << "], [";
    }

    for (const auto& ask : asksSnapshot) {
        ss << "(" << ask.first << ", " << ask.second << "), ";
    }
    // Remove trailing comma and space if any asks were added
    if (!asksSnapshot.empty()) {
        ss.seekp(-2, ss.cur); // Go back two characters to remove the last ", "
    }

    ss << "]";

    return ss.str();
}
