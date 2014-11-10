#include <iostream>
#include <vector>
#include <pqxx/pqxx>
#include <boost/any.hpp>
#include "sql.h"

typedef struct EmpData {
    std::string name;
    int emp_id;
} EmpData;

namespace queries {
    static const std::string emp_update_ = 
                "UPDATE employee "
                "SET name = $1 "
                "WHERE emp_id = $2;";

    static const std::string emp_insert_ = 
                "INSERT INTO employee "
                "VALUES($1, $2);";
};

class EndpointData
{
public:
    boost::any& get_value(){return value_;}
    EndpointData(){}
private:
    template<typename T> friend
    EndpointData create_endpoint_data(T& val);

    EndpointData(const boost::any& val, int j):value_(val){}

    boost::any value_;
};

template<typename T>
static inline
EndpointData create_endpoint_data(T& val)
{
    return EndpointData(val, 5);
}

int
update(pqxx::work* txn, std::string q_id, std::string name, int emp_id) {
    try {
        pqxx::result r = txn->prepared(q_id)(name)(emp_id).exec();
        int i = r.affected_rows();
        std::cout << "Res = " << i << std::endl;
        return i;
    }
    catch (const pqxx::pqxx_exception& e){
        std::cerr << "Failed to exec update query"
            << e.base().what() << std::endl;
        return 0;
    }
}

int
insert(pqxx::work* txn, std::string q_id, std::string name, int emp_id) {
        try {
                    pqxx::result r = txn->prepared(q_id)(name)(emp_id).exec();
                    int i = r.inserted_oid();
                    std::cout << "Res = " << i << std::endl;
                    return i;
                                }
            catch (const pqxx::pqxx_exception& e){
                        std::cerr << "Failed to exec insert query" 
                                        << e.base().what() << std::endl;
                                return 0;
                                    }
}

int main() {
    pqxx::connection conn("dbname=test user=postgres");
    pqxx::work* txn = NULL;
    txn = new pqxx::work(conn);
    bool recs_ = true;

    if(!txn){
        std::cerr << "Error while creating transaction" << std::endl;
        return 1;
    }

    // 1. Select Operation On DB
    pqxx::result res = txn->exec(
            "SELECT name, emp_id "
            "FROM employee"
            );

    if(res.size() < 1){
        std::cerr << "No records found in employee table" << std::endl;
        recs_ = false;
    }

    if(recs_) {
        int emp_id = res[0][1].as<int>();
        std::cout << "Employee ID fetched is : " << emp_id << std::endl;
    }
    //txn->commit();
    //delete txn;

    // 2. Update/Insert On DB
    txn = NULL;
    txn = new pqxx::work(conn);
    recs_ = false;
    EmpData e1 = {"Aru32", 1};
    //conn.prepare("p1", queries::emp_update_)("text")("integer");
    //conn.prepare("p2", queries::emp_insert_)("text")("integer");
    conn.prepare("p1", queries::emp_update_);
    conn.prepare("p2", queries::emp_insert_);

    if(!update(txn, "p1", e1.name, e1.emp_id)){
        if(!insert(txn, "p2", e1.name, e1.emp_id)){
            std::cerr << "Both insert and update failed" <<std::endl;
        }
    }
    pqxx::prepare::invocation Inv = txn->prepared("p1");
    Inv(e1.name)(e1.emp_id);
    Inv.exec();

    //txn->commit();
    //txn = new pqxx::work(conn);


    std::string parsed("NBTTECH\\am''u'ra'lid'haran''dsouza'\\ahem$%^&*()!@#%~\"     `:;,./<>+=-_|{}[]");

    /*
    std::size_t pos  = parsed.find_first_of('\\');
    if(pos != std::string::npos){
        parsed.insert(pos, 1, '\\');
        std::cout << "Changed string = " << parsed << std::endl;
    }
    */
    std::size_t pos = parsed.find_first_of("\\\'");
    while(pos != std::string::npos) {
        parsed.insert(pos, 1, '\\');
        pos = parsed.find_first_of("\\\'", pos+2);
    }
    std::vector<EndpointData> info;
    info.push_back(create_endpoint_data(parsed));

    std::string new_str = boost::any_cast<std::string>(info[0].get_value());
    std::cout << "New str = " << new_str << std::endl;

    std::stringstream q;
    q << "Update employee set ";
    q << "name = E'" << boost::any_cast<std::string>(info[0].get_value()) << "'";
    q << " where emp_id = 1;" ;

    std::cout << q.str() << std::endl;
    bool tx_failed = false;

    try{
        pqxx::result r = txn->exec(q.str().c_str());
        std::cout << "OID = " << r.inserted_oid() << std::endl;
    }catch(const pqxx::pqxx_exception& e){
        std::cerr << "Failed new insert/update: " << e.base().what() <<  std::endl;
        tx_failed = true;
    }
    std::cout << "Are we here ? " << std::endl;
    if(tx_failed) {
        delete txn;
        txn = NULL;
        txn = new pqxx::work(conn);
    }

    try{
        pqxx::result r = txn->exec("update employee set name = 'WAW' where emp_id = 2");
    }catch(const pqxx::pqxx_exception& e){
        std::cerr << "Failed new insert/update 2: " << e.base().what() <<  std::endl;
    }
    std::cout << "Are we here again? " << std::endl;

    txn->commit();
    delete txn;

    return 0;
}
