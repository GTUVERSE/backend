#include "../external/crow/include/crow.h"
#include "../include/user_service.h"
#include "room_service.h"
#include "room.h"
#include "room_user_service.h"
#include <crow.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

RoomService roomService;
RoomUserService roomUserService;

int main() {
    crow::SimpleApp app;
    UserService userService;

    CROW_ROUTE(app, "/register").methods("POST"_method)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }
        
        std::string username = body["username"].s();
        std::string email = body["email"].s();  // Yeni eklenen alan
        std::string password = body["password"].s();
        
        bool ok = userService.registerUser(username, email, password);
        return ok ? crow::response(200, "Registered") : crow::response(409, "Username exists");
    });
    

   /*
    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([&](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        auto user = userService.loginUser(username, password);
        if (user.has_value()) {
            return crow::response(200, "Login successful");
        } else {
            return crow::response(401, "Invalid credentials");
        }
    });*/

   

CROW_ROUTE(app, "/login").methods("POST"_method)
([&](const crow::request& req){
    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);

    std::string username = body["username"].s();
    std::string password = body["password"].s();
    auto optUser = userService.loginUser(username, password);

    if (optUser.has_value()) {
        // Başarılı login → kullanıcı id’sini JSON ile dön
        crow::json::wvalue res;
        res["id"]       = optUser->id;
        res["username"] = optUser->username;
        // eğer email vs. de dönmek istersen ekleyebilirsin
        return crow::response{200, res};
    } else {
        crow::json::wvalue err;
        err["error"] = "Invalid credentials";
        return crow::response{401, err};
    }
});


    CROW_ROUTE(app, "/users").methods("GET"_method)
    ([&](){
        auto users = userService.getAllUsers();
        crow::json::wvalue result;
        for (size_t i = 0; i < users.size(); ++i) {
            result[i]["id"] = users[i].id;
            result[i]["username"] = users[i].username;
        }
        return crow::response(result);
    });



    CROW_ROUTE(app, "/roomsWEB").methods("GET"_method)([](){
        auto rooms = roomService.getAllRooms();
        json j = json::array();
        for (const auto& room : rooms) {
            j.push_back({
                {"id", room.getId()},
                {"name", room.getName()},
                {"size", room.getSize()},
                {"type", room.getType()},
                {"capacity", room.getCapacity()}
            });
        }
        return crow::response{j.dump()};
    });
    
      CROW_ROUTE(app, "/rooms").methods("GET"_method)([](){
        auto rooms = roomService.getAllRooms();
        json j = json::array();
        for (const auto& room : rooms) {
            j.push_back({
                {"id", room.getId()},
                {"name", room.getName()},
                {"size", room.getSize()},
                {"capacity", room.getCapacity()}
            });
        }
        return crow::response{j.dump()};
    });

    CROW_ROUTE(app, "/rooms/<int>").methods("GET"_method)([](int id){
        auto result = roomService.getRoomById(id);
        if (result) {
            json j = {
                {"id", result->getId()},
                {"name", result->getName()},
                {"size", result->getSize()},
                {"capacity", result->getCapacity()}
            };
            return crow::response{j.dump()};
        }
        return crow::response{404};
    });

      CROW_ROUTE(app, "/roomsWEB/<int>").methods("GET"_method)([](int id){
        auto result = roomService.getRoomById(id);
        if (result) {
            json j = {
                {"id", result->getId()},
                {"name", result->getName()},
                {"size", result->getSize()},
                {"type", result->getType()}, // Oda tipi eklendi
                {"capacity", result->getCapacity()}
            };
            return crow::response{j.dump()};
        }
        return crow::response{404};
    });
    
    

    /*CROW_ROUTE(app, "/rooms").methods("POST"_method)([](const crow::request& req){
        auto j = json::parse(req.body);
        std::string name = j["name"];
        int capacity = j["capacity"];
    
        Room newRoom(0, name, capacity); // id 0 çünkü AUTO_INCREMENT
        roomService.createRoom(newRoom);
        return crow::response{201};
    });*/
    
    CROW_ROUTE(app, "/rooms").methods("POST"_method)([](const crow::request& req){
        try {
            auto j = json::parse(req.body);
            std::string name = j["name"];
            std::string type = j["type"]; // Oda tipi eklendi
           // int size = j["size"];
           int size = 0; // Varsayılan boyut
    
            // Aynı isimde oda var mı kontrol et
            if (roomService.roomExistsByName(name)) {
                crow::json::wvalue error;
                error["error"] = "Room with the same name already exists";
                return crow::response{409, error};  // 409 Conflict
            }
    
            Room newRoom(0, name, size,type);
            roomService.createRoom(newRoom);
    
            auto rooms = roomService.getAllRooms();
            const Room& lastRoom = rooms.back();
    
            crow::json::wvalue response;
            response["message"] = "Room created";
            response["id"] = lastRoom.getId();
            response["name"] = lastRoom.getName();
            response["size"] = lastRoom.getSize();
            response["capacity"] = lastRoom.getCapacity();
            response["type"] = lastRoom.getType(); // Oda tipi de eklendi
            return crow::response{201, response};
    
        } catch (const std::exception& e) {
            crow::json::wvalue error;
            error["error"] = e.what();
            return crow::response{500, error};  // Internal Server Error
        }
    });
    

