#pragma once

// LoginForm.h - форма авторизации
// Поддерживает вход через Windows аутентификацию и через логин/пароль SQL Server

#include "DatabaseHelper.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::ComponentModel;

namespace AutoService {

    public ref class LoginForm : public Form {
    private:
        // Элементы формы
        Label^ lblTitle;
        Label^ lblServer;
        Label^ lblLogin;
        Label^ lblPassword;
        Label^ lblAppLogin;
        Label^ lblAppPassword;

        TextBox^ txtServer;
        TextBox^ txtLogin;
        TextBox^ txtPassword;
        TextBox^ txtAppLogin;
        TextBox^ txtAppPassword;

        CheckBox^ chkWindowsAuth;
        Button^ btnConnect;
        Button^ btnExit;

        GroupBox^ grpConnection;
        GroupBox^ grpAppLogin;

        // Результаты авторизации
        String^ userRole;
        String^ userLogin;
        DatabaseHelper^ dbHelper;

    public:
        // Свойства для получения результатов после входа
        property String^ UserRole {
            String^ get() { return userRole; }
        }

        property String^ UserLogin {
            String^ get() { return userLogin; }
        }

        property DatabaseHelper^ DbHelper {
            DatabaseHelper^ get() { return dbHelper; }
        }

        LoginForm() {
            InitializeComponent();
        }

    private:
        void InitializeComponent() {
            // === Настройки формы ===
            this->Text = L"Автосервис - Вход в систему";
            this->Size = Drawing::Size(480, 520);
            this->StartPosition = FormStartPosition::CenterScreen;
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->BackColor = Color::FromArgb(240, 244, 248);
            this->Font = gcnew Drawing::Font("Segoe UI", 9);

            // === Заголовок ===
            lblTitle = gcnew Label();
            lblTitle->Text = L"Автосервис - Авторизация";
            lblTitle->Font = gcnew Drawing::Font("Segoe UI", 16, FontStyle::Bold);
            lblTitle->ForeColor = Color::FromArgb(44, 62, 80);
            lblTitle->AutoSize = true;
            lblTitle->Location = Point(90, 15);
            this->Controls->Add(lblTitle);

            // === Группа "Подключение к серверу" ===
            grpConnection = gcnew GroupBox();
            grpConnection->Text = L"Подключение к серверу БД";
            grpConnection->Location = Point(20, 55);
            grpConnection->Size = Drawing::Size(425, 195);
            grpConnection->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            this->Controls->Add(grpConnection);

            // Сервер
            lblServer = gcnew Label();
            lblServer->Text = L"Сервер:";
            lblServer->Location = Point(15, 30);
            lblServer->AutoSize = true;
            lblServer->Font = gcnew Drawing::Font("Segoe UI", 9);
            grpConnection->Controls->Add(lblServer);

            txtServer = gcnew TextBox();
            txtServer->Text = L"localhost\\SQLEXPRESS";
            txtServer->Location = Point(15, 50);
            txtServer->Size = Drawing::Size(395, 23);
            txtServer->Font = gcnew Drawing::Font("Segoe UI", 9);
            grpConnection->Controls->Add(txtServer);

            // Чекбокс Windows аутентификации
            chkWindowsAuth = gcnew CheckBox();
            chkWindowsAuth->Text = L"Использовать Windows аутентификацию (текущий пользователь Windows)";
            chkWindowsAuth->Location = Point(15, 82);
            chkWindowsAuth->AutoSize = true;
            chkWindowsAuth->Checked = true;
            chkWindowsAuth->Font = gcnew Drawing::Font("Segoe UI", 9);
            chkWindowsAuth->CheckedChanged += gcnew EventHandler(this, &LoginForm::OnAuthModeChanged);
            grpConnection->Controls->Add(chkWindowsAuth);

            // Логин SQL
            lblLogin = gcnew Label();
            lblLogin->Text = L"Логин SQL Server:";
            lblLogin->Location = Point(15, 110);
            lblLogin->AutoSize = true;
            lblLogin->Font = gcnew Drawing::Font("Segoe UI", 9);
            grpConnection->Controls->Add(lblLogin);

            txtLogin = gcnew TextBox();
            txtLogin->Location = Point(15, 130);
            txtLogin->Size = Drawing::Size(185, 23);
            txtLogin->Font = gcnew Drawing::Font("Segoe UI", 9);
            txtLogin->Enabled = false; // по умолчанию выключен (Windows Auth)
            grpConnection->Controls->Add(txtLogin);

            // Пароль SQL
            lblPassword = gcnew Label();
            lblPassword->Text = L"Пароль SQL Server:";
            lblPassword->Location = Point(215, 110);
            lblPassword->AutoSize = true;
            lblPassword->Font = gcnew Drawing::Font("Segoe UI", 9);
            grpConnection->Controls->Add(lblPassword);

            txtPassword = gcnew TextBox();
            txtPassword->Location = Point(215, 130);
            txtPassword->Size = Drawing::Size(195, 23);
            txtPassword->Font = gcnew Drawing::Font("Segoe UI", 9);
            txtPassword->UseSystemPasswordChar = true;
            txtPassword->Enabled = false;
            grpConnection->Controls->Add(txtPassword);

            // === Группа "Вход в приложение" ===
            grpAppLogin = gcnew GroupBox();
            grpAppLogin->Text = L"Вход в приложение";
            grpAppLogin->Location = Point(20, 260);
            grpAppLogin->Size = Drawing::Size(425, 130);
            grpAppLogin->Font = gcnew Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            this->Controls->Add(grpAppLogin);

            // Логин приложения
            lblAppLogin = gcnew Label();
            lblAppLogin->Text = L"Логин:";
            lblAppLogin->Location = Point(15, 30);
            lblAppLogin->AutoSize = true;
            lblAppLogin->Font = gcnew Drawing::Font("Segoe UI", 9);
            grpAppLogin->Controls->Add(lblAppLogin);

            txtAppLogin = gcnew TextBox();
            txtAppLogin->Location = Point(15, 50);
            txtAppLogin->Size = Drawing::Size(185, 23);
            txtAppLogin->Font = gcnew Drawing::Font("Segoe UI", 9);
            grpAppLogin->Controls->Add(txtAppLogin);

            // Пароль приложения
            lblAppPassword = gcnew Label();
            lblAppPassword->Text = L"Пароль:";
            lblAppPassword->Location = Point(215, 30);
            lblAppPassword->AutoSize = true;
            lblAppPassword->Font = gcnew Drawing::Font("Segoe UI", 9);
            grpAppLogin->Controls->Add(lblAppPassword);

            txtAppPassword = gcnew TextBox();
            txtAppPassword->Location = Point(215, 50);
            txtAppPassword->Size = Drawing::Size(195, 23);
            txtAppPassword->Font = gcnew Drawing::Font("Segoe UI", 9);
            txtAppPassword->UseSystemPasswordChar = true;
            grpAppLogin->Controls->Add(txtAppPassword);

            // === Кнопки ===
            btnConnect = gcnew Button();
            btnConnect->Text = L"Войти";
            btnConnect->Location = Point(130, 405);
            btnConnect->Size = Drawing::Size(100, 35);
            btnConnect->BackColor = Color::FromArgb(52, 152, 219);
            btnConnect->ForeColor = Color::White;
            btnConnect->FlatStyle = FlatStyle::Flat;
            btnConnect->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
            btnConnect->Click += gcnew EventHandler(this, &LoginForm::OnConnectClick);
            this->Controls->Add(btnConnect);

            btnExit = gcnew Button();
            btnExit->Text = L"Выход";
            btnExit->Location = Point(250, 405);
            btnExit->Size = Drawing::Size(100, 35);
            btnExit->BackColor = Color::FromArgb(231, 76, 60);
            btnExit->ForeColor = Color::White;
            btnExit->FlatStyle = FlatStyle::Flat;
            btnExit->Font = gcnew Drawing::Font("Segoe UI", 10);
            btnExit->Click += gcnew EventHandler(this, &LoginForm::OnExitClick);
            this->Controls->Add(btnExit);

            // Enter для быстрого входа
            this->AcceptButton = btnConnect;
        }

        // Переключение режима аутентификации
        void OnAuthModeChanged(Object^ sender, EventArgs^ e) {
            bool sqlMode = !chkWindowsAuth->Checked;
            txtLogin->Enabled = sqlMode;
            txtPassword->Enabled = sqlMode;

            if (!sqlMode) {
                txtLogin->Text = "";
                txtPassword->Text = "";
            }
        }

        // Нажатие кнопки "Войти"
        void OnConnectClick(Object^ sender, EventArgs^ e) {
            // Проверяем что заполнены поля входа в приложение
            if (String::IsNullOrWhiteSpace(txtAppLogin->Text) ||
                String::IsNullOrWhiteSpace(txtAppPassword->Text)) {
                MessageBox::Show(
                    "Введите логин и пароль для входа в приложение!",
                    "Ошибка",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Warning);
                return;
            }

            // Создаем подключение в зависимости от выбранного режима
            if (chkWindowsAuth->Checked) {
                // Windows аутентификация
                dbHelper = gcnew DatabaseHelper();
            }
            else {
                // SQL Server аутентификация
                if (String::IsNullOrWhiteSpace(txtLogin->Text) ||
                    String::IsNullOrWhiteSpace(txtPassword->Text)) {
                    MessageBox::Show(
                        "Введите логин и пароль SQL Server!",
                        "Ошибка",
                        MessageBoxButtons::OK,
                        MessageBoxIcon::Warning);
                    return;
                }
                dbHelper = gcnew DatabaseHelper(txtLogin->Text, txtPassword->Text);
            }

            // Проверяем подключение к серверу
            if (!dbHelper->TestConnection()) {
                return;
            }

            // Аутентификация пользователя приложения
            String^ role = dbHelper->AuthenticateUser(txtAppLogin->Text, txtAppPassword->Text);

            if (role == nullptr) {
                MessageBox::Show(
                    "Неверный логин или пароль!",
                    "Ошибка входа",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Error);
                return;
            }

            userRole = role;
            userLogin = txtAppLogin->Text;
            this->DialogResult = System::Windows::Forms::DialogResult::OK;
            this->Close();
        }

        // Нажатие кнопки "Выход"
        void OnExitClick(Object^ sender, EventArgs^ e) {
            this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
            this->Close();
        }
    };
}
