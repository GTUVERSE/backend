#include <iostream> 
#include "../include/user_service.h"
#include <mysqlx/xdevapi.h>
#include <stdexcept>
   

UserService::UserService()
    : session("173.212.195.170", 33060, "root", "funda123"),
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
std::optional<User> UserService::getUserByUsername(const std::string& username) {
    try {
        auto result = usersTable
            .select("id", "username", "email", "password")
            .where("username = :un")
            .bind("un", username)
            // you can also add .limit(1) here if you like
            .execute();

        // fetch exactly one row
        auto row = result.fetchOne();
        if (!row) {
            // no matching username
            return std::nullopt;
        }

        User u;
        u.id       = row[0].get<int>();
        u.username = row[1].get<std::string>();
        // email might be NULL in the DB, guard against that:
        if (!row[2].isNull()) {
            u.email = row[2].get<std::string>();
        } else {
            u.email = "";  
        }
        u.password = row[3].get<std::string>();
        return u;
    }
    catch (const mysqlx::Error &err) {
        std::cerr << "getUserByUsername DB error: " << err.what() << std::endl;
    }
    catch (const std::exception &ex) {
        std::cerr << "getUserByUsername error: " << ex.what() << std::endl;
    }
    return std::nullopt;
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




bool UserService::updateUsername(int userId, const std::string& newUsername) {
    try {
        // Yeni kullanıcı adının benzersiz olup olmadığını kontrol et
        auto checkResult = usersTable.select("id")
                                  .where("username = :username AND id != :userId")
                                  .bind("username", newUsername)
                                  .bind("userId", userId)
                                  .execute();
        
        if (checkResult.count() > 0) {
            // Bu kullanıcı adı başka bir kullanıcı tarafından zaten kullanılıyor
            return false;
        }
        
        // Kullanıcı adını güncelle
        auto result = usersTable.update()
                              .set("username", newUsername)
                              .where("id = :id")
                              .bind("id", userId)
                              .execute();
        
        // Etkilenen satır sayısını kontrol et
        return result.getAffectedItemsCount() > 0;
    } 
    catch (const mysqlx::Error &err) {
        std::cerr << "updateUsername DB error: " << err.what() << std::endl;
        return false;
    }
    catch (const std::exception &ex) {
        std::cerr << "updateUsername error: " << ex.what() << std::endl;
        return false;
    }
}