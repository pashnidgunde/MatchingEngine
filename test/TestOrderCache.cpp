#include "OrderCacheImpl.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class OrderCacheTest : public ::testing::Test {
    

protected:
    void SetUp() override {
        
    }

    void TearDown() override {
    }

    std::unique_ptr<OrderCacheInterface> orderCache = std::make_unique<OrderCacheImpl>();
};

TEST_F(OrderCacheTest, IsEmptyInitially) {
    EXPECT_EQ(orderCache->getAllOrders().size(), 0);
    EXPECT_EQ(orderCache->getMatchingSizeForSecurity("Any"), 0);
}

TEST_F(OrderCacheTest, addOrder) {
    Order o("Any", "", "", 0, "", "");
    orderCache->addOrder(o);
    auto orders = orderCache->getAllOrders();
    EXPECT_EQ(orders.size(), 1);
    auto& oa = orders[0];
    EXPECT_EQ(oa, o);
}

TEST_F(OrderCacheTest, cancelOrder) {
    Order o("1", "", "", 0, "", "");
    Order o2("2", "", "", 0, "", "");
    orderCache->addOrder(o);
    orderCache->addOrder(o2);
    auto orders = orderCache->getAllOrders();
    EXPECT_EQ(orders.size(), 2);

    orderCache->cancelOrder(o2.orderId());

    orders = orderCache->getAllOrders();
    EXPECT_EQ(orders.size(), 1);
    auto& oa = orders[0];
    EXPECT_EQ(oa, o);
}


TEST_F(OrderCacheTest, cancelOrderByUser) {
    Order o("1", "", "", 0, "u1", "");
    Order o2("2", "", "", 0, "u2", "");
    Order o3("3", "", "", 0, "u1", "");
    Order o4("4", "", "", 0, "u2", "");

    orderCache->addOrder(o);
    orderCache->addOrder(o2);
    orderCache->addOrder(o3);
    orderCache->addOrder(o4);

    orderCache->cancelOrdersForUser(o2.user());

    auto orders = orderCache->getAllOrders();
    EXPECT_EQ(orders.size(), 2);
    auto& oa = orders[0];
    auto& oa3 = orders[1];
    EXPECT_EQ(oa, o);
    EXPECT_EQ(oa3, o3);
}

TEST_F(OrderCacheTest, cancelOrdersForSecIdWithMinimumQty) {
    Order o("1", "IBM", "", 1200, "u1", "");
    Order o2("2","GOOG", "", 500, "u2", "");
    Order o3("3", "IBM", "", 500, "u1", "");
    Order o4("4", "GOOG", "", 4000, "u2", "");

    orderCache->addOrder(o);
    orderCache->addOrder(o2);
    orderCache->addOrder(o3);
    orderCache->addOrder(o4);

    orderCache->cancelOrdersForSecIdWithMinimumQty("NON_EXISTING", 100);
    EXPECT_EQ(orderCache->getAllOrders().size(), 4);

    orderCache->cancelOrdersForSecIdWithMinimumQty("IBM", 1500);
    EXPECT_EQ(orderCache->getAllOrders().size(), 4);

    orderCache->cancelOrdersForSecIdWithMinimumQty("IBM", 1200);
    EXPECT_EQ(orderCache->getAllOrders().size(), 3);

    orderCache->cancelOrdersForSecIdWithMinimumQty("IBM", 600);
    EXPECT_EQ(orderCache->getAllOrders().size(), 3);

    orderCache->cancelOrdersForSecIdWithMinimumQty("GOOG", 400);
    EXPECT_EQ(orderCache->getAllOrders().size(), 1);

    orderCache->cancelOrdersForSecIdWithMinimumQty("IBM", 0);
    EXPECT_EQ(orderCache->getAllOrders().size(), 0);

}

TEST_F(OrderCacheTest, getMatchingSizeForSecurity) {
    Order o("OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA");
    Order o1("OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB");
    Order o2("OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA");
    Order o3("OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC");
    Order o4("OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB");
    Order o5("OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD");
    Order o6("OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE");
    Order o7("OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE");
    
    orderCache->addOrder(o);
    orderCache->addOrder(o1);
    orderCache->addOrder(o2);
    orderCache->addOrder(o3);
    orderCache->addOrder(o4);
    orderCache->addOrder(o5);
    orderCache->addOrder(o6);
    orderCache->addOrder(o7);

    EXPECT_EQ(orderCache->getMatchingSizeForSecurity("SecId2"), 2700);
}

