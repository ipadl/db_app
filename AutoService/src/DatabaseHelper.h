#pragma once

// DatabaseHelper.h - слой доступа к данным (DAL)
// Поддерживает подключение через Windows аутентификацию и через логин/пароль

using namespace System;
using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::Windows::Forms;

namespace AutoService {

    // Перечисление для типа аутентификации
    public enum class AuthMode {
        WindowsAuth,   // Через учетную запись Windows (Integrated Security)
        SqlAuth         // Через логин и пароль SQL Server
    };

    // Класс для работы с базой данных
    public ref class DatabaseHelper {
    private:
        // Строка подключения
        String^ connectionString;

        // Режим аутентификации
        AuthMode authMode;

        // Логин и пароль (для SQL Server аутентификации)
        String^ sqlLogin;
        String^ sqlPassword;

    public:
        // Конструктор для Windows аутентификации
        DatabaseHelper();

        // Конструктор для SQL Server аутентификации
        DatabaseHelper(String^ login, String^ password);

        // Получить строку подключения
        String^ GetConnectionString();

        // Проверка подключения к БД
        bool TestConnection();

        // Выполнить запрос SELECT и вернуть таблицу данных
        DataTable^ ExecuteQuery(String^ query);

        // Выполнить запрос SELECT с параметрами
        DataTable^ ExecuteQuery(String^ query, array<SqlParameter^>^ params);

        // Выполнить INSERT, UPDATE, DELETE (возвращает кол-во затронутых строк)
        int ExecuteNonQuery(String^ query, array<SqlParameter^>^ params);

        // Выполнить скалярный запрос (вернуть одно значение)
        Object^ ExecuteScalar(String^ query, array<SqlParameter^>^ params);

        // Авторизация пользователя - возвращает роль или nullptr
        String^ AuthenticateUser(String^ login, String^ password);

        // Получить режим аутентификации
        AuthMode GetAuthMode();
    };
}
