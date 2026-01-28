#include "client_manager.h"
#include <stdexcept>

// constructor with db connection
ClientManager::ClientManager(const std::string& connection_string)
{
    try
    {
        conn = std::make_shared<pqxx::connection>(connection_string);
        if (!conn->is_open()) {
            throw std::runtime_error("Cannot connect to database");
        }
        std::cout << "Connected!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Connection error: " << e.what() << std::endl;
        throw;
    }
}

// create client table
void ClientManager::createClientsTable()
{
    try
    {
        pqxx::work txn(*conn);
        txn.exec(
            "CREATE TABLE IF NOT EXISTS clients ("
            "id SERIAL PRIMARY KEY,"
            "first_name VARCHAR(50) NOT NULL,"
            "last_name VARCHAR(50) NOT NULL,"
            "email VARCHAR(100) UNIQUE NOT NULL,"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ")"
        );
        txn.commit();
        std::cout << "Table clients created!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Some error of client table creating happened: " << e.what() << std::endl;
        throw;
    }
}

// create phone table
void ClientManager::createPhonesTable()
{
    try
    {
        pqxx::work txn(*conn);
        txn.exec(
            "CREATE TABLE IF NOT EXISTS client_phones ("
            "id SERIAL PRIMARY KEY,"
            "client_id INTEGER NOT NULL,"
            "phone_number VARCHAR(20) NOT NULL,"
            "FOREIGN KEY (client_id) REFERENCES clients(id) ON DELETE CASCADE,"
            "CONSTRAINT unique_client_phone UNIQUE(client_id, phone_number)"
            ")"
        );
        txn.commit();
        std::cout << "Table client_phones created!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Some error of client_phones table creating happened: " << e.what() << std::endl;
        throw;
    }
}

// create db structure
void ClientManager::createDatabaseStructure()
{
    createClientsTable();
    createPhonesTable();
}

// add a new client
int ClientManager::addClient(const std::string& first_name, const std::string& last_name, const std::string& email)
{
    try
    {
        pqxx::work txn(*conn);

        // check if a client with such email exists
        pqxx::result r = txn.exec_params(
            "SELECT id FROM clients WHERE email = $1",
            email
        );

        if (!r.empty())
        {
            throw std::runtime_error("A client with email " + email + " already exists!");
        }

        // add a new client
        r = txn.exec_params(
            "INSERT INTO clients (first_name, last_name, email) "
            "VALUES ($1, $2, $3) RETURNING id",
            first_name, last_name, email
        );

        int client_id = r[0][0].as<int>();
        txn.commit();

        std::cout << "Client added!" << std::endl;
        return client_id;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Creating client error: " << e.what() << std::endl;
        throw;
    }
}

// add phone
void ClientManager::addPhone(int client_id, const std::string& phone_number)
{
    if (!clientExists(client_id))
    {
        throw std::runtime_error("Client with ID " + std::to_string(client_id) + " does not exist");
    }

    try
    {
        pqxx::work txn(*conn);

        // check if the client already have such phone number
        pqxx::result r = txn.exec_params(
            "SELECT id FROM client_phones WHERE client_id = $1 AND phone_number = $2",
            client_id, phone_number
        );

        if (!r.empty())
        {
            throw std::runtime_error("The client already have such phone number");
        }

        // add a new phone number
        txn.exec_params(
            "INSERT INTO client_phones (client_id, phone_number) "
            "VALUES ($1, $2)",
            client_id, phone_number
        );

        txn.commit();
        std::cout << "Phone number is added" << std::endl;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Some error happened: " << e.what() << std::endl;
        throw;
    }
}

// get client phones
std::vector<PhoneNumber> ClientManager::getClientPhones(int client_id)
{
    std::vector<PhoneNumber> phones;

    try
    {
        pqxx::work txn(*conn);
        pqxx::result r = txn.exec_params(
            "SELECT id, client_id, phone_number FROM client_phones WHERE client_id = $1",
            client_id
        );

        for (const auto& row : r)
        {
            PhoneNumber phone;
            phone.id = row[0].as<int>();
            phone.client_id = row[1].as<int>();
            phone.number = row[2].as<std::string>();
            phones.push_back(phone);
        }

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error of phone receiving: " << e.what() << std::endl;
    }

    return phones;
}

