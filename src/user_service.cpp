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
        auto result = usersTable.select("username", "password")
            .where("username = :un AND password = :pw")
            .bind("un", username)
            .bind("pw", password)
            .execute();

        if (result.count() > 0) {
            return User{username, password};
        }
    } catch (...) {}
    return std::nullopt;
}

std::vector<User> UserService::getAllUsers() {
    std::vector<User> resultList;
    try {
        auto result = usersTable.select("username", "password").execute();
        for (auto row : result) {
            resultList.push_back({row[0].get<std::string>(), row[1].get<std::string>()});
        }
    } catch (...) {}
    return resultList;
}
