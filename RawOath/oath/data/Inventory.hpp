class Inventory {
public:
    std::vector<Item> items;

    bool hasItem(const std::string& itemId, int quantity = 1) const;
    bool addItem(const Item& item);
    bool removeItem(const std::string& itemId, int quantity = 1);
};