// update client
void ClientManager::updateClient(int client_id, const std::string& new_first_name, const std::string& new_last_name, const std::string& new_email)
{
    if (!clientExists(client_id))
    {
        throw std::runtime_error("Client with ID " + std::to_string(client_id) + " does not exist");
    }

    try
    {
        pqxx::work txn(*conn);

        std::string query = "UPDATE clients SET ";
        std::vector<std::string> updates;
        std::vector<std::string> params;
        int param_count = 1;

        // first name
        if (!new_first_name.empty())
        {
            updates.push_back("first_name = $" + std::to_string(param_count++));
            params.push_back(new_first_name);
        }
        // last name
        if (!new_last_name.empty())
        {
            updates.push_back("last_name = $" + std::to_string(param_count++));
            params.push_back(new_last_name);
        }
        // new email
        if (!new_email.empty())
        {
            // check if there is client with such email
            pqxx::result r = txn.exec_params(
                "SELECT id FROM clients WHERE email = $1 AND id != $2",
                new_email, client_id
            );

            if (!r.empty())
            {
                throw std::runtime_error("Email " + new_email + " is already used by other client");
            }

            updates.push_back("email = $" + std::to_string(param_count++));
            params.push_back(new_email);
        }

        if (updates.empty())
        {
            std::cout << "data is the same" << std::endl;
            return;
        }

        query += pqxx::separated_list(", ", updates.begin(), updates.end());
        query += " WHERE id = $" + std::to_string(param_count);
        params.push_back(std::to_string(client_id));

        txn.exec_params(query, pqxx::prepare::make_dynamic_params(params));
        txn.commit();

        std::cout << "Data updated" << std::endl;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error of data updating: " << e.what() << std::endl;
        throw;
    }
}

// remove phone
void ClientManager::deletePhone(int phone_id)
{
    try {
        pqxx::work txn(*conn);

        pqxx::result r = txn.exec_params(
            "DELETE FROM client_phones WHERE id = $1 RETURNING id",
            phone_id
        );

        if (r.affected_rows() > 0)
        {
            txn.commit();
            std::cout << "Phone removed" << std::endl;
        }
        else
        {
            std::cout << "Such phone does not exist" << std::endl;
        }

    }
    catch (const std::exception& e)
    {
        std::cerr << "Remove phone error: " << e.what() << std::endl;
        throw;
    }
}

// remove client
void ClientManager::deleteClient(int client_id)
{
    if (!clientExists(client_id))
    {
        throw std::runtime_error("Client with ID " + std::to_string(client_id) + " does not exist");
    }

    try
    {
        pqxx::work txn(*conn);

        pqxx::result r = txn.exec_params(
            "DELETE FROM clients WHERE id = $1",
            client_id
        );

        txn.commit();
        std::cout << "Client and his/her phones was removed" << std::endl;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Remove client error: " << e.what() << std::endl;
        throw;
    }
}

// find clients
std::vector<Client> ClientManager::findClients(const std::string& search_term)
{
    std::vector<Client> clients;

    try
    {
        pqxx::work txn(*conn);

        std::string search_pattern = "%" + search_term + "%";

        pqxx::result r = txn.exec_params(
            "SELECT DISTINCT c.id, c.first_name, c.last_name, c.email "
            "FROM clients c "
            "LEFT JOIN client_phones cp ON c.id = cp.client_id "
            "WHERE c.first_name ILIKE $1 "
            "OR c.last_name ILIKE $1 "
            "OR c.email ILIKE $1 "
            "OR cp.phone_number ILIKE $1 "
            "ORDER BY c.id",
            search_pattern
        );

        for (const auto& row : r)
        {
            Client client;
            client.id = row[0].as<int>();
            client.first_name = row[1].as<std::string>();
            client.last_name = row[2].as<std::string>();
            client.email = row[3].as<std::string>();
            client.phones = getClientPhones(client.id);

            clients.push_back(client);
        }

    }
    catch (const std::exception& e)
    {
        std::cerr << "Search clients error: " << e.what() << std::endl;
    }

    return clients;
}

// get client by its id
Client ClientManager::getClientById(int client_id)
{
    Client client;

    try
    {
        pqxx::work txn(*conn);

        pqxx::result r = txn.exec_params(
            "SELECT id, first_name, last_name, email FROM clients WHERE id = $1",
            client_id
        );

        if (!r.empty())
        {
            client.id = r[0][0].as<int>();
            client.first_name = r[0][1].as<std::string>();
            client.last_name = r[0][2].as<std::string>();
            client.email = r[0][3].as<std::string>();
            client.phones = getClientPhones(client_id);
        }

    }
    catch (const std::exception& e)
    {
        std::cerr << "Get client by id error: " << e.what() << std::endl;
    }

    return client;
}

// get all clients
std::vector<Client> ClientManager::getAllClients()
{
    std::vector<Client> clients;

    try
    {
        pqxx::work txn(*conn);

        pqxx::result r = txn.exec(
            "SELECT id, first_name, last_name, email FROM clients ORDER BY id"
        );

        for (const auto& row : r)
        {
            Client client;
            client.id = row[0].as<int>();
            client.first_name = row[1].as<std::string>();
            client.last_name = row[2].as<std::string>();
            client.email = row[3].as<std::string>();
            client.phones = getClientPhones(client.id);

            clients.push_back(client);
        }

    }
    catch (const std::exception& e)
    {
        std::cerr << "Some error happened: " << e.what() << std::endl;
    }

    return clients;
}

// check if client with such id exist
bool ClientManager::clientExists(int client_id)
{
    try
    {
        pqxx::work txn(*conn);

        pqxx::result r = txn.exec_params(
            "SELECT 1 FROM clients WHERE id = $1",
            client_id
        );

        return !r.empty();

    }
    catch (const std::exception& e)
    {
        std::cerr << "Some erorr happened: " << e.what() << std::endl;
        return false;
    }
}