#include <iostream>
#include <boost/format.hpp>
#include <pqxx/pqxx>
#include "sql.h"

namespace QUERY {
    static const std::string DROP_TABLE = 
        "DROP TABLE IF EXISTS %1%;";
};

void
test_case1(pqxx::work* txn)
{
    std::string query1((boost::format(QUERY::DROP_TABLE)%"deltas_test_table").str());
    std::string query2((boost::format(QUERY::DROP_TABLE)%"deltas_test_table_agg").str());
    try
    {
        txn->exec(query1.c_str());
    }catch(const pqxx::pqxx_exception& e){
        std::cerr << "Failed in query1: " << e.base().what() <<  std::endl;
    }
    txn->commit();
    try
    {
        txn->exec(query2.c_str());
    }
    catch(const pqxx::pqxx_exception& e){
        std::cerr << "Failed in query2: " << e.base().what() <<  std::endl;
    }

    return;
}

int main()
{
    pqxx::connection conn("dbname=test user=postgres");
    pqxx::work* txn = NULL;
    txn = new pqxx::work(conn);

    if (!txn){
        std::cerr << "Error while creating transaction" << std::endl;
        return 1;
    }

    test_case1(txn);

    delete txn;
    return 0;
}
