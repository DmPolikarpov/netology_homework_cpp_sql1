#include <iostream>
#include "client_manager.h"

// print client data
void printClient(const Client& client)
{
    std::cout << "ID: " << client.id << std::endl;
    std::cout << "First name: " << client.first_name << std::endl;
    std::cout << "Second name: " << client.last_name << std::endl;
    std::cout << "Email: " << client.email << std::endl;

    if (!client.phones.empty())
    {
        std::cout << "Phone numbers:" << std::endl;
        for (const auto& phone : client.phones)
        {
            std::cout << "  " << phone.number << " (phone ID: " << phone.id << ")" << std::endl;
        }
    }
    else
    {
        std::cout << "Phone numbers: none" << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

// print all clients
void printAllClients(ClientManager& manager)
{
    std::cout << "\n=== ALL CLIENTS ===" << std::endl;
    auto clients = manager.getAllClients();

    if (clients.empty())
    {
        std::cout << "Clients do not exist" << std::endl;
    }
    else
    {
        for (const auto& client : clients)
        {
            printClient(client);
        }
    }
}

int main() {
    try {
        // connect to db
        std::string conn_str = "host=localhost "
            "port=5432 "
            "dbname=clientdb "
            "user=postgres "
            "password=1234567";

        ClientManager manager(conn_str);

        // create db structure
        manager.createDatabaseStructure();

        // add clients
        std::cout << "\n=== ADD CLIENTS ===" << std::endl;
        int client1_id = manager.addClient("Ivan", "Ivanov", "ivan@example.com");
        int client2_id = manager.addClient("Peter", "Petrov", "petr@example.com");
        int client3_id = manager.addClient("Maria", "Sidorova", "maria@example.com");

        // add phones
        std::cout << "\n=== ADD PHONES ===" << std::endl;
        manager.addPhone(client1_id, "+7-911-111-11-11");
        manager.addPhone(client1_id, "+7-911-111-11-12");
        manager.addPhone(client2_id, "+7-922-222-22-22");

        // show all clients
        printAllClients(manager);

        // search for clients
        std::cout << "\n=== Search client by string 'Ivan' ===" << std::endl;
        auto found_clients = manager.findClients("Ivan");
        for (const auto& client : found_clients)
        {
            printClient(client);
        }

        // update client
        std::cout << "\n=== UPDATE CLIENT ===" << std::endl;
        manager.updateClient(client1_id, "IVAN", "IVANOV", "new_ivan@example.com");

        // get client by ID
        std::cout << "\n=== GET CLIENT BY ID ===" << std::endl;
        auto client = manager.getClientById(client1_id);
        printClient(client);

        // remove phone
        std::cout << "\n=== REMOVE PHONE ===" << std::endl;
        manager.deletePhone(2);

        // shoe all clients
        printAllClients(manager);

        // remove client
        std::cout << "\n=== REMOVE CLIENT ===" << std::endl;
        manager.deleteClient(client3_id);

        // show all clients
        printAllClients(manager);

    }
    catch (const std::exception& e)
    {
        std::cerr << "Some error happened: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}