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
        if (!body) return crow::response(400);
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        bool ok = userService.registerUser(username, password);
        return ok ? crow::response(200, "Registered") : crow::response(409, "Username exists");
    });

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



    CROW_ROUTE(app, "/rooms").methods("GET"_method)([](){
        auto rooms = roomService.getAllRooms();
        json j = json::array();
        for (const auto& room : rooms) {
            j.push_back({
                {"id", room.getId()},
                {"name", room.getName()},
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
        auto j = json::parse(req.body);
        std::string name = j["name"];
        int capacity = j["capacity"];
    
        Room newRoom(0, name, capacity);
        roomService.createRoom(newRoom);
    
        // ID'yi almak için tekrar sorgulama:
        auto rooms = roomService.getAllRooms();
        const Room& lastRoom = rooms.back();  // En son eklenen oda
    
        crow::json::wvalue response;
        response["message"] = "Room created";
        response["id"] = lastRoom.getId();
        return crow::response{201, response};
    });
    

CROW_ROUTE(app, "/rooms/<int>").methods("DELETE"_method)([](int id){
    bool success = roomService.deleteRoom(id);
    return crow::response{success ? 200 : 404};
});



CROW_ROUTE(app, "/rooms/<int>/users").methods("POST"_method)
([&](const crow::request& req, int roomId) {
    auto j = json::parse(req.body);
    int userId = j["user_id"];
    roomUserService.addUserToRoom(userId, roomId);
    return crow::response{201, "User added to room"};
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
    bool success = roomUserService.removeUserFromRoom(roomId, userId);
    return crow::response{success ? 200 : 404};
});




CROW_ROUTE(app, "/users/<int>/rooms").methods("GET"_method)
([&](int userId) {
    try {
        auto rooms = roomUserService.getRoomsForUser(userId);
        crow::json::wvalue response;

        for (size_t i = 0; i < rooms.size(); ++i) {
            response[i]["id"] = rooms[i].getId();
            response[i]["name"] = rooms[i].getName();
            response[i]["capacity"] = rooms[i].getCapacity();
        }

        return crow::response{response};
    } catch (const std::exception& e) {
        std::cerr << "Error in GET /users/<id>/rooms: " << e.what() << std::endl;
        return crow::response{500};
    }
});


    app.port(18080).multithreaded().run();
}
