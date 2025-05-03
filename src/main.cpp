#include "../external/crow/include/crow.h"
#include "../include/user_service.h"
#include "room_service.h"
#include "room.h"
#include <crow.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

RoomService roomService;


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
            result[i]["username"] = users[i].username;
        }
        return crow::response(result);
    });



CROW_ROUTE(app, "/rooms").methods("GET"_method)([](){
    auto rooms = roomService.getAllRooms();
    json j = json::array();
    for (const auto& room : rooms) {
        j.push_back({{"id", room.getId()}, {"name", room.getName()}});
    }
    return crow::response{j.dump()};
});

CROW_ROUTE(app, "/rooms/<int>").methods("GET"_method)([](int id){
    auto result = roomService.getRoomById(id);
    if (result) {
        json j = {{"id", result->getId()}, {"name", result->getName()}};
        return crow::response{j.dump()};
    }
    return crow::response{404};
});

CROW_ROUTE(app, "/rooms").methods("POST"_method)([](const crow::request& req){
    auto j = json::parse(req.body);
    Room newRoom(j["id"].get<int>(), j["name"].get<std::string>(), j["capacity"].get<int>());
 
   roomService.createRoom(newRoom);
    return crow::response{201};
});

CROW_ROUTE(app, "/rooms/<int>").methods("DELETE"_method)([](int id){
    bool success = roomService.deleteRoom(id);
    return crow::response{success ? 200 : 404};
});







    app.port(18080).multithreaded().run();
}
