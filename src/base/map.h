#pragma once

template <typename Key, typename Value>
class r_unordered_map {
private:
    struct Node {
        Key key;
        Value value;
        Node* next;
    };

    static const std::size_t TABLE_SIZE = 2048;
    Node* table[TABLE_SIZE];

    std::size_t hash(const Key& key) const {
        std::size_t hash = 0;
        const char* str = reinterpret_cast<const char*>(&key);
        for (std::size_t i = 0; i < sizeof(Key); ++i) {
            hash = hash * 31 + str[i];
        }
        return hash % TABLE_SIZE;
    }

public:
    class Iterator {
    private:
        Node* node;
    public:
        Iterator(Node* n) : node(n) {}

        std::pair<Key, Value> operator*() const {
            return {node->key, node->value};
        }

        Iterator& operator++() {
            if (node) node = node->next;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return node != other.node;
        }
    };

    r_unordered_map() {
        for (std::size_t i = 0; i < TABLE_SIZE; ++i) {
            table[i] = nullptr;
        }
    }

    ~r_unordered_map() {
      clear();
    }

    void insert(const Key& key, const Value& value) {
        std::size_t idx = hash(key);
        Node* current = table[idx];
        while (current) {
          if (current->key == key) {  // Direct comparison
            current->value = value;
            return;
          }
          current = current->next;
        }

        Node* new_node = (Node*)malloc(sizeof(Node));
        new_node->key = key;
        new_node->value = value;
        new_node->next = table[idx];
        table[idx] = new_node;
    }

    Value& operator[](const Key& key) {
        std::size_t idx = hash(key);
        Node* current = table[idx];
        while (current) {
          if(current->key == key)
            return current->value;
          
          current = current->next;
        }

        Node* new_node = (Node*)malloc(sizeof(Node));
        new_node->key = key;
        new_node->value = Value();
        new_node->next = table[idx];
        table[idx] = new_node;
        return new_node->value;
    }

    Iterator find(const Key& key) const {
        std::size_t idx = hash(key);
        Node* current = table[idx];
        while (current) {
          if(current->key == key)
            return Iterator(current);
          
          current = current->next;
        }
        return end();
    }

    Value& at(const Key& key) const {
        std::size_t idx = hash(key);
        Node* current = table[idx];
        while (current) {
          if(current->key == key) 
            return current->value;
            
          current = current->next;
        }
        assert(false && "key not found");
    }

    void clear() {
        for (std::size_t i = 0; i < TABLE_SIZE; ++i) {
            Node* current = table[i];
            while (current) {
                Node* to_delete = current;
                current = current->next;
                free(to_delete);
            }
            table[i] = nullptr;
        }
    }

    Iterator end() const {
        return Iterator(nullptr);
    }
};