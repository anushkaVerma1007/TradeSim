# High-Frequency Trading Engine

A simplified order matching system that simulates stock exchange trading engines using price-time priority logic and supports partial order execution.

---

## 🚀 Features

- ✅ **Order Matching** — Price-time priority with support for partial order fills  
- 📘 **Order Book** — Real-time view of buy/sell order queues  
- 📝 **Trade Logging** — Logs trades to both the console and `trades.log` file  
- 🔒 **Risk Management** — Rejects orders exceeding 1000 shares  
- 🖥️ **Interactive Menu** — Simple and clean user interface  
- 🧪 **Random Orders** — Automatically generate test orders for load simulation

---

## 🛠️ Quick Start

```bash
# Compile the source code
g++ -std=c++17 -Wall -Wextra -O2 -o trading_engine main.cpp

# Run the executable
./trading_engine
```

### 📋 Menu Options

- **Place new order** – Manually enter a buy or sell order
- **Show order book** – Displays current unmatched orders
- **Generate random orders** – Automatically simulate test orders
- **Exit** – End the program and save trade logs

---

## 💡 Example Session

```
Enter your choice (1-4): 1
Enter order type (buy/sell): buy
Enter price: $95.00
Enter quantity: 100

Processing new order:
Order ID: 1, Type: buy, Price: $95, Quantity: 100

Trade executed: BuyOrderID 1 SellOrderID 2 at price $94.500000 for quantity 100
```

---

## ⚙️ How It Works

- 📈 **Buy Orders** match with the lowest priced available sell orders
- 📉 **Sell Orders** match with the highest priced available buy orders
- ✅ **Matching Condition:** Buy price ≥ Sell price
- ⏱️ **Priority Rules:** Better price wins; otherwise, earlier order timestamp wins
- 🔁 **Partial Fills:** Orders can be partially matched if quantities differ

---

## 📦 Project Structure

```
high-frequency-trading/
├── main.cpp                  # Core logic and menu interface
├── trades.log                # Log of executed trades (created on run)
└── README.md                 # Project documentation
```

---

## 📌 Requirements

- C++17-compatible compiler
- Standard C++ libraries only (no external dependencies)

---

## 📄 Output Files

- `trading_engine` — The compiled executable
- `trades.log` — Text file containing all executed trades during session