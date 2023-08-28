#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>

#include "OrderCache.h"
#include "MatchingEngine.h"
#include <unordered_set>

#include <type_traits>

// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed. 
class OrderCacheImpl : public OrderCacheInterface
{
public:

    // add order to the cache
    void addOrder(Order order) override {
        auto& orders_by_orderid = m_orders.get<OrderIdTag>();
        orders_by_orderid.insert(order);
    }

    // remove order with this unique order id from the cache
    void cancelOrder(const std::string& orderId) override {
        auto &orders_by_orderid = m_orders.get<OrderIdTag>();
        orders_by_orderid.erase(orderId);
    }

    // remove all orders in the cache for this user
    void cancelOrdersForUser(const std::string& user) override {
        auto& orders_by_userid = m_orders.get<UserIdTag>();
        orders_by_userid.erase(user);
    }

    // remove all orders in the cache for this security with qty >= minQty
    void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) override {
        
        auto satisfiesCancelCondition = [](unsigned int qty, unsigned int minQty) {
            return qty >= minQty;
        };
        
        std::unordered_set<std::string> orderIds;
        auto range = m_orders.get<SecurityIdTag>().equal_range(securityId);
        for (auto begin = range.first; begin != range.second; begin++) {
            if (satisfiesCancelCondition(begin->qty(), minQty)) {
                orderIds.insert(begin->orderId());
            }
        }
        
        auto& orders_by_orderId = m_orders.get<OrderIdTag>();
        for (const auto& orderId : orderIds) {
            orders_by_orderId.erase(orderId);
        }
    }

    // return the total qty that can match for the security id
    unsigned int getMatchingSizeForSecurity(const std::string& securityId) override {
        auto buy_range = m_orders.get<SecurityIdSideTag>().equal_range(std::make_tuple(securityId, SIDE_BUY));
        auto sell_range = m_orders.get<SecurityIdSideTag>().equal_range(std::make_tuple(securityId, SIDE_SELL));

        auto shouldFilter = [](const auto& buy, const auto& sell) {
            return buy.company() == sell.company();
        };

        auto sweeped = [](const auto& order) {
            return order.qty() == 0;
        };

        auto matchingQty = [&](const auto& buy, const auto& sell) {
            return shouldFilter(buy, sell) ? 0 : std::min(buy.qty(), sell.qty());
        };

        /*struct newQty {
            explicit newQty(unsigned int qty) : m_qty(qty) {}
            unsigned int m_qty = 0;
            void operator()(Order& o) {
                o.setQty(o.qty() - m_qty);
            }
        };*/

        auto adjustQty = [&](const auto& order, unsigned int matchedQty) {
            auto& orders_by_orderid = m_orders.get<OrderIdTag>();
            auto it = orders_by_orderid.find(order.orderId());
            const_cast<Order&>(order).setQty(order.qty() - matchedQty);
            //orders_by_orderid.modify(it, newQty(order.qty() - matchedQty));
        };

        unsigned int total_matched = 0;

        std::unordered_set<std::string> sweepedOrderIds;
        for (auto buyIter = buy_range.first; buyIter != buy_range.second; buyIter++) {
            auto& buy = *buyIter;
            for (auto sellIter = sell_range.first; sellIter != sell_range.second; sellIter++) {
                auto& sell = *sellIter;
                auto matchedQty = matchingQty(*buyIter, *sellIter);

                if (matchedQty == 0) {
                    continue;
                }

                total_matched += matchedQty;
                adjustQty(buy, matchedQty);
                adjustQty(sell, matchedQty);

                if (sweeped(buy)) {
                    sweepedOrderIds.insert(buy.orderId());
                    break;
                }

                if (sweeped(sell)) {
                    sweepedOrderIds.insert(sell.orderId());
                }
            }
        }

        auto& orders_by_orderid = m_orders.get<OrderIdTag>();
        for (const auto& sweepedOrder : sweepedOrderIds) {
            orders_by_orderid.erase(sweepedOrder);
        }
        
        return total_matched;
    }

    // return all orders in cache in a vector
    std::vector<Order> getAllOrders() const override {
        std::vector<Order> r;
        auto& idx = m_orders.get<OrderIdTag>();
        r.reserve(idx.size());
        for (auto begin = idx.begin(); begin != idx.end(); begin++) {
            r.emplace_back(*begin);
        }
        return r;
    }

private:
    
    struct OrderIdTag;
    struct UserIdTag;
    struct SecurityIdTag;
    struct SecurityIdSideTag;

    using OrderIdType = std::invoke_result_t<decltype(&Order::orderId),Order>;
    static_assert(std::is_same_v<OrderIdType, std::string>);

    using UserIdType = std::invoke_result_t<decltype(&Order::user), Order>;
    static_assert(std::is_same_v<UserIdType, std::string>);

    using SecurityIdType = std::invoke_result_t<decltype(&Order::securityId), Order>;
    static_assert(std::is_same_v<SecurityIdType, std::string>);

    using SideType = std::invoke_result_t<decltype(&Order::side), Order>;
    static_assert(std::is_same_v<SideType, std::string>);
    
    using CompanyType = std::invoke_result_t<decltype(&Order::company), Order>;
    static_assert(std::is_same_v<CompanyType, std::string>);


    inline static const SideType SIDE_BUY = "Buy";
    inline static const SideType SIDE_SELL = "Sell";

    using Orders = boost::multi_index_container<
        Order,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<OrderIdTag>,
                boost::multi_index::const_mem_fun<Order, OrderIdType, &Order::orderId>
            >,
            boost::multi_index::hashed_non_unique<
                boost::multi_index::tag<UserIdTag>,
                boost::multi_index::const_mem_fun<Order, UserIdType, &Order::user>
            >,
            boost::multi_index::hashed_non_unique<
                boost::multi_index::tag<SecurityIdTag>,
                boost::multi_index::const_mem_fun<Order, SecurityIdType, &Order::securityId>
            >,
            boost::multi_index::hashed_non_unique<
                boost::multi_index::tag<SecurityIdSideTag>,
                boost::multi_index::composite_key<
                    Order,
                    boost::multi_index::const_mem_fun<Order, SecurityIdType, &Order::securityId>,
                    boost::multi_index::const_mem_fun<Order, SideType, &Order::side>
                >
            >
        >
    >;
    
    Orders m_orders;

};