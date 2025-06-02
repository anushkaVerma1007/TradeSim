#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <limits>

using namespace std;

// ================================= Order Class =================================

class Order {
public:
    int orderID;
    string type;
    double price;
    int quantity;
    long timestamp;
    
    Order(int id, const string& orderType, double orderPrice, int orderQuantity, long orderTimestamp)
        : orderID(id), type(orderType), price(orderPrice), quantity(orderQuantity), timestamp(orderTimestamp) {}
    
    void display() const {
        cout << "Order ID: " << orderID << ", Type: " << type 
             << ", Price: $" << price << ", Quantity: " << quantity 
             << ", Timestamp: " << timestamp << endl;
    }
};

// ================================= Order Comparators =================================

// Comparator for buy orders (max-heap by price, then by timestamp for same price)
struct BuyOrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) {
            return a.price < b.price; // Max-heap: higher price has priority
        }
        return a.timestamp > b.timestamp; // Earlier timestamp has priority
    }
};

// Comparator for sell orders (min-heap by price, then by timestamp for same price)
struct SellOrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) {
            return a.price > b.price; // Min-heap: lower price has priority
        }
        return a.timestamp > b.timestamp; // Earlier timestamp has priority
    }
};

// ================================= Utils Class =================================

class Utils {
public:
    static long getCurrentTimestamp() {
        return chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count();
    }
    
    static Order generateRandomOrder(int orderID) {
        static random_device rd;
        static mt19937 gen(rd());
        static uniform_int_distribution<> typeDist(0, 1);
        static uniform_real_distribution<> priceDist(50.0, 150.0);
        static uniform_int_distribution<> quantityDist(10, 500);
        
        string type = (typeDist(gen) == 0) ? "buy" : "sell";
        double price = round(priceDist(gen) * 100.0) / 100.0; // Round to 2 decimal places
        int quantity = quantityDist(gen);
        long timestamp = getCurrentTimestamp();
        
        return Order(orderID, type, price, quantity, timestamp);
    }
};

// ================================= TradeLogger Class =================================

class TradeLogger {
private:
    ofstream logFile;
    
public:
    TradeLogger(const string& filename = "trades.log") {
        logFile.open(filename, ios::app);
        if (logFile.is_open()) {
            logFile << "\n========== Trading Session Started at " 
                    << getCurrentTimeString() << " ==========\n";
        }
    }
    
    ~TradeLogger() {
        if (logFile.is_open()) {
            logFile << "========== Trading Session Ended at " 
                    << getCurrentTimeString() << " ==========\n\n";
            logFile.close();
        }
    }
    
