#include "../include/user_service.h"
#include <mysqlx/xdevapi.h>
#include <stdexcept>

UserService::UserService()
    : session("localhost", 33060, "root", "4662Azraelde."),
      schema(session.getSchema("gtuverse_db")),
      usersTable(schema.getTable("users"))
{}

bool UserService::registerUser(const std::string& username, const std::string& password) {
    try {
        usersTable.insert("username", "password").values(username, password).execute();
        return true;
    } catch (const mysqlx::Error &e) {
        return false;  // muhtemelen username zaten var
    }
}

std::optional<User> UserService::loginUser(const std::string& username, const std::string& password) {
    try {
        auto result = usersTable.select("id", "username", "password")
            .where("username = :un AND password = :pw")
            .bind("un", username)
            .bind("pw", password)
            .execute();

        for (auto row : result) { 
            User user;
            user.id = row[0].get<int>();
            user.username = row[1].get<std::string>();
            user.password = row[2].get<std::string>();
            return user;
        }
    } catch (...) {}
    return std::nullopt;
}



std::vector<User> UserService::getAllUsers() {
    std::vector<User> resultList;
    try {
        auto result = usersTable.select("id", "username", "password").execute();
        for (auto row : result) {
            User user;
            user.id = row[0].get<int>();
            user.username = row[1].get<std::string>();
            user.password = row[2].get<std::string>();
            resultList.push_back(user);
        }
    } catch (...) {}
    return resultList;
}

