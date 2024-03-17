#include "message.h"
#include "orderbook.h"
#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <levels>" << std::endl;
        return 1;
    }

    std::ofstream outputFile("output.log", std::ios::out | std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output.log for writing." << std::endl;
        return 1;
    }

    size_t levels = std::stoi(argv[1]);
    OrderBook orderBook;

    auto opt = MessageReader::ReadNext(std::cin);
    for (; opt.has_value(); opt = MessageReader::ReadNext(std::cin)) {
        auto& mh = opt.value();
        std::string symbol; // Variable to store the symbol for each message

        switch (mh.header.msg_type) {
            case EventType::ADD: {
                auto orderAdd = std::any_cast<OrderAdd>(mh.msg);
                symbol = std::string(orderAdd.symbol, 3); // Assuming symbol is exactly 3 characters
                bool isBid = orderAdd.side == Side::BUY;
                Order newOrder(orderAdd.order_id, orderAdd.size, orderAdd.price);
                orderBook.addOrder(newOrder, isBid);
                break;
            }
            case EventType::UPDATE: {
                auto orderUpdate = std::any_cast<OrderUpdate>(mh.msg);
                symbol = std::string(orderUpdate.symbol, 3); // Extract the symbol
                orderBook.updateOrder(orderUpdate.order_id, orderUpdate.size, orderUpdate.price);
                break;
            }
            case EventType::DELETE: {
                auto orderDelete = std::any_cast<OrderDelete>(mh.msg);
                symbol = std::string(orderDelete.symbol, 3); // Extract the symbol
                orderBook.deleteOrder(orderDelete.order_id);
                break;
            }
            case EventType::TRADED: {
                auto orderExecuted = std::any_cast<OrderTraded>(mh.msg);
                symbol = std::string(orderExecuted.symbol, 3); // Extract the symbol
                orderBook.executeOrder(orderExecuted.order_id, orderExecuted.volume);
                break;
            }
            default:
                std::cout << "UNKNOWN MESSAGE TYPE" << std::endl;
                continue;
        }
        // Generate and print the snapshot string
        std::string snapshotStr = orderBook.getSnapshotAsString(mh.header.seq_num, symbol, levels);
        std::cout << snapshotStr << std::endl;

        // Write the snapshot to the already opened output.log file
        outputFile << snapshotStr << std::endl;
    }

	outputFile.close();
    return 0;
}