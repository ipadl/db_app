// DatabaseHelper.cpp - реализация слоя доступа к данным
// Поддержка Windows Auth и SQL Server Auth для подключения к MSSQL

#include "DatabaseHelper.h"

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Configuration;
using namespace System::Windows::Forms;

namespace AutoService {

    // Конструктор для Windows аутентификации (Integrated Security)
    DatabaseHelper::DatabaseHelper() {
        this->authMode = AuthMode::WindowsAuth;
        this->sqlLogin = nullptr;
        this->sqlPassword = nullptr;

        // Читаем строку подключения из app.config
        try {
            this->connectionString = ConfigurationManager::ConnectionStrings["AutoServiceWinAuth"]->ConnectionString;
        }
        catch (Exception^) {
            // Если конфиг не найден - используем строку по умолчанию
            this->connectionString = "Server=localhost\\SQLEXPRESS;Database=AutoServiceDB;Integrated Security=true;TrustServerCertificate=true;";
        }
    }

    // Конструктор для SQL Server аутентификации (логин + пароль)
    DatabaseHelper::DatabaseHelper(String^ login, String^ password) {
        this->authMode = AuthMode::SqlAuth;
        this->sqlLogin = login;
        this->sqlPassword = password;

        // Читаем шаблон строки подключения из app.config и подставляем логин/пароль
        try {
            String^ connectionTemplate = ConfigurationManager::ConnectionStrings["AutoServiceSqlAuth"]->ConnectionString;
            this->connectionString = String::Format(connectionTemplate, login, password);
        }
        catch (Exception^) {
            // Если конфиг не найден - собираем строку вручную
            this->connectionString = String::Format(
                "Server=localhost\\SQLEXPRESS;Database=AutoServiceDB;User Id={0};Password={1};TrustServerCertificate=true;",
                login, password);
        }
    }

    // Получить строку подключения
    String^ DatabaseHelper::GetConnectionString() {
        return this->connectionString;
    }

    // Получить режим аутентификации
    AuthMode DatabaseHelper::GetAuthMode() {
        return this->authMode;
    }

    // Проверка подключения к серверу БД
    bool DatabaseHelper::TestConnection() {
        SqlConnection^ conn = nullptr;
        try {
            conn = gcnew SqlConnection(this->connectionString);
            conn->Open();
            return true;
        }
        catch (SqlException^ ex) {
            MessageBox::Show(
                "Ошибка подключения к базе данных:\n" + ex->Message,
                "Ошибка соединения",
                MessageBoxButtons::OK,
                MessageBoxIcon::Error);
            return false;
        }
        finally {
            if (conn != nullptr && conn->State == ConnectionState::Open) {
                conn->Close();
            }
        }
    }

    // Выполнить SELECT запрос без параметров
    DataTable^ DatabaseHelper::ExecuteQuery(String^ query) {
        return ExecuteQuery(query, nullptr);
    }

    // Выполнить SELECT запрос с параметрами (защита от SQL-инъекций)
    DataTable^ DatabaseHelper::ExecuteQuery(String^ query, array<SqlParameter^>^ params) {
        DataTable^ table = gcnew DataTable();
        SqlConnection^ conn = nullptr;

        try {
            conn = gcnew SqlConnection(this->connectionString);
            SqlCommand^ cmd = gcnew SqlCommand(query, conn);
            cmd->CommandTimeout = 30;

            // Добавляем параметры если есть
            if (params != nullptr) {
                for each (SqlParameter^ p in params) {
                    cmd->Parameters->Add(p);
                }
            }

            SqlDataAdapter^ adapter = gcnew SqlDataAdapter(cmd);
            conn->Open();
            adapter->Fill(table);
        }
        catch (SqlException^ ex) {
            MessageBox::Show(
                "Ошибка выполнения запроса:\n" + ex->Message,
                "Ошибка БД",
                MessageBoxButtons::OK,
                MessageBoxIcon::Error);
        }
        finally {
            if (conn != nullptr && conn->State == ConnectionState::Open) {
                conn->Close();
            }
        }

        return table;
    }

    // Выполнить INSERT/UPDATE/DELETE с параметрами
    int DatabaseHelper::ExecuteNonQuery(String^ query, array<SqlParameter^>^ params) {
        SqlConnection^ conn = nullptr;
        int result = 0;

        try {
            conn = gcnew SqlConnection(this->connectionString);
            SqlCommand^ cmd = gcnew SqlCommand(query, conn);
            cmd->CommandTimeout = 30;

            if (params != nullptr) {
                for each (SqlParameter^ p in params) {
                    cmd->Parameters->Add(p);
                }
            }

            conn->Open();
            result = cmd->ExecuteNonQuery();
        }
        catch (SqlException^ ex) {
            MessageBox::Show(
                "Ошибка выполнения операции:\n" + ex->Message,
                "Ошибка БД",
                MessageBoxButtons::OK,
                MessageBoxIcon::Error);
            result = -1;
        }
        finally {
            if (conn != nullptr && conn->State == ConnectionState::Open) {
                conn->Close();
            }
        }

        return result;
    }

    // Выполнить скалярный запрос (одно значение)
    Object^ DatabaseHelper::ExecuteScalar(String^ query, array<SqlParameter^>^ params) {
        SqlConnection^ conn = nullptr;
        Object^ result = nullptr;

        try {
            conn = gcnew SqlConnection(this->connectionString);
            SqlCommand^ cmd = gcnew SqlCommand(query, conn);
            cmd->CommandTimeout = 30;

            if (params != nullptr) {
                for each (SqlParameter^ p in params) {
                    cmd->Parameters->Add(p);
                }
            }

            conn->Open();
            result = cmd->ExecuteScalar();
        }
        catch (SqlException^ ex) {
            MessageBox::Show(
                "Ошибка запроса:\n" + ex->Message,
                "Ошибка БД",
                MessageBoxButtons::OK,
                MessageBoxIcon::Error);
        }
        finally {
            if (conn != nullptr && conn->State == ConnectionState::Open) {
                conn->Close();
            }
        }

        return result;
    }

    // Аутентификация пользователя приложения (проверка логина/пароля в таблице Users)
    // Возвращает название роли или nullptr если пользователь не найден
    String^ DatabaseHelper::AuthenticateUser(String^ login, String^ password) {
        String^ query = "SELECT r.RoleName FROM Users u "
                        "INNER JOIN Roles r ON u.RoleId = r.RoleId "
                        "WHERE u.Login = @login AND u.PasswordHash = HASHBYTES('SHA2_256', @password)";

        array<SqlParameter^>^ params = gcnew array<SqlParameter^>(2);
        params[0] = gcnew SqlParameter("@login", SqlDbType::NVarChar);
        params[0]->Value = login;
        params[1] = gcnew SqlParameter("@password", SqlDbType::NVarChar);
        params[1]->Value = password;

        Object^ result = ExecuteScalar(query, params);

        if (result != nullptr && result != DBNull::Value) {
            return result->ToString();
        }

        return nullptr;
    }
}
