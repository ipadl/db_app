// Main.cpp - точка входа приложения Автосервис
// Лабораторная работа №3 - Вариант 2

#include "LoginForm.h"
#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace AutoService;

[STAThread]
int main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    // Цикл авторизации (для повторного входа после таймаута)
    bool keepRunning = true;

    while (keepRunning) {
        // Показываем форму входа
        LoginForm^ loginForm = gcnew LoginForm();
        System::Windows::Forms::DialogResult loginResult = loginForm->ShowDialog();

        if (loginResult == System::Windows::Forms::DialogResult::OK) {
            // Вход выполнен - открываем главную форму
            MainForm^ mainForm = gcnew MainForm(
                loginForm->DbHelper,
                loginForm->UserRole,
                loginForm->UserLogin
            );

            System::Windows::Forms::DialogResult mainResult = mainForm->ShowDialog();

            if (mainResult == System::Windows::Forms::DialogResult::Retry) {
                // Таймаут или выход из системы - показываем логин снова
                continue;
            }
            else {
                // Закрытие программы
                keepRunning = false;
            }
        }
        else {
            // Нажали "Выход" на форме логина
            keepRunning = false;
        }
    }

    return 0;
}
