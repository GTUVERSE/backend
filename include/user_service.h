#include "user.h"
#include <mysqlx/xdevapi.h>
#include <vector>
#include <optional>

class UserService {
public:
    UserService();  // constructor
    bool registerUser(const std::string& username, const std::string& password);
    std::optional<User> loginUser(const std::string& username, const std::string& password);
    std::vector<User> getAllUsers();

private:
    mysqlx::Session session;
    mysqlx::Schema schema;
    mysqlx::Table usersTable;
};
