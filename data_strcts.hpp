#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

struct LevelStr {
    std::string price; // e.g. "89595.90000000"
    std::string qty;   // e.g. "1.91037000"
};

struct DepthUpdate {
    std::string e;
    long long E;
    long long T;
    std::string s;
    long long U;
    long long u;
    long long pu;
    std::vector<LevelStr> b;
    std::vector<LevelStr> a;
};

class OrderBook {
public:
    using SideMap = std::unordered_map<std::string, std::string>; // price -> qty (strings)
    
    std::int64_t lastUpdateId{};
    SideMap bids;  // price -> qty
    SideMap asks;  // price -> qty

    explicit OrderBook(const std::string& json_text) {
        using nlohmann::json;

        json j;
        try {
            j = json::parse(json_text);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("JSON parse failed: ") + e.what());
        }

        if (!j.contains("lastUpdateId") || !j.contains("bids") || !j.contains("asks")) {
            throw std::runtime_error("JSON missing required fields: lastUpdateId/bids/asks");
        }

        lastUpdateId = j.at("lastUpdateId").get<std::int64_t>();

        fill_side(bids, j.at("bids"), "bids");
        fill_side(asks, j.at("asks"), "asks");
    }

    const std::string* get_bid_qty(const std::string& price) const {
        auto it = bids.find(price);
        return (it == bids.end()) ? nullptr : &it->second;
    }

    const std::string* get_ask_qty(const std::string& price) const {
        auto it = asks.find(price);
        return (it == asks.end()) ? nullptr : &it->second;
    }



private:
    static void fill_side(SideMap& dst, const nlohmann::json& arr, const char* side_name) {
        if (!arr.is_array()) throw std::runtime_error(std::string(side_name) + " is not an array");

        for (const auto& lvl : arr) {
            // Each level should be ["price", "qty"]
            if (!lvl.is_array() || lvl.size() < 2){
                throw std::runtime_error(std::string("Bad level format in ") + side_name);
            }

            std::string price = as_string(lvl.at(0));
            std::string qty = as_string(lvl.at(1));
            dst[price] = qty;
        }
    }
    static std::string as_string(const nlohmann::json& x) {
        if (x.is_string()) return x.get<std::string>();
        if (x.is_number_float() || x.is_number_integer() || x.is_number_unsigned()) return x.dump();
        throw std::runtime_error("Expected string/number for price/qty");
    }
};
