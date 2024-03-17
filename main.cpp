#include "message.h"
#include "orderbook.h"
#include <iostream>
#include <string>
#include <fstream>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <levels>" << std::endl;
        return 1;
    }

    // Set std::cin to binary mode on Windows
    #ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    #endif

    std::ofstream outputFile("output.log", std::ios::out | std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output.log for writing." << std::endl;
        return 1;
    }

    // Logging to track the reading process
    std::ofstream debugLog("debug.log", std::ios::out | std::ios::trunc);
    if (!debugLog.is_open()) {
        std::cerr << "Failed to open debug.log for writing." << std::endl;
        return 1;
    }

    size_t levels = std::stoi(argv[1]);
    OrderBook orderBook;

    debugLog << "Attempting to read first message from stream." << std::endl;

    auto opt = MessageReader::ReadNext(std::cin);
    while (opt.has_value()) {
        auto& mh = opt.value();
        std::string symbol; // Variable to store the symbol for each message

        // Log the message type and sequence number
        debugLog << "Read message: Seq No " << mh.header.seq_num << ", Type " << static_cast<char>(mh.header.msg_type) << std::endl;

        switch (mh.header.msg_type) {
            case EventType::ADD: {
                auto orderAdd = std::any_cast<OrderAdd>(mh.msg);
                symbol = std::string(orderAdd.symbol, 3); // Assuming symbol is exactly 3 characters
                bool isBid = orderAdd.side == Side::BUY;
                Order newOrder(orderAdd.order_id, orderAdd.size, orderAdd.price);
                orderBook.addOrder(newOrder, isBid);

                // Log the action taken
                debugLog << "Added Order: " << orderAdd.order_id << std::endl;
                break;
            }
            case EventType::UPDATE: {
                auto orderUpdate = std::any_cast<OrderUpdate>(mh.msg);
                symbol = std::string(orderUpdate.symbol, 3); // Extract the symbol
                orderBook.updateOrder(orderUpdate.order_id, orderUpdate.size, orderUpdate.price);

                // Log the action taken
                debugLog << "Updated Order: " << orderUpdate.order_id << std::endl;
                break;
            }
            case EventType::DELETE: {
                auto orderDelete = std::any_cast<OrderDelete>(mh.msg);
                symbol = std::string(orderDelete.symbol, 3); // Extract the symbol
                orderBook.deleteOrder(orderDelete.order_id);

                // Log the action taken
                debugLog << "Deleted Order: " << orderDelete.order_id << std::endl;
                break;
            }
            case EventType::TRADED: {
                auto orderExecuted = std::any_cast<OrderTraded>(mh.msg);
                symbol = std::string(orderExecuted.symbol, 3); // Extract the symbol
                orderBook.executeOrder(orderExecuted.order_id, orderExecuted.volume);

                // Log the action taken
                debugLog << "Executed Order: " << orderExecuted.order_id << std::endl;
                break;
            }
            default:
                debugLog << "Encountered unknown message type." << std::endl;
                continue;
        }

        // Generate and print the snapshot string
        std::string snapshotStr = orderBook.getSnapshotAsString(mh.header.seq_num, symbol, levels);
        std::cout << snapshotStr << std::endl;

        // Write the snapshot to the already opened output.log file
        outputFile << snapshotStr << std::endl;

        // Read the next message
        opt = MessageReader::ReadNext(std::cin);
    }

    outputFile.close();
    debugLog.close(); // Ensure the debug log is closed properly

    return 0;
}
