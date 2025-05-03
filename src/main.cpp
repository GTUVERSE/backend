#include "../external/crow/include/crow.h"
#include "../include/user_service.h"

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

    app.port(18080).multithreaded().run();
}
