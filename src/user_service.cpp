#include "../include/user_service.h"
#include <mysqlx/xdevapi.h>
#include <stdexcept>

UserService::UserService()
    : session("localhost", 33060, "root", "4662Azraelde."),
      schema(session.getSchema("gtuverse_db")),
      usersTable(schema.getTable("users"))
{}

bool UserService::registerUser(const std::string& username, const std::string& email, const std::string& password) {
    // Kullanıcı adının benzersiz olduğunu kontrol et
    auto result = usersTable.select("id")
                          .where("username = :username")
                          .bind("username", username)
                          .execute();
    
    if (result.count() > 0) {
        return false;  // Kullanıcı adı zaten var
    }
    
    // Yeni kullanıcıyı ekle
    usersTable.insert("username", "email", "password")
              .values(username, email, password)  // email de ekliyoruz
              .execute();
    
    return true;
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

