#include "data_strcts.hpp"

#include <curl/curl.h>
#include <iostream>
#include <string>
#include <stdexcept>

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* s = static_cast<std::string*>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

static std::string http_get(const char* url) {
    CURL* h = curl_easy_init();
    if (!h) throw std::runtime_error("curl_easy_init failed");

    std::string body;
    curl_easy_setopt(h, CURLOPT_URL, url);
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, &body);

    CURLcode rc = curl_easy_perform(h);
    curl_easy_cleanup(h);

    if (rc != CURLE_OK) throw std::runtime_error(curl_easy_strerror(rc));
    return body;
}

int main() {
    try {
        std::string body = http_get("https://api.binance.com/api/v3/depth?symbol=BTCUSDT&limit=5");
        OrderBook ob (body);

        std::cout << "LastUpdatedID: " <<ob.lastUpdateId << "\n";

        std::cout << "Asks: \n";
        for (const auto& [price, qty] : ob.asks) {
            std::cout << price << "  " << qty << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}