CROW_ROUTE(app, "/rooms/<int>").methods("DELETE"_method)([](int id){
    bool success = roomService.deleteRoom(id);
    return crow::response{success ? 200 : 404};
});

/*
CROW_ROUTE(app, "/rooms/<int>/users").methods("POST"_method)
([&](const crow::request& req, int roomId) {
    try {
        auto j = json::parse(req.body);
        int userId = j["user_id"];
        
        // Kullanıcı zaten odada mı kontrolü
        if (roomUserService.isUserInRoom(roomId, userId)) {
            crow::json::wvalue error;
            error["error"] = "User is already in this room";
            return crow::response(409, error);
        }
        
        // Oda mevcut mu ve dolu mu kontrolü
        auto room = roomService.getRoomById(roomId);
        if (!room.has_value()) {
            crow::json::wvalue error;
            error["error"] = "Room not found";
            return crow::response(404, error);
        }
        
        if (room->getSize() >= room->getCapacity()) {
            crow::json::wvalue error;
            error["error"] = "Room is full";
            return crow::response(409, error);
        }
        
        roomUserService.addUserToRoom(userId, roomId);
        
        crow::json::wvalue success;
        success["message"] = "User added to room";
        success["room_id"] = roomId;
        success["user_id"] = userId;
        success["current_size"] = room->getSize() + 1;  // Güncellenmiş oda boyutu
        
        return crow::response(201, success);
    } catch (const std::exception& e) {
        crow::json::wvalue error;
        error["error"] = e.what();
        return crow::response(500, error);
    }
});*/

CROW_ROUTE(app, "/rooms/<int>/users").methods("POST"_method)
([&](const crow::request& req, int roomId) {
    try {
        auto j = json::parse(req.body);
        int userId = j["user_id"];

        // Kullanıcı var mı kontrolü
        auto userOpt = userService.getUserById(userId);
        if (!userOpt.has_value()) {
            crow::json::wvalue error;
            error["error"] = "User not found";
            return crow::response(404, error);
        }

        // Kullanıcı zaten odada mı?
        if (roomUserService.isUserInRoom(roomId, userId)) {
            crow::json::wvalue error;
            error["error"] = "User is already in this room";
            return crow::response(409, error);
        }

        // Oda kontrolü
        auto roomOpt = roomService.getRoomById(roomId);
        if (!roomOpt.has_value()) {
            crow::json::wvalue error;
            error["error"] = "Room not found";
            return crow::response(404, error);
        }

        Room room = *roomOpt;

        // Kapasite dolu mu?
        if (room.getSize() >= room.getCapacity()) {
            crow::json::wvalue error;
            error["error"] = "Room is full";
            return crow::response(409, error);
        }

       
        roomUserService.addUserToRoom(userId, roomId);

      
        room.setSize(room.getSize() + 1);
        roomService.updateRoom(room);

        crow::json::wvalue success;
        success["message"] = "User added to room";
        success["room_id"] = roomId;
        success["user_id"] = userId;
        success["current_size"] = room.getSize();

        return crow::response(201, success);

    } catch (const std::exception& e) {
        crow::json::wvalue error;
        error["error"] = e.what();
        return crow::response(500, error);
    }
});





CROW_ROUTE(app, "/rooms/<int>/users").methods("GET"_method)
([&](int roomId) {
    try {
        auto users = roomUserService.getUsersWithNamesInRoom(roomId);
        crow::json::wvalue response;

        for (size_t i = 0; i < users.size(); ++i) {
            response[i]["id"] = users[i].first;
            response[i]["username"] = users[i].second;
        }

        return crow::response{response};
    } catch (const std::exception& e) {
        std::cerr << "Error in GET /rooms/<id>/users: " << e.what() << std::endl;
        return crow::response{500, "Internal Server Error"};
    }
});





CROW_ROUTE(app, "/rooms/<int>/users/<int>").methods("DELETE"_method)
([&](int roomId, int userId){
    try {
        // Kullanıcı odada mı kontrolü
        if (!roomUserService.isUserInRoom(roomId, userId)) {
            crow::json::wvalue error;
            error["error"] = "User is not in this room";
            return crow::response(404, error);
        }
        
        bool success = roomUserService.removeUserFromRoom(roomId, userId);
        if (success) {
            auto room = roomService.getRoomById(roomId);
            crow::json::wvalue result;
            result["message"] = "User removed from room";
            result["room_id"] = roomId;
            result["user_id"] = userId;
            if (room.has_value()) {
                result["current_size"] = room->getSize();
            }
            return crow::response(200, result);
        } else {
            crow::json::wvalue error;
            error["error"] = "Failed to remove user from room";
            return crow::response(500, error);
        }
    } catch (const std::exception& e) {
        crow::json::wvalue error;
        error["error"] = e.what();
        return crow::response(500, error);
    }
});