    void logTrade(int buyOrderID, int sellOrderID, double price, int quantity) {
        string tradeMsg = "Trade executed: BuyOrderID " + to_string(buyOrderID) +
                          " SellOrderID " + to_string(sellOrderID) +
                          " at price $" + to_string(price) +
                          " for quantity " + to_string(quantity);
        
        // Print to console
        cout << tradeMsg << endl;
        
        // Log to file
        if (logFile.is_open()) {
            logFile << getCurrentTimeString() << " - " << tradeMsg << endl;
            logFile.flush();
        }
    }
    
private:
    string getCurrentTimeString() {
        auto now = time(nullptr);
        auto tm = *localtime(&now);
        ostringstream oss;
        oss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// ================================= OrderBook Class =================================

class OrderBook {
private:
    priority_queue<Order, vector<Order>, BuyOrderComparator> buyOrders;
    priority_queue<Order, vector<Order>, SellOrderComparator> sellOrders;

public:
    void addBuyOrder(const Order& order) {
        buyOrders.push(order);
    }
    
    void addSellOrder(const Order& order) {
        sellOrders.push(order);
    }
    
    bool hasBuyOrders() const {
        return !buyOrders.empty();
    }
    
    bool hasSellOrders() const {
        return !sellOrders.empty();
    }
    
    Order getTopBuyOrder() {
        Order order = buyOrders.top();
        buyOrders.pop();
        return order;
    }
    
    Order getTopSellOrder() {
        Order order = sellOrders.top();
        sellOrders.pop();
        return order;
    }
    
    Order peekTopBuyOrder() const {
        return buyOrders.top();
    }
    
    Order peekTopSellOrder() const {
        return sellOrders.top();
    }
    
    void displayOrderBook() const {
        cout << "\n========== ORDER BOOK ==========\n";
        
        // Display buy orders (we need to copy the queue to display without modifying)
        cout << "BUY ORDERS (Highest price first):\n";
        if (buyOrders.empty()) {
            cout << "  No buy orders\n";
        } else {
            auto tempBuyQueue = buyOrders;
            int count = 0;
            while (!tempBuyQueue.empty() && count < 5) { // Show top 5
                Order order = tempBuyQueue.top();
                tempBuyQueue.pop();
                cout << "  ";
                order.display();
                count++;
            }
            if (buyOrders.size() > 5) {
                cout << "  ... and " << (buyOrders.size() - 5) << " more buy orders\n";
            }
        }
        
        cout << "\nSELL ORDERS (Lowest price first):\n";
        if (sellOrders.empty()) {
            cout << "  No sell orders\n";
        } else {
            auto tempSellQueue = sellOrders;
            int count = 0;
            while (!tempSellQueue.empty() && count < 5) { // Show top 5
                Order order = tempSellQueue.top();
                tempSellQueue.pop();
                cout << "  ";
                order.display();
                count++;
            }
            if (sellOrders.size() > 5) {
                cout << "  ... and " << (sellOrders.size() - 5) << " more sell orders\n";
            }
        }
        cout << "===============================\n\n";
    }
    
    size_t getBuyOrderCount() const { return buyOrders.size(); }
    size_t getSellOrderCount() const { return sellOrders.size(); }
};

// ================================= MatchingEngine Class =================================

class MatchingEngine {
private:
    OrderBook orderBook;
    TradeLogger tradeLogger;
    
public:
    MatchingEngine() : tradeLogger("trades.log") {}
    
    void processOrder(const Order& newOrder) {
        // Risk check: don't allow orders over 1000 quantity
        if (newOrder.quantity > 1000) {
            cout << "Order rejected: Quantity " << newOrder.quantity 
                 << " exceeds maximum allowed (1000)\n";
            return;
        }
        
        cout << "\nProcessing new order:\n";
        newOrder.display();
        
        if (newOrder.type == "buy") {
            processBuyOrder(newOrder);
        } else {
            processSellOrder(newOrder);
        }
    }
    
    void displayOrderBook() {
        orderBook.displayOrderBook();
    }
    
    void generateRandomOrders(int count) {
        cout << "\nGenerating " << count << " random orders...\n";
        static int orderCounter = 10000; // Start from 10000 for random orders
        
        for (int i = 0; i < count; i++) {
            Order randomOrder = Utils::generateRandomOrder(orderCounter++);
            processOrder(randomOrder);
        }
    }
    
private:
    void processBuyOrder(Order buyOrder) {
        // Try to match with existing sell orders
        while (buyOrder.quantity > 0 && orderBook.hasSellOrders()) {
            Order topSellOrder = orderBook.peekTopSellOrder();
            
            // Check if prices match (buy price >= sell price)
            if (buyOrder.price >= topSellOrder.price) {
                // Remove the sell order from the book
                orderBook.getTopSellOrder();
                
                // Execute trade
                int tradeQuantity = min(buyOrder.quantity, topSellOrder.quantity);
                double tradePrice = topSellOrder.price; // Use the sell order's price
                
                tradeLogger.logTrade(buyOrder.orderID, topSellOrder.orderID, tradePrice, tradeQuantity);
                
                // Update quantities
                buyOrder.quantity -= tradeQuantity;
                topSellOrder.quantity -= tradeQuantity;
                
                // If sell order still has quantity, put it back in the book
                if (topSellOrder.quantity > 0) {
                    orderBook.addSellOrder(topSellOrder);
                }
            } else {
                // No match possible, break the loop
                break;
            }
        }
        
        // If there's remaining quantity, add to order book
        if (buyOrder.quantity > 0) {
            orderBook.addBuyOrder(buyOrder);
        }
    }
    
    void processSellOrder(Order sellOrder) {
        // Try to match with existing buy orders
        while (sellOrder.quantity > 0 && orderBook.hasBuyOrders()) {
            Order topBuyOrder = orderBook.peekTopBuyOrder();
            
            // Check if prices match (sell price <= buy price)
            if (sellOrder.price <= topBuyOrder.price) {
                // Remove the buy order from the book
                orderBook.getTopBuyOrder();
                
                // Execute trade
                int tradeQuantity = min(sellOrder.quantity, topBuyOrder.quantity);
                double tradePrice = topBuyOrder.price; // Use the buy order's price
                
                tradeLogger.logTrade(topBuyOrder.orderID, sellOrder.orderID, tradePrice, tradeQuantity);
                
                // Update quantities
                sellOrder.quantity -= tradeQuantity;
                topBuyOrder.quantity -= tradeQuantity;
                
                // If buy order still has quantity, put it back in the book
                if (topBuyOrder.quantity > 0) {
                    orderBook.addBuyOrder(topBuyOrder);
                }
            } else {
                // No match possible, break the loop
                break;
            }
        }
        
        // If there's remaining quantity, add to order book
        if (sellOrder.quantity > 0) {
            orderBook.addSellOrder(sellOrder);
        }
    }
};

// ================================= Main Function =================================

int main() {
    MatchingEngine engine;
    int choice;
    static int orderCounter = 1;
    
    cout << "=== High-Frequency Trading Engine ===\n";
    cout << "Welcome to the Order Matching System!\n\n";
    
    while (true) {
        cout << "\n========== MAIN MENU ==========\n";
        cout << "1. Place new order\n";
        cout << "2. Show current order book\n";
        cout << "3. Generate random orders\n";
        cout << "4. Exit\n";
        cout << "==============================\n";
        cout << "Enter your choice (1-4): ";
        
        cin >> choice;
        
        // Clear input buffer
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: {
                string type;
                double price;
                int quantity;
                
                cout << "\n--- Place New Order ---\n";
                cout << "Enter order type (buy/sell): ";
                cin >> type;
                
                if (type != "buy" && type != "sell") {
                    cout << "Invalid order type! Please enter 'buy' or 'sell'.\n";
                    break;
                }
                
                cout << "Enter price: $";
                cin >> price;
                
                if (price <= 0) {
                    cout << "Invalid price! Price must be positive.\n";
                    break;
                }
                
                cout << "Enter quantity: ";
                cin >> quantity;
                
                if (quantity <= 0) {
                    cout << "Invalid quantity! Quantity must be positive.\n";
                    break;
                }
                
                Order newOrder(orderCounter++, type, price, quantity, Utils::getCurrentTimestamp());
                engine.processOrder(newOrder);
                break;
            }
            
            case 2: {
                engine.displayOrderBook();
                break;
            }
            
            case 3: {
                int count;
                cout << "Enter number of random orders to generate: ";
                cin >> count;
                
                if (count <= 0 || count > 100) {
                    cout << "Please enter a number between 1 and 100.\n";
                    break;
                }
                
                engine.generateRandomOrders(count);
                break;
            }
            
            case 4: {
                cout << "\nThank you for using the High-Frequency Trading Engine!\n";
                cout << "All trades have been logged to 'trades.log'.\n";
                cout << "Goodbye!\n";
                return 0;
            }
            
            default: {
                cout << "Invalid choice! Please enter a number between 1 and 4.\n";
                break;
            }
        }
    }
    
    return 0;
}