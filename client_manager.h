#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

// structure to store phone number of a client
struct PhoneNumber
{
    int id;
    int client_id;
    std::string number;
};

// structure to store data about clients
struct Client
{
    int id;
    std::string first_name;
    std::string last_name;
    std::string email;
    std::vector<PhoneNumber> phones;
};

class ClientManager
{
private:
    std::shared_ptr<pqxx::connection> conn;

    // method to create client table
    void createClientsTable();
    // method to create phone table
    void createPhonesTable();
    // method to get client phones
    std::vector<PhoneNumber> getClientPhones(int client_id);

public:
    ClientManager(const std::string& connection_string);

    // method to create db structure
    void createDatabaseStructure();
    // method to add a new client
    int addClient(const std::string& first_name, const std::string& last_name, const std::string& email);
    // method to add a new phone to client
    void addPhone(int client_id, const std::string& phone_number);
    // method to update client data
    void updateClient(int client_id, const std::string& new_first_name = "", const std::string& new_last_name = "", const std::string& new_email = "");
    // method to remode a phone
    void deletePhone(int phone_id);
    //method to remove a client
    void deleteClient(int client_id);
    // method to find clients by given data
    std::vector<Client> findClients(const std::string& search_term);

    // additional methods

    // method to get a client by its id
    Client getClientById(int client_id);
    // method to get all clients
    std::vector<Client> getAllClients();
    // method to check if there is a client with such id
    bool clientExists(int client_id);
};

#endif