/*

CROW_ROUTE(app, "/users/<int>/rooms").methods("GET"_method)
([&](int userId) {
    try {
        auto rooms = roomUserService.getRoomsForUser(userId);
        crow::json::wvalue response;

        for (size_t i = 0; i < rooms.size(); ++i) {
            response[i]["id"] = rooms[i].getId();
            response[i]["name"] = rooms[i].getName();
            response[i]["size"] = rooms[i].getSize();
        }

        return crow::response{response};
    } catch (const std::exception& e) {
        std::cerr << "Error in GET /users/<id>/rooms: " << e.what() << std::endl;
        return crow::response{500};
    }

});





 CROW_ROUTE(app, "/usersWEB/<int>/rooms").methods("GET"_method)
 ([&](int userId) {
     try {
         auto rooms = roomUserService.getRoomsForUser(userId);
         crow::json::wvalue response;

         for (size_t i = 0; i < rooms.size(); ++i) {
             response[i]["id"]   = rooms[i].getId();
             response[i]["name"] = rooms[i].getName();
     response[i]["size"] = rooms[i].getSize();
         response[i]["type"] = rooms[i].getType();   // ← Oda tipi eklendi
         }

         return crow::response{response};
     } catch (const std::exception& e) {
         std::cerr << "Error in GET /users/<id>/rooms: " << e.what() << std::endl;
         return crow::response{500};
     }
 });



*/


// /usersWEB/<int>/rooms endpoint'i düzeltmesi
CROW_ROUTE(app, "/usersWEB/<int>/rooms").methods("GET"_method)
([&](int userId) {
    try {
        auto rooms = roomUserService.getRoomsForUser(userId);
        crow::json::wvalue response = crow::json::wvalue::list(); // Liste olarak başlat

        for (size_t i = 0; i < rooms.size(); ++i) {
            response[i]["id"] = rooms[i].getId();
            response[i]["name"] = rooms[i].getName();
            response[i]["size"] = rooms[i].getSize();
            response[i]["type"] = rooms[i].getType();
        }

        return crow::response(200, response);
    } catch (const std::exception& e) {
        crow::json::wvalue error;
        error["error"] = e.what();
        return crow::response(500, error);
    }
});

// /users/<int>/rooms endpoint'i düzeltmesi
CROW_ROUTE(app, "/users/<int>/rooms").methods("GET"_method)
([&](int userId) {
    try {
        auto rooms = roomUserService.getRoomsForUser(userId);
        crow::json::wvalue response = crow::json::wvalue::list(); // Liste olarak başlat

        for (size_t i = 0; i < rooms.size(); ++i) {
            response[i]["id"] = rooms[i].getId();
            response[i]["name"] = rooms[i].getName();
            response[i]["size"] = rooms[i].getSize();
        }

        return crow::response(200, response);
    } catch (const std::exception& e) {
        crow::json::wvalue error;
        error["error"] = e.what();
        return crow::response(500, error);
    }
});


// GET /users/username/<string> ile kullanıcıyı getir
CROW_ROUTE(app, "/users/username/<string>")
.methods("GET"_method)
([&](const crow::request&, const std::string& username){
    std::cout << "Aranan kullanıcı adı: " << username << std::endl;
    
    auto optUser = userService.getUserByUsername(username);
    if (!optUser) {
        std::cout << "Kullanıcı bulunamadı: " << username << std::endl;
        crow::json::wvalue error;
        error["error"] = "User not found";
        return crow::response{404, error};
    }
    
    std::cout << "Kullanıcı bulundu: " << username << " (ID: " << optUser->id << ")" << std::endl;
    const auto& u = *optUser;
    crow::json::wvalue res;
    res["id"]       = u.id;
    //res["username"] = u.username;
    //res["email"]    = u.email;
    return crow::response{200, res};
});





// PUT /users/<int>/username - Kullanıcı adını değiştir
CROW_ROUTE(app, "/users/<int>/username")
.methods("PUT"_method)
([&](const crow::request& req, int userId) {
    try {
        auto body = crow::json::load(req.body);
        if (!body) {
            crow::json::wvalue error;
            error["error"] = "Invalid JSON";
            return crow::response(400, error);
        }
        
        // Yeni kullanıcı adını al
        std::string newUsername = body["username"].s();
        
        // Boş kullanıcı adı kontrolü
        if (newUsername.empty()) {
            crow::json::wvalue error;
            error["error"] = "Username cannot be empty";
            return crow::response(400, error);
        }
        
        // Kullanıcı adını güncelle
        bool success = userService.updateUsername(userId, newUsername);
        
        if (success) {
            // Güncellenmiş kullanıcı bilgisini al
            auto updatedUser = userService.getUserByUsername(newUsername);
            
            crow::json::wvalue result;
            result["message"] = "Username updated successfully";
            result["id"] = userId;
            result["username"] = newUsername;
            
            return crow::response(200, result);
        } else {
            crow::json::wvalue error;
            error["error"] = "Username is already taken or user not found";
            return crow::response(409, error);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in PUT /users/<id>/username: " << e.what() << std::endl;
        crow::json::wvalue error;
        error["error"] = e.what();
        return crow::response(500, error);
    }
});

    app.port(18080).multithreaded().run();
}