TEST_F(OrderCacheTest, getMatchingSizeForSecurity1) {
    Order o1("OrdId1", "SecId1", "Sell", 100, "User10", "Company2");
    Order o2("OrdId2", "SecId3", "Sell", 200, "User8", "Company2");
    Order o3("OrdId3", "SecId1", "Buy", 300, "User13", "Company2");
    Order o4("OrdId4", "SecId2", "Sell", 400, "User12", "Company2");
    Order o5("OrdId5", "SecId3", "Sell", 500, "User7", "Company2");
    Order o6("OrdId6", "SecId3", "Buy", 600, "User3", "Company1");
    Order o7("OrdId7", "SecId1", "Sell", 700, "User10", "Company2");
    Order o8("OrdId8", "SecId1", "Sell", 800, "User2", "Company1");
    Order o9("OrdId9", "SecId2", "Buy", 900, "User6", "Company2");
    Order o10("OrdId10", "SecId2", "Sell", 1000, "User5", "Company1");
    Order o11("OrdId11", "SecId1", "Sell", 1100, "User13", "Company2");
    Order o12("OrdId12", "SecId2", "Buy", 1200, "User9", "Company2");
    Order o13("OrdId13", "SecId1", "Sell", 1300, "User1", "Company");


    orderCache->addOrder(o1);
    orderCache->addOrder(o2);
    orderCache->addOrder(o3);
    orderCache->addOrder(o4);
    orderCache->addOrder(o5);
    orderCache->addOrder(o6);
    orderCache->addOrder(o7);
    orderCache->addOrder(o8);
    orderCache->addOrder(o9);
    orderCache->addOrder(o10);
    orderCache->addOrder(o11);
    orderCache->addOrder(o12);
    orderCache->addOrder(o13);


    EXPECT_EQ(orderCache->getMatchingSizeForSecurity("SecId1"), 300);
    EXPECT_EQ(orderCache->getMatchingSizeForSecurity("SecId2"), 1000);
    EXPECT_EQ(orderCache->getMatchingSizeForSecurity("SecId3"), 600);
}

TEST_F(OrderCacheTest, getMatchingSizeForSecurity2) {
        
    Order o1("OrdId1", "SecId1", "Sell", 100, "User1", "Company1");
    Order o2("OrdId2", "SecId3", "Sell", 200, "User3", "Company2");
    Order o3("OrdId3", "SecId1", "Buy", 300, "User2", "Company1");
    Order o4("OrdId4", "SecId3", "Sell", 400, "User5", "Company2");
    Order o5("OrdId5", "SecId2", "Sell", 500, "User2", "Company1");
    Order o6("OrdId6", "SecId2", "Buy", 600, "User3", "Company2");
    Order o7("OrdId7", "SecId2", "Sell", 700, "User1", "Company1");
    Order o8("OrdId8", "SecId1", "Sell", 800, "User2", "Company1");
    Order o9("OrdId9", "SecId1", "Buy", 900, "User5", "Company2");
    Order o10("OrdId10", "SecId1", "Sell", 1000, "User1", "Company1");
    Order o11("OrdId11", "SecId2", "Sell", 1100, "User6", "Company2");
    

    orderCache->addOrder(o1);
    orderCache->addOrder(o2);
    orderCache->addOrder(o3);
    orderCache->addOrder(o4);
    orderCache->addOrder(o5);
    orderCache->addOrder(o6);
    orderCache->addOrder(o7);
    orderCache->addOrder(o8);
    orderCache->addOrder(o9);
    orderCache->addOrder(o10);
    orderCache->addOrder(o11);
    
    EXPECT_EQ(orderCache->getMatchingSizeForSecurity("SecId1"), 900);
    EXPECT_EQ(orderCache->getMatchingSizeForSecurity("SecId2"), 600);
    EXPECT_EQ(orderCache->getMatchingSizeForSecurity("SecId3"), 0);
}



