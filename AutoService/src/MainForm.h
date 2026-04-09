#pragma once

// MainForm.h - главная форма приложения автосервиса
// Динамически формирует интерфейс в зависимости от роли пользователя
// Роли: Admin, Client, Mechanic, Manager

#include "DatabaseHelper.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Data::SqlClient;

namespace AutoService {

    public ref class MainForm : public Form {
    private:
        // Данные подключения и роль
        DatabaseHelper^ db;
        String^ currentRole;
        String^ currentUser;

        // Основные элементы интерфейса
        MenuStrip^ mainMenu;
        StatusStrip^ statusBar;
        ToolStripStatusLabel^ lblStatus;
        ToolStripStatusLabel^ lblRole;
        ToolStripStatusLabel^ lblTimer;
        TabControl^ tabMain;

        // Таймер неактивности (15 минут)
        Timer^ inactivityTimer;
        DateTime lastActivity;
        int timeoutMinutes;

        // ===== Вкладки для Admin =====
        TabPage^ tabDashboard;
        Label^ lblOpenOrders;
        Label^ lblMechanicsLoad;
        Label^ lblOpenOrdersValue;
        Label^ lblMechanicsLoadValue;
        Button^ btnRefreshDashboard;

        TabPage^ tabServices;
        DataGridView^ dgvServices;
        Button^ btnAddService;
        Button^ btnEditService;
        Button^ btnDeleteService;
        TextBox^ txtServiceName;
        TextBox^ txtServicePrice;
        Label^ lblServiceName;
        Label^ lblServicePrice;

        TabPage^ tabParts;
        DataGridView^ dgvParts;
        Button^ btnAddPart;
        Button^ btnEditPart;
        Button^ btnDeletePart;
        TextBox^ txtPartName;
        TextBox^ txtPartPrice;
        TextBox^ txtPartQty;
        Label^ lblPartName;
        Label^ lblPartPrice;
        Label^ lblPartQty;

        // ===== Вкладки для Client =====
        TabPage^ tabMyOrders;
        DataGridView^ dgvMyOrders;
        Button^ btnNewOrder;

        TabPage^ tabNewOrder;
        ComboBox^ cmbCar;
        CheckedListBox^ clbServices;
        TextBox^ txtOrderComment;
        MonthCalendar^ calendarDate;
        Button^ btnCreateOrder;
        Label^ lblSelectCar;
        Label^ lblSelectServices;
        Label^ lblComment;
        Label^ lblDate;

        // ===== Вкладки для Mechanic =====
        TabPage^ tabMechOrders;
        DataGridView^ dgvMechOrders;
        ComboBox^ cmbStatusFilter;
        Label^ lblFilterStatus;

        TabPage^ tabOrderDetail;
        DataGridView^ dgvOrderWorks;
        DataGridView^ dgvOrderParts;
        PropertyGrid^ propCarInfo;
        ProgressBar^ progressRepair;
        Button^ btnStartWork;
        Button^ btnPauseWork;
        Button^ btnFinishWork;
        Label^ lblWorksList;
        Label^ lblPartsList;
        Label^ lblCarInfo;
        Label^ lblProgress;

        // ===== Вкладки для Manager =====
        TabPage^ tabManageOrders;
        DataGridView^ dgvAllOrders;
        ComboBox^ cmbAssignMechanic;
        ComboBox^ cmbChangeStatus;
        Button^ btnAssign;
        Button^ btnChangeStatus;
        Label^ lblAssignMech;
        Label^ lblChangeStatus;

        TabPage^ tabReport;
        DataGridView^ dgvReport;
        DateTimePicker^ dtpFrom;
        DateTimePicker^ dtpTo;
        Button^ btnGenerateReport;
        Label^ lblFrom;
        Label^ lblTo;
        Label^ lblTotalIncome;

    public:
        MainForm(DatabaseHelper^ dbHelper, String^ role, String^ userName) {
            this->db = dbHelper;
            this->currentRole = role;
            this->currentUser = userName;
            this->timeoutMinutes = 15;

            InitializeComponent();
            SetupRoleTabs();
            StartInactivityTimer();
        }

    private:
        // =============================================
        // ИНИЦИАЛИЗАЦИЯ КОМПОНЕНТОВ
        // =============================================
        void InitializeComponent() {
            this->Text = L"Автосервис - Главная [" + currentRole + "]";
            this->Size = Drawing::Size(1024, 700);
            this->StartPosition = FormStartPosition::CenterScreen;
            this->BackColor = Color::FromArgb(240, 244, 248);
            this->Font = gcnew Drawing::Font("Segoe UI", 9);

            // Обработка активности мыши/клавиатуры для таймера неактивности
            this->MouseMove += gcnew MouseEventHandler(this, &MainForm::OnUserActivity);
            this->KeyPress += gcnew KeyPressEventHandler(this, &MainForm::OnKeyActivity);

            // === Меню ===
            mainMenu = gcnew MenuStrip();
            ToolStripMenuItem^ menuFile = gcnew ToolStripMenuItem("Файл");
            ToolStripMenuItem^ menuLogout = gcnew ToolStripMenuItem("Выйти из системы");
            menuLogout->Click += gcnew EventHandler(this, &MainForm::OnLogout);
            ToolStripMenuItem^ menuExit = gcnew ToolStripMenuItem("Закрыть программу");
            menuExit->Click += gcnew EventHandler(this, &MainForm::OnExit);
            menuFile->DropDownItems->Add(menuLogout);
            menuFile->DropDownItems->Add(menuExit);
            mainMenu->Items->Add(menuFile);
            this->MainMenuStrip = mainMenu;
            this->Controls->Add(mainMenu);

            // === Статусбар ===
            statusBar = gcnew StatusStrip();
            lblStatus = gcnew ToolStripStatusLabel("Готово");
            lblRole = gcnew ToolStripStatusLabel("Роль: " + currentRole);
            lblTimer = gcnew ToolStripStatusLabel("Сессия активна");
            statusBar->Items->Add(lblStatus);
            statusBar->Items->Add(gcnew ToolStripStatusLabel(" | "));
            statusBar->Items->Add(lblRole);
            statusBar->Items->Add(gcnew ToolStripStatusLabel(" | "));
            statusBar->Items->Add(lblTimer);
            this->Controls->Add(statusBar);

            // === Главный TabControl ===
            tabMain = gcnew TabControl();
            tabMain->Location = Point(10, 30);
            tabMain->Size = Drawing::Size(990, 600);
            tabMain->Anchor = static_cast<AnchorStyles>(
                AnchorStyles::Top | AnchorStyles::Bottom |
                AnchorStyles::Left | AnchorStyles::Right);
            this->Controls->Add(tabMain);
        }

        // =============================================
        // НАСТРОЙКА ВКЛАДОК ПО РОЛИ
        // =============================================
        void SetupRoleTabs() {
            tabMain->TabPages->Clear();

            if (currentRole == "Admin") {
                SetupAdminTabs();
            }
            else if (currentRole == "Client") {
                SetupClientTabs();
            }
            else if (currentRole == "Mechanic") {
                SetupMechanicTabs();
            }
            else if (currentRole == "Manager") {
                SetupManagerTabs();
            }
        }

        // =============================================
        // ВКЛАДКИ АДМИНИСТРАТОРА
        // =============================================
        void SetupAdminTabs() {
            // --- Панель мониторинга ---
            tabDashboard = gcnew TabPage("Панель мониторинга");
            tabDashboard->BackColor = Color::White;

            lblOpenOrders = gcnew Label();
            lblOpenOrders->Text = "Открытых заказов:";
            lblOpenOrders->Location = Point(30, 30);
            lblOpenOrders->AutoSize = true;
            lblOpenOrders->Font = gcnew Drawing::Font("Segoe UI", 12);
            tabDashboard->Controls->Add(lblOpenOrders);

            lblOpenOrdersValue = gcnew Label();
            lblOpenOrdersValue->Text = "0";
            lblOpenOrdersValue->Location = Point(220, 30);
            lblOpenOrdersValue->AutoSize = true;
            lblOpenOrdersValue->Font = gcnew Drawing::Font("Segoe UI", 14, FontStyle::Bold);
            lblOpenOrdersValue->ForeColor = Color::FromArgb(52, 152, 219);
            tabDashboard->Controls->Add(lblOpenOrdersValue);

            lblMechanicsLoad = gcnew Label();
            lblMechanicsLoad->Text = "Загрузка механиков:";
            lblMechanicsLoad->Location = Point(30, 70);
            lblMechanicsLoad->AutoSize = true;
            lblMechanicsLoad->Font = gcnew Drawing::Font("Segoe UI", 12);
            tabDashboard->Controls->Add(lblMechanicsLoad);

            lblMechanicsLoadValue = gcnew Label();
            lblMechanicsLoadValue->Text = "0";
            lblMechanicsLoadValue->Location = Point(230, 70);
            lblMechanicsLoadValue->AutoSize = true;
            lblMechanicsLoadValue->Font = gcnew Drawing::Font("Segoe UI", 14, FontStyle::Bold);
            lblMechanicsLoadValue->ForeColor = Color::FromArgb(46, 204, 113);
            tabDashboard->Controls->Add(lblMechanicsLoadValue);

            btnRefreshDashboard = gcnew Button();
            btnRefreshDashboard->Text = "Обновить";
            btnRefreshDashboard->Location = Point(30, 120);
            btnRefreshDashboard->Size = Drawing::Size(120, 30);
            btnRefreshDashboard->BackColor = Color::FromArgb(52, 152, 219);
            btnRefreshDashboard->ForeColor = Color::White;
            btnRefreshDashboard->FlatStyle = FlatStyle::Flat;
            btnRefreshDashboard->Click += gcnew EventHandler(this, &MainForm::OnRefreshDashboard);
            tabDashboard->Controls->Add(btnRefreshDashboard);

            tabMain->TabPages->Add(tabDashboard);

            // --- Управление услугами ---
            tabServices = gcnew TabPage("Услуги");
            tabServices->BackColor = Color::White;

            dgvServices = gcnew DataGridView();
            dgvServices->Location = Point(10, 10);
            dgvServices->Size = Drawing::Size(700, 300);
            dgvServices->AllowUserToAddRows = false;
            dgvServices->ReadOnly = true;
            dgvServices->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
            dgvServices->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvServices->BackgroundColor = Color::White;
            tabServices->Controls->Add(dgvServices);

            lblServiceName = gcnew Label();
            lblServiceName->Text = "Название услуги:";
            lblServiceName->Location = Point(10, 320);
            lblServiceName->AutoSize = true;
            tabServices->Controls->Add(lblServiceName);

            txtServiceName = gcnew TextBox();
            txtServiceName->Location = Point(10, 340);
            txtServiceName->Size = Drawing::Size(300, 23);
            tabServices->Controls->Add(txtServiceName);

            lblServicePrice = gcnew Label();
            lblServicePrice->Text = "Цена:";
            lblServicePrice->Location = Point(320, 320);
            lblServicePrice->AutoSize = true;
            tabServices->Controls->Add(lblServicePrice);

            txtServicePrice = gcnew TextBox();
            txtServicePrice->Location = Point(320, 340);
            txtServicePrice->Size = Drawing::Size(120, 23);
            tabServices->Controls->Add(txtServicePrice);

            btnAddService = gcnew Button();
            btnAddService->Text = "Добавить";
            btnAddService->Location = Point(10, 375);
            btnAddService->Size = Drawing::Size(100, 30);
            btnAddService->BackColor = Color::FromArgb(46, 204, 113);
            btnAddService->ForeColor = Color::White;
            btnAddService->FlatStyle = FlatStyle::Flat;
            btnAddService->Click += gcnew EventHandler(this, &MainForm::OnAddService);
            tabServices->Controls->Add(btnAddService);

            btnEditService = gcnew Button();
            btnEditService->Text = "Изменить";
            btnEditService->Location = Point(120, 375);
            btnEditService->Size = Drawing::Size(100, 30);
            btnEditService->BackColor = Color::FromArgb(243, 156, 18);
            btnEditService->ForeColor = Color::White;
            btnEditService->FlatStyle = FlatStyle::Flat;
            btnEditService->Click += gcnew EventHandler(this, &MainForm::OnEditService);
            tabServices->Controls->Add(btnEditService);

            btnDeleteService = gcnew Button();
            btnDeleteService->Text = "Удалить";
            btnDeleteService->Location = Point(230, 375);
            btnDeleteService->Size = Drawing::Size(100, 30);
            btnDeleteService->BackColor = Color::FromArgb(231, 76, 60);
            btnDeleteService->ForeColor = Color::White;
            btnDeleteService->FlatStyle = FlatStyle::Flat;
            btnDeleteService->Click += gcnew EventHandler(this, &MainForm::OnDeleteService);
            tabServices->Controls->Add(btnDeleteService);

            tabMain->TabPages->Add(tabServices);

            // --- Управление запчастями ---
            tabParts = gcnew TabPage("Запчасти");
            tabParts->BackColor = Color::White;

            dgvParts = gcnew DataGridView();
            dgvParts->Location = Point(10, 10);
            dgvParts->Size = Drawing::Size(700, 300);
            dgvParts->AllowUserToAddRows = false;
            dgvParts->ReadOnly = true;
            dgvParts->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
            dgvParts->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvParts->BackgroundColor = Color::White;
            tabParts->Controls->Add(dgvParts);

            lblPartName = gcnew Label();
            lblPartName->Text = "Название:";
            lblPartName->Location = Point(10, 320);
            lblPartName->AutoSize = true;
            tabParts->Controls->Add(lblPartName);

            txtPartName = gcnew TextBox();
            txtPartName->Location = Point(10, 340);
            txtPartName->Size = Drawing::Size(200, 23);
            tabParts->Controls->Add(txtPartName);

            lblPartPrice = gcnew Label();
            lblPartPrice->Text = "Цена:";
            lblPartPrice->Location = Point(220, 320);
            lblPartPrice->AutoSize = true;
            tabParts->Controls->Add(lblPartPrice);

            txtPartPrice = gcnew TextBox();
            txtPartPrice->Location = Point(220, 340);
            txtPartPrice->Size = Drawing::Size(100, 23);
            tabParts->Controls->Add(txtPartPrice);

            lblPartQty = gcnew Label();
            lblPartQty->Text = "Кол-во:";
            lblPartQty->Location = Point(330, 320);
            lblPartQty->AutoSize = true;
            tabParts->Controls->Add(lblPartQty);

            txtPartQty = gcnew TextBox();
            txtPartQty->Location = Point(330, 340);
            txtPartQty->Size = Drawing::Size(80, 23);
            tabParts->Controls->Add(txtPartQty);

            btnAddPart = gcnew Button();
            btnAddPart->Text = "Добавить";
            btnAddPart->Location = Point(10, 375);
            btnAddPart->Size = Drawing::Size(100, 30);
            btnAddPart->BackColor = Color::FromArgb(46, 204, 113);
            btnAddPart->ForeColor = Color::White;
            btnAddPart->FlatStyle = FlatStyle::Flat;
            btnAddPart->Click += gcnew EventHandler(this, &MainForm::OnAddPart);
            tabParts->Controls->Add(btnAddPart);

            btnEditPart = gcnew Button();
            btnEditPart->Text = "Изменить";
            btnEditPart->Location = Point(120, 375);
            btnEditPart->Size = Drawing::Size(100, 30);
            btnEditPart->BackColor = Color::FromArgb(243, 156, 18);
            btnEditPart->ForeColor = Color::White;
            btnEditPart->FlatStyle = FlatStyle::Flat;
            btnEditPart->Click += gcnew EventHandler(this, &MainForm::OnEditPart);
            tabParts->Controls->Add(btnEditPart);

            btnDeletePart = gcnew Button();
            btnDeletePart->Text = "Удалить";
            btnDeletePart->Location = Point(230, 375);
            btnDeletePart->Size = Drawing::Size(100, 30);
            btnDeletePart->BackColor = Color::FromArgb(231, 76, 60);
            btnDeletePart->ForeColor = Color::White;
            btnDeletePart->FlatStyle = FlatStyle::Flat;
            btnDeletePart->Click += gcnew EventHandler(this, &MainForm::OnDeletePart);
            tabParts->Controls->Add(btnDeletePart);

            tabMain->TabPages->Add(tabParts);

            // Загружаем данные
            LoadDashboard();
            LoadServices();
            LoadParts();
        }

        // =============================================
        // ВКЛАДКИ КЛИЕНТА
        // =============================================
        void SetupClientTabs() {
            // --- Мои заказы ---
            tabMyOrders = gcnew TabPage("Мои заказы");
            tabMyOrders->BackColor = Color::White;

            dgvMyOrders = gcnew DataGridView();
            dgvMyOrders->Location = Point(10, 10);
            dgvMyOrders->Size = Drawing::Size(950, 400);
            dgvMyOrders->AllowUserToAddRows = false;
            dgvMyOrders->ReadOnly = true;
            dgvMyOrders->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
            dgvMyOrders->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvMyOrders->BackgroundColor = Color::White;
            // Цветовая индикация статуса
            dgvMyOrders->CellFormatting += gcnew DataGridViewCellFormattingEventHandler(
                this, &MainForm::OnOrdersCellFormatting);
            tabMyOrders->Controls->Add(dgvMyOrders);

            btnNewOrder = gcnew Button();
            btnNewOrder->Text = "Создать новый заказ";
            btnNewOrder->Location = Point(10, 420);
            btnNewOrder->Size = Drawing::Size(180, 35);
            btnNewOrder->BackColor = Color::FromArgb(52, 152, 219);
            btnNewOrder->ForeColor = Color::White;
            btnNewOrder->FlatStyle = FlatStyle::Flat;
            btnNewOrder->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
            btnNewOrder->Click += gcnew EventHandler(this, &MainForm::OnNewOrderTab);
            tabMyOrders->Controls->Add(btnNewOrder);

            tabMain->TabPages->Add(tabMyOrders);

            // --- Новый заказ ---
            tabNewOrder = gcnew TabPage("Новый заказ");
            tabNewOrder->BackColor = Color::White;

            lblSelectCar = gcnew Label();
            lblSelectCar->Text = "Выберите автомобиль:";
            lblSelectCar->Location = Point(15, 15);
            lblSelectCar->AutoSize = true;
            lblSelectCar->Font = gcnew Drawing::Font("Segoe UI", 10);
            tabNewOrder->Controls->Add(lblSelectCar);

            cmbCar = gcnew ComboBox();
            cmbCar->Location = Point(15, 40);
            cmbCar->Size = Drawing::Size(300, 23);
            cmbCar->DropDownStyle = ComboBoxStyle::DropDownList;
            tabNewOrder->Controls->Add(cmbCar);

            lblSelectServices = gcnew Label();
            lblSelectServices->Text = "Выберите услуги:";
            lblSelectServices->Location = Point(15, 80);
            lblSelectServices->AutoSize = true;
            lblSelectServices->Font = gcnew Drawing::Font("Segoe UI", 10);
            tabNewOrder->Controls->Add(lblSelectServices);

            clbServices = gcnew CheckedListBox();
            clbServices->Location = Point(15, 105);
            clbServices->Size = Drawing::Size(300, 200);
            tabNewOrder->Controls->Add(clbServices);

            lblComment = gcnew Label();
            lblComment->Text = "Комментарий:";
            lblComment->Location = Point(350, 15);
            lblComment->AutoSize = true;
            lblComment->Font = gcnew Drawing::Font("Segoe UI", 10);
            tabNewOrder->Controls->Add(lblComment);

            txtOrderComment = gcnew TextBox();
            txtOrderComment->Location = Point(350, 40);
            txtOrderComment->Size = Drawing::Size(300, 100);
            txtOrderComment->Multiline = true;
            tabNewOrder->Controls->Add(txtOrderComment);

            lblDate = gcnew Label();
            lblDate->Text = "Желаемая дата завершения:";
            lblDate->Location = Point(350, 150);
            lblDate->AutoSize = true;
            lblDate->Font = gcnew Drawing::Font("Segoe UI", 10);
            tabNewOrder->Controls->Add(lblDate);

            calendarDate = gcnew MonthCalendar();
            calendarDate->Location = Point(350, 175);
            calendarDate->MinDate = DateTime::Today;
            tabNewOrder->Controls->Add(calendarDate);

            btnCreateOrder = gcnew Button();
            btnCreateOrder->Text = "Оформить заказ";
            btnCreateOrder->Location = Point(15, 320);
            btnCreateOrder->Size = Drawing::Size(180, 40);
            btnCreateOrder->BackColor = Color::FromArgb(46, 204, 113);
            btnCreateOrder->ForeColor = Color::White;
            btnCreateOrder->FlatStyle = FlatStyle::Flat;
            btnCreateOrder->Font = gcnew Drawing::Font("Segoe UI", 11, FontStyle::Bold);
            btnCreateOrder->Click += gcnew EventHandler(this, &MainForm::OnCreateOrder);
            tabNewOrder->Controls->Add(btnCreateOrder);

            tabMain->TabPages->Add(tabNewOrder);

            // Загружаем данные
            LoadClientOrders();
            LoadCarsCombo();
            LoadServicesChecklist();
        }

        // =============================================
        // ВКЛАДКИ МЕХАНИКА
        // =============================================
        void SetupMechanicTabs() {
            // --- Мои заказы ---
            tabMechOrders = gcnew TabPage("Мои заказы");
            tabMechOrders->BackColor = Color::White;

            lblFilterStatus = gcnew Label();
            lblFilterStatus->Text = "Фильтр по статусу:";
            lblFilterStatus->Location = Point(10, 15);
            lblFilterStatus->AutoSize = true;
            tabMechOrders->Controls->Add(lblFilterStatus);

            cmbStatusFilter = gcnew ComboBox();
            cmbStatusFilter->Location = Point(140, 12);
            cmbStatusFilter->Size = Drawing::Size(200, 23);
            cmbStatusFilter->DropDownStyle = ComboBoxStyle::DropDownList;
            cmbStatusFilter->Items->Add("Все");
            cmbStatusFilter->Items->Add("Новый");
            cmbStatusFilter->Items->Add("В работе");
            cmbStatusFilter->Items->Add("Приостановлен");
            cmbStatusFilter->Items->Add("Завершен");
            cmbStatusFilter->SelectedIndex = 0;
            cmbStatusFilter->SelectedIndexChanged += gcnew EventHandler(this, &MainForm::OnStatusFilterChanged);
            tabMechOrders->Controls->Add(cmbStatusFilter);

            dgvMechOrders = gcnew DataGridView();
            dgvMechOrders->Location = Point(10, 45);
            dgvMechOrders->Size = Drawing::Size(950, 350);
            dgvMechOrders->AllowUserToAddRows = false;
            dgvMechOrders->ReadOnly = true;
            dgvMechOrders->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
            dgvMechOrders->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvMechOrders->BackgroundColor = Color::White;
            dgvMechOrders->CellFormatting += gcnew DataGridViewCellFormattingEventHandler(
                this, &MainForm::OnOrdersCellFormatting);
            dgvMechOrders->SelectionChanged += gcnew EventHandler(this, &MainForm::OnMechOrderSelected);
            tabMechOrders->Controls->Add(dgvMechOrders);

            tabMain->TabPages->Add(tabMechOrders);

            // --- Детали заказа ---
            tabOrderDetail = gcnew TabPage("Детали заказа");
            tabOrderDetail->BackColor = Color::White;
            tabOrderDetail->AutoScroll = true;

            lblCarInfo = gcnew Label();
            lblCarInfo->Text = "Информация об автомобиле:";
            lblCarInfo->Location = Point(10, 10);
            lblCarInfo->AutoSize = true;
            lblCarInfo->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
            tabOrderDetail->Controls->Add(lblCarInfo);

            // PropertyGrid для характеристик автомобиля
            propCarInfo = gcnew PropertyGrid();
            propCarInfo->Location = Point(10, 35);
            propCarInfo->Size = Drawing::Size(350, 200);
            propCarInfo->ToolbarVisible = false;
            propCarInfo->HelpVisible = false;
            tabOrderDetail->Controls->Add(propCarInfo);

            lblProgress = gcnew Label();
            lblProgress->Text = "Прогресс ремонта:";
            lblProgress->Location = Point(380, 10);
            lblProgress->AutoSize = true;
            lblProgress->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
            tabOrderDetail->Controls->Add(lblProgress);

            // ProgressBar для прогресса ремонта
            progressRepair = gcnew ProgressBar();
            progressRepair->Location = Point(380, 35);
            progressRepair->Size = Drawing::Size(580, 30);
            progressRepair->Style = ProgressBarStyle::Continuous;
            tabOrderDetail->Controls->Add(progressRepair);

            // Кнопки управления статусом
            btnStartWork = gcnew Button();
            btnStartWork->Text = "Начать работу";
            btnStartWork->Location = Point(380, 75);
            btnStartWork->Size = Drawing::Size(140, 35);
            btnStartWork->BackColor = Color::FromArgb(46, 204, 113);
            btnStartWork->ForeColor = Color::White;
            btnStartWork->FlatStyle = FlatStyle::Flat;
            btnStartWork->Click += gcnew EventHandler(this, &MainForm::OnStartWork);
            tabOrderDetail->Controls->Add(btnStartWork);

            btnPauseWork = gcnew Button();
            btnPauseWork->Text = "Приостановить";
            btnPauseWork->Location = Point(530, 75);
            btnPauseWork->Size = Drawing::Size(140, 35);
            btnPauseWork->BackColor = Color::FromArgb(243, 156, 18);
            btnPauseWork->ForeColor = Color::White;
            btnPauseWork->FlatStyle = FlatStyle::Flat;
            btnPauseWork->Click += gcnew EventHandler(this, &MainForm::OnPauseWork);
            tabOrderDetail->Controls->Add(btnPauseWork);

            btnFinishWork = gcnew Button();
            btnFinishWork->Text = "Завершить";
            btnFinishWork->Location = Point(680, 75);
            btnFinishWork->Size = Drawing::Size(140, 35);
            btnFinishWork->BackColor = Color::FromArgb(231, 76, 60);
            btnFinishWork->ForeColor = Color::White;
            btnFinishWork->FlatStyle = FlatStyle::Flat;
            btnFinishWork->Click += gcnew EventHandler(this, &MainForm::OnFinishWork);
            tabOrderDetail->Controls->Add(btnFinishWork);

            lblWorksList = gcnew Label();
            lblWorksList->Text = "Список работ:";
            lblWorksList->Location = Point(10, 245);
            lblWorksList->AutoSize = true;
            lblWorksList->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
            tabOrderDetail->Controls->Add(lblWorksList);

            dgvOrderWorks = gcnew DataGridView();
            dgvOrderWorks->Location = Point(10, 270);
            dgvOrderWorks->Size = Drawing::Size(470, 200);
            dgvOrderWorks->AllowUserToAddRows = false;
            dgvOrderWorks->ReadOnly = true;
            dgvOrderWorks->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvOrderWorks->BackgroundColor = Color::White;
            tabOrderDetail->Controls->Add(dgvOrderWorks);

            lblPartsList = gcnew Label();
            lblPartsList->Text = "Запчасти:";
            lblPartsList->Location = Point(500, 245);
            lblPartsList->AutoSize = true;
            lblPartsList->Font = gcnew Drawing::Font("Segoe UI", 10, FontStyle::Bold);
            tabOrderDetail->Controls->Add(lblPartsList);

            dgvOrderParts = gcnew DataGridView();
            dgvOrderParts->Location = Point(500, 270);
            dgvOrderParts->Size = Drawing::Size(460, 200);
            dgvOrderParts->AllowUserToAddRows = false;
            dgvOrderParts->ReadOnly = true;
            dgvOrderParts->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvOrderParts->BackgroundColor = Color::White;
            tabOrderDetail->Controls->Add(dgvOrderParts);

            tabMain->TabPages->Add(tabOrderDetail);

            // Загрузка данных
            LoadMechanicOrders();
        }

        // =============================================
        // ВКЛАДКИ МЕНЕДЖЕРА
        // =============================================
        void SetupManagerTabs() {
            // --- Управление заказами ---
            tabManageOrders = gcnew TabPage("Управление заказами");
            tabManageOrders->BackColor = Color::White;

            dgvAllOrders = gcnew DataGridView();
            dgvAllOrders->Location = Point(10, 10);
            dgvAllOrders->Size = Drawing::Size(950, 350);
            dgvAllOrders->AllowUserToAddRows = false;
            dgvAllOrders->ReadOnly = true;
            dgvAllOrders->SelectionMode = DataGridViewSelectionMode::FullRowSelect;
            dgvAllOrders->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvAllOrders->BackgroundColor = Color::White;
            dgvAllOrders->CellFormatting += gcnew DataGridViewCellFormattingEventHandler(
                this, &MainForm::OnOrdersCellFormatting);
            tabManageOrders->Controls->Add(dgvAllOrders);

            // Назначение механика
            lblAssignMech = gcnew Label();
            lblAssignMech->Text = "Назначить механика:";
            lblAssignMech->Location = Point(10, 370);
            lblAssignMech->AutoSize = true;
            tabManageOrders->Controls->Add(lblAssignMech);

            cmbAssignMechanic = gcnew ComboBox();
            cmbAssignMechanic->Location = Point(160, 367);
            cmbAssignMechanic->Size = Drawing::Size(250, 23);
            cmbAssignMechanic->DropDownStyle = ComboBoxStyle::DropDownList;
            tabManageOrders->Controls->Add(cmbAssignMechanic);

            btnAssign = gcnew Button();
            btnAssign->Text = "Назначить";
            btnAssign->Location = Point(420, 365);
            btnAssign->Size = Drawing::Size(100, 28);
            btnAssign->BackColor = Color::FromArgb(52, 152, 219);
            btnAssign->ForeColor = Color::White;
            btnAssign->FlatStyle = FlatStyle::Flat;
            btnAssign->Click += gcnew EventHandler(this, &MainForm::OnAssignMechanic);
            tabManageOrders->Controls->Add(btnAssign);

            // Изменение статуса
            lblChangeStatus = gcnew Label();
            lblChangeStatus->Text = "Изменить статус:";
            lblChangeStatus->Location = Point(10, 405);
            lblChangeStatus->AutoSize = true;
            tabManageOrders->Controls->Add(lblChangeStatus);

            cmbChangeStatus = gcnew ComboBox();
            cmbChangeStatus->Location = Point(160, 402);
            cmbChangeStatus->Size = Drawing::Size(250, 23);
            cmbChangeStatus->DropDownStyle = ComboBoxStyle::DropDownList;
            cmbChangeStatus->Items->Add("Новый");
            cmbChangeStatus->Items->Add("В работе");
            cmbChangeStatus->Items->Add("Приостановлен");
            cmbChangeStatus->Items->Add("Завершен");
            cmbChangeStatus->Items->Add("Выдан клиенту");
            tabManageOrders->Controls->Add(cmbChangeStatus);

            btnChangeStatus = gcnew Button();
            btnChangeStatus->Text = "Изменить";
            btnChangeStatus->Location = Point(420, 400);
            btnChangeStatus->Size = Drawing::Size(100, 28);
            btnChangeStatus->BackColor = Color::FromArgb(243, 156, 18);
            btnChangeStatus->ForeColor = Color::White;
            btnChangeStatus->FlatStyle = FlatStyle::Flat;
            btnChangeStatus->Click += gcnew EventHandler(this, &MainForm::OnChangeOrderStatus);
            tabManageOrders->Controls->Add(btnChangeStatus);

            tabMain->TabPages->Add(tabManageOrders);

            // --- Отчёт по доходам ---
            tabReport = gcnew TabPage("Отчёт по доходам");
            tabReport->BackColor = Color::White;

            lblFrom = gcnew Label();
            lblFrom->Text = "С даты:";
            lblFrom->Location = Point(10, 15);
            lblFrom->AutoSize = true;
            tabReport->Controls->Add(lblFrom);

            dtpFrom = gcnew DateTimePicker();
            dtpFrom->Location = Point(70, 12);
            dtpFrom->Size = Drawing::Size(200, 23);
            dtpFrom->Value = DateTime::Today.AddMonths(-1);
            tabReport->Controls->Add(dtpFrom);

            lblTo = gcnew Label();
            lblTo->Text = "По дату:";
            lblTo->Location = Point(290, 15);
            lblTo->AutoSize = true;
            tabReport->Controls->Add(lblTo);

            dtpTo = gcnew DateTimePicker();
            dtpTo->Location = Point(355, 12);
            dtpTo->Size = Drawing::Size(200, 23);
            tabReport->Controls->Add(dtpTo);

            btnGenerateReport = gcnew Button();
            btnGenerateReport->Text = "Сформировать отчёт";
            btnGenerateReport->Location = Point(575, 10);
            btnGenerateReport->Size = Drawing::Size(170, 28);
            btnGenerateReport->BackColor = Color::FromArgb(52, 152, 219);
            btnGenerateReport->ForeColor = Color::White;
            btnGenerateReport->FlatStyle = FlatStyle::Flat;
            btnGenerateReport->Click += gcnew EventHandler(this, &MainForm::OnGenerateReport);
            tabReport->Controls->Add(btnGenerateReport);

            dgvReport = gcnew DataGridView();
            dgvReport->Location = Point(10, 50);
            dgvReport->Size = Drawing::Size(950, 350);
            dgvReport->AllowUserToAddRows = false;
            dgvReport->ReadOnly = true;
            dgvReport->AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode::Fill;
            dgvReport->BackgroundColor = Color::White;
            tabReport->Controls->Add(dgvReport);

            lblTotalIncome = gcnew Label();
            lblTotalIncome->Text = "Итого: 0 руб.";
            lblTotalIncome->Location = Point(10, 410);
            lblTotalIncome->AutoSize = true;
            lblTotalIncome->Font = gcnew Drawing::Font("Segoe UI", 14, FontStyle::Bold);
            lblTotalIncome->ForeColor = Color::FromArgb(46, 204, 113);
            tabReport->Controls->Add(lblTotalIncome);

            tabMain->TabPages->Add(tabReport);

            // Загрузка данных
            LoadAllOrders();
            LoadMechanicsList();
        }

        // =============================================
        // ТАЙМЕР НЕАКТИВНОСТИ
        // =============================================
        void StartInactivityTimer() {
            lastActivity = DateTime::Now;

            inactivityTimer = gcnew Timer();
            inactivityTimer->Interval = 60000; // проверяем каждую минуту
            inactivityTimer->Tick += gcnew EventHandler(this, &MainForm::OnTimerTick);
            inactivityTimer->Start();
        }

        void OnTimerTick(Object^ sender, EventArgs^ e) {
            TimeSpan diff = DateTime::Now - lastActivity;
            int minutesLeft = timeoutMinutes - (int)diff.TotalMinutes;

            if (minutesLeft <= 0) {
                inactivityTimer->Stop();
                MessageBox::Show(
                    "Сессия истекла из-за неактивности (15 минут).\nПожалуйста, войдите заново.",
                    "Таймаут сессии",
                    MessageBoxButtons::OK,
                    MessageBoxIcon::Information);
                this->DialogResult = System::Windows::Forms::DialogResult::Retry;
                this->Close();
            }
            else {
                lblTimer->Text = "Таймаут через: " + minutesLeft + " мин.";
            }
        }

        void OnUserActivity(Object^ sender, MouseEventArgs^ e) {
            lastActivity = DateTime::Now;
        }

        void OnKeyActivity(Object^ sender, KeyPressEventArgs^ e) {
            lastActivity = DateTime::Now;
        }

        // =============================================
        // ЗАГРУЗКА ДАННЫХ
        // =============================================

        // Панель мониторинга (Admin)
        void LoadDashboard() {
            try {
                Object^ count = db->ExecuteScalar(
                    "SELECT COUNT(*) FROM Orders WHERE Status IN (N'Новый', N'В работе', N'Приостановлен')", nullptr);
                if (count != nullptr) lblOpenOrdersValue->Text = count->ToString();

                Object^ mechCount = db->ExecuteScalar(
                    "SELECT COUNT(DISTINCT MechanicId) FROM Orders WHERE Status = N'В работе'", nullptr);
                if (mechCount != nullptr) lblMechanicsLoadValue->Text = mechCount->ToString();
            }
            catch (Exception^) {
                // Если таблицы еще не созданы
            }
        }

        // Услуги (Admin)
        void LoadServices() {
            DataTable^ dt = db->ExecuteQuery("SELECT ServiceId, ServiceName AS [Название], Price AS [Цена] FROM Services");
            dgvServices->DataSource = dt;
            if (dgvServices->Columns->Count > 0 && dgvServices->Columns["ServiceId"] != nullptr)
                dgvServices->Columns["ServiceId"]->Visible = false;
        }

        // Запчасти (Admin)
        void LoadParts() {
            DataTable^ dt = db->ExecuteQuery(
                "SELECT PartId, PartName AS [Название], Price AS [Цена], Quantity AS [Кол-во] FROM Parts");
            dgvParts->DataSource = dt;
            if (dgvParts->Columns->Count > 0 && dgvParts->Columns["PartId"] != nullptr)
                dgvParts->Columns["PartId"]->Visible = false;
        }

        // Заказы клиента
        void LoadClientOrders() {
            DataTable^ dt = db->ExecuteQuery(
                "SELECT o.OrderId, c.Brand + ' ' + c.Model AS [Автомобиль], "
                "o.Status AS [Статус], o.CreatedDate AS [Дата создания], "
                "o.DesiredDate AS [Желаемая дата], o.Comment AS [Комментарий] "
                "FROM Orders o "
                "INNER JOIN Cars c ON o.CarId = c.CarId "
                "WHERE o.ClientId = (SELECT UserId FROM Users WHERE Login = @login)",
                gcnew array<SqlParameter^>{ gcnew SqlParameter("@login", currentUser) });
            dgvMyOrders->DataSource = dt;
            if (dgvMyOrders->Columns->Count > 0 && dgvMyOrders->Columns["OrderId"] != nullptr)
                dgvMyOrders->Columns["OrderId"]->Visible = false;
        }

        // Автомобили для комбобокса
        void LoadCarsCombo() {
            DataTable^ dt = db->ExecuteQuery(
                "SELECT CarId, Brand + ' ' + Model + ' (' + CAST(Year AS VARCHAR) + ')' AS CarName "
                "FROM Cars WHERE OwnerId = (SELECT UserId FROM Users WHERE Login = @login)",
                gcnew array<SqlParameter^>{ gcnew SqlParameter("@login", currentUser) });
            cmbCar->DataSource = dt;
            cmbCar->DisplayMember = "CarName";
            cmbCar->ValueMember = "CarId";
        }

        // Услуги для CheckedListBox
        void LoadServicesChecklist() {
            DataTable^ dt = db->ExecuteQuery("SELECT ServiceId, ServiceName + ' - ' + CAST(Price AS VARCHAR) + ' руб.' AS Descr FROM Services");
            clbServices->Items->Clear();
            for (int i = 0; i < dt->Rows->Count; i++) {
                clbServices->Items->Add(dt->Rows[i]["Descr"]->ToString());
            }
        }

        // Заказы механика
        void LoadMechanicOrders() {
            String^ query = "SELECT o.OrderId, c.Brand + ' ' + c.Model AS [Автомобиль], "
                "o.Status AS [Статус], o.CreatedDate AS [Дата создания], "
                "o.Comment AS [Комментарий] "
                "FROM Orders o "
                "INNER JOIN Cars c ON o.CarId = c.CarId "
                "WHERE o.MechanicId = (SELECT UserId FROM Users WHERE Login = @login)";

            String^ filter = cmbStatusFilter->SelectedItem->ToString();
            if (filter != "Все") {
                query += " AND o.Status = @status";
                dgvMechOrders->DataSource = db->ExecuteQuery(query,
                    gcnew array<SqlParameter^>{
                        gcnew SqlParameter("@login", currentUser),
                        gcnew SqlParameter("@status", filter) });
            }
            else {
                dgvMechOrders->DataSource = db->ExecuteQuery(query,
                    gcnew array<SqlParameter^>{ gcnew SqlParameter("@login", currentUser) });
            }

            if (dgvMechOrders->Columns->Count > 0 && dgvMechOrders->Columns["OrderId"] != nullptr)
                dgvMechOrders->Columns["OrderId"]->Visible = false;
        }

        // Все заказы (Manager)
        void LoadAllOrders() {
            DataTable^ dt = db->ExecuteQuery(
                "SELECT o.OrderId, c.Brand + ' ' + c.Model AS [Автомобиль], "
                "u.Login AS [Клиент], m.Login AS [Механик], "
                "o.Status AS [Статус], o.CreatedDate AS [Дата создания] "
                "FROM Orders o "
                "INNER JOIN Cars c ON o.CarId = c.CarId "
                "INNER JOIN Users u ON o.ClientId = u.UserId "
                "LEFT JOIN Users m ON o.MechanicId = m.UserId");
            dgvAllOrders->DataSource = dt;
            if (dgvAllOrders->Columns->Count > 0 && dgvAllOrders->Columns["OrderId"] != nullptr)
                dgvAllOrders->Columns["OrderId"]->Visible = false;
        }

        // Список механиков (Manager)
        void LoadMechanicsList() {
            DataTable^ dt = db->ExecuteQuery(
                "SELECT UserId, Login FROM Users u "
                "INNER JOIN Roles r ON u.RoleId = r.RoleId WHERE r.RoleName = 'Mechanic'");
            cmbAssignMechanic->DataSource = dt;
            cmbAssignMechanic->DisplayMember = "Login";
            cmbAssignMechanic->ValueMember = "UserId";
        }

        // =============================================
        // ОБРАБОТЧИКИ СОБЫТИЙ
        // =============================================

        // Цветовая индикация статуса заказа в DataGridView
        void OnOrdersCellFormatting(Object^ sender, DataGridViewCellFormattingEventArgs^ e) {
            DataGridView^ dgv = safe_cast<DataGridView^>(sender);
            if (dgv->Columns[e->ColumnIndex]->HeaderText == "Статус" && e->Value != nullptr) {
                String^ status = e->Value->ToString();
                if (status == "Новый") {
                    e->CellStyle->BackColor = Color::FromArgb(214, 234, 248);
                    e->CellStyle->ForeColor = Color::FromArgb(41, 128, 185);
                }
                else if (status == "В работе") {
                    e->CellStyle->BackColor = Color::FromArgb(212, 239, 223);
                    e->CellStyle->ForeColor = Color::FromArgb(39, 174, 96);
                }
                else if (status == "Приостановлен") {
                    e->CellStyle->BackColor = Color::FromArgb(252, 243, 207);
                    e->CellStyle->ForeColor = Color::FromArgb(243, 156, 18);
                }
                else if (status == "Завершен") {
                    e->CellStyle->BackColor = Color::FromArgb(235, 235, 235);
                    e->CellStyle->ForeColor = Color::FromArgb(100, 100, 100);
                }
                else if (status == "Выдан клиенту") {
                    e->CellStyle->BackColor = Color::FromArgb(209, 242, 235);
                    e->CellStyle->ForeColor = Color::FromArgb(22, 160, 133);
                }
            }
        }

        // ---- Admin ----
        void OnRefreshDashboard(Object^ sender, EventArgs^ e) {
            LoadDashboard();
            lblStatus->Text = "Панель обновлена";
        }

        void OnAddService(Object^ sender, EventArgs^ e) {
            if (String::IsNullOrWhiteSpace(txtServiceName->Text)) {
                MessageBox::Show("Введите название услуги!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            Decimal price;
            if (!Decimal::TryParse(txtServicePrice->Text, price)) {
                MessageBox::Show("Введите корректную цену!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            int res = db->ExecuteNonQuery(
                "INSERT INTO Services (ServiceName, Price) VALUES (@name, @price)",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@name", txtServiceName->Text),
                    gcnew SqlParameter("@price", price) });
            if (res > 0) {
                LoadServices();
                txtServiceName->Text = "";
                txtServicePrice->Text = "";
                lblStatus->Text = "Услуга добавлена";
            }
        }

        void OnEditService(Object^ sender, EventArgs^ e) {
            if (dgvServices->SelectedRows->Count == 0) {
                MessageBox::Show("Выберите услугу для изменения!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            if (String::IsNullOrWhiteSpace(txtServiceName->Text)) {
                MessageBox::Show("Введите название услуги!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            Decimal price;
            if (!Decimal::TryParse(txtServicePrice->Text, price)) {
                MessageBox::Show("Введите корректную цену!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            int id = Convert::ToInt32(dgvServices->SelectedRows[0]->Cells["ServiceId"]->Value);
            int res = db->ExecuteNonQuery(
                "UPDATE Services SET ServiceName = @name, Price = @price WHERE ServiceId = @id",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@name", txtServiceName->Text),
                    gcnew SqlParameter("@price", price),
                    gcnew SqlParameter("@id", id) });
            if (res > 0) {
                LoadServices();
                lblStatus->Text = "Услуга обновлена";
            }
        }

        void OnDeleteService(Object^ sender, EventArgs^ e) {
            if (dgvServices->SelectedRows->Count == 0) {
                MessageBox::Show("Выберите услугу для удаления!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            if (MessageBox::Show("Удалить выбранную услугу?", "Подтверждение",
                MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes) {
                int id = Convert::ToInt32(dgvServices->SelectedRows[0]->Cells["ServiceId"]->Value);
                int res = db->ExecuteNonQuery("DELETE FROM Services WHERE ServiceId = @id",
                    gcnew array<SqlParameter^>{ gcnew SqlParameter("@id", id) });
                if (res > 0) {
                    LoadServices();
                    lblStatus->Text = "Услуга удалена";
                }
            }
        }

        void OnAddPart(Object^ sender, EventArgs^ e) {
            if (String::IsNullOrWhiteSpace(txtPartName->Text)) {
                MessageBox::Show("Введите название запчасти!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            Decimal price;
            int qty;
            if (!Decimal::TryParse(txtPartPrice->Text, price)) {
                MessageBox::Show("Введите корректную цену!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            if (!Int32::TryParse(txtPartQty->Text, qty)) {
                MessageBox::Show("Введите корректное количество!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            int res = db->ExecuteNonQuery(
                "INSERT INTO Parts (PartName, Price, Quantity) VALUES (@name, @price, @qty)",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@name", txtPartName->Text),
                    gcnew SqlParameter("@price", price),
                    gcnew SqlParameter("@qty", qty) });
            if (res > 0) {
                LoadParts();
                txtPartName->Text = "";
                txtPartPrice->Text = "";
                txtPartQty->Text = "";
                lblStatus->Text = "Запчасть добавлена";
            }
        }

        void OnEditPart(Object^ sender, EventArgs^ e) {
            if (dgvParts->SelectedRows->Count == 0) {
                MessageBox::Show("Выберите запчасть!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            if (String::IsNullOrWhiteSpace(txtPartName->Text)) {
                MessageBox::Show("Введите название!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            Decimal price; int qty;
            if (!Decimal::TryParse(txtPartPrice->Text, price) || !Int32::TryParse(txtPartQty->Text, qty)) {
                MessageBox::Show("Проверьте цену и количество!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            int id = Convert::ToInt32(dgvParts->SelectedRows[0]->Cells["PartId"]->Value);
            db->ExecuteNonQuery(
                "UPDATE Parts SET PartName = @name, Price = @price, Quantity = @qty WHERE PartId = @id",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@name", txtPartName->Text),
                    gcnew SqlParameter("@price", price),
                    gcnew SqlParameter("@qty", qty),
                    gcnew SqlParameter("@id", id) });
            LoadParts();
            lblStatus->Text = "Запчасть обновлена";
        }

        void OnDeletePart(Object^ sender, EventArgs^ e) {
            if (dgvParts->SelectedRows->Count == 0) return;
            if (MessageBox::Show("Удалить выбранную запчасть?", "Подтверждение",
                MessageBoxButtons::YesNo, MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::Yes) {
                int id = Convert::ToInt32(dgvParts->SelectedRows[0]->Cells["PartId"]->Value);
                db->ExecuteNonQuery("DELETE FROM Parts WHERE PartId = @id",
                    gcnew array<SqlParameter^>{ gcnew SqlParameter("@id", id) });
                LoadParts();
                lblStatus->Text = "Запчасть удалена";
            }
        }

        // ---- Client ----
        void OnNewOrderTab(Object^ sender, EventArgs^ e) {
            tabMain->SelectedTab = tabNewOrder;
        }

        void OnCreateOrder(Object^ sender, EventArgs^ e) {
            if (cmbCar->SelectedValue == nullptr) {
                MessageBox::Show("Выберите автомобиль!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            if (clbServices->CheckedItems->Count == 0) {
                MessageBox::Show("Выберите хотя бы одну услугу!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }

            int carId = Convert::ToInt32(cmbCar->SelectedValue);
            DateTime desiredDate = calendarDate->SelectionStart;

            // Создаем заказ
            Object^ orderId = db->ExecuteScalar(
                "INSERT INTO Orders (CarId, ClientId, Status, CreatedDate, DesiredDate, Comment) "
                "OUTPUT INSERTED.OrderId "
                "VALUES (@carId, (SELECT UserId FROM Users WHERE Login = @login), "
                "N'Новый', GETDATE(), @date, @comment)",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@carId", carId),
                    gcnew SqlParameter("@login", currentUser),
                    gcnew SqlParameter("@date", desiredDate),
                    gcnew SqlParameter("@comment", txtOrderComment->Text) });

            if (orderId != nullptr) {
                // Добавляем услуги к заказу
                DataTable^ servicesTable = db->ExecuteQuery("SELECT ServiceId, ServiceName FROM Services");
                for (int i = 0; i < clbServices->CheckedIndices->Count; i++) {
                    int idx = clbServices->CheckedIndices[i];
                    if (idx < servicesTable->Rows->Count) {
                        int serviceId = Convert::ToInt32(servicesTable->Rows[idx]["ServiceId"]);
                        db->ExecuteNonQuery(
                            "INSERT INTO OrderServices (OrderId, ServiceId) VALUES (@oid, @sid)",
                            gcnew array<SqlParameter^>{
                                gcnew SqlParameter("@oid", orderId),
                                gcnew SqlParameter("@sid", serviceId) });
                    }
                }

                MessageBox::Show("Заказ успешно создан!", "Успех",
                    MessageBoxButtons::OK, MessageBoxIcon::Information);
                LoadClientOrders();
                tabMain->SelectedTab = tabMyOrders;

                // Уведомление в статусбаре
                lblStatus->Text = "Заказ #" + orderId->ToString() + " создан";
            }
        }

        // ---- Mechanic ----
        void OnStatusFilterChanged(Object^ sender, EventArgs^ e) {
            LoadMechanicOrders();
        }

        void OnMechOrderSelected(Object^ sender, EventArgs^ e) {
            if (dgvMechOrders->SelectedRows->Count == 0) return;

            int orderId = Convert::ToInt32(dgvMechOrders->SelectedRows[0]->Cells["OrderId"]->Value);

            // Загружаем работы по заказу
            dgvOrderWorks->DataSource = db->ExecuteQuery(
                "SELECT s.ServiceName AS [Услуга], s.Price AS [Цена] "
                "FROM OrderServices os INNER JOIN Services s ON os.ServiceId = s.ServiceId "
                "WHERE os.OrderId = @id",
                gcnew array<SqlParameter^>{ gcnew SqlParameter("@id", orderId) });

            // Загружаем запчасти
            dgvOrderParts->DataSource = db->ExecuteQuery(
                "SELECT p.PartName AS [Запчасть], op.Quantity AS [Кол-во], p.Price AS [Цена] "
                "FROM OrderParts op INNER JOIN Parts p ON op.PartId = p.PartId "
                "WHERE op.OrderId = @id",
                gcnew array<SqlParameter^>{ gcnew SqlParameter("@id", orderId) });

            // Обновляем прогресс
            String^ status = dgvMechOrders->SelectedRows[0]->Cells["Статус"]->Value->ToString();
            if (status == "Новый") progressRepair->Value = 0;
            else if (status == "В работе") progressRepair->Value = 50;
            else if (status == "Приостановлен") progressRepair->Value = 30;
            else if (status == "Завершен") progressRepair->Value = 100;
        }

        void UpdateOrderStatus(String^ newStatus) {
            if (dgvMechOrders->SelectedRows->Count == 0) {
                MessageBox::Show("Выберите заказ!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            int orderId = Convert::ToInt32(dgvMechOrders->SelectedRows[0]->Cells["OrderId"]->Value);
            db->ExecuteNonQuery(
                "UPDATE Orders SET Status = @status WHERE OrderId = @id",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@status", newStatus),
                    gcnew SqlParameter("@id", orderId) });
            LoadMechanicOrders();
            lblStatus->Text = "Статус заказа изменен на: " + newStatus;
        }

        void OnStartWork(Object^ sender, EventArgs^ e) {
            UpdateOrderStatus("В работе");
            progressRepair->Value = 50;
        }

        void OnPauseWork(Object^ sender, EventArgs^ e) {
            UpdateOrderStatus("Приостановлен");
            progressRepair->Value = 30;
        }

        void OnFinishWork(Object^ sender, EventArgs^ e) {
            UpdateOrderStatus("Завершен");
            progressRepair->Value = 100;
        }

        // ---- Manager ----
        void OnAssignMechanic(Object^ sender, EventArgs^ e) {
            if (dgvAllOrders->SelectedRows->Count == 0 || cmbAssignMechanic->SelectedValue == nullptr) {
                MessageBox::Show("Выберите заказ и механика!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            int orderId = Convert::ToInt32(dgvAllOrders->SelectedRows[0]->Cells["OrderId"]->Value);
            int mechId = Convert::ToInt32(cmbAssignMechanic->SelectedValue);
            db->ExecuteNonQuery(
                "UPDATE Orders SET MechanicId = @mechId WHERE OrderId = @orderId",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@mechId", mechId),
                    gcnew SqlParameter("@orderId", orderId) });
            LoadAllOrders();
            lblStatus->Text = "Механик назначен";
        }

        void OnChangeOrderStatus(Object^ sender, EventArgs^ e) {
            if (dgvAllOrders->SelectedRows->Count == 0 || cmbChangeStatus->SelectedItem == nullptr) {
                MessageBox::Show("Выберите заказ и статус!", "Ошибка", MessageBoxButtons::OK, MessageBoxIcon::Warning);
                return;
            }
            int orderId = Convert::ToInt32(dgvAllOrders->SelectedRows[0]->Cells["OrderId"]->Value);
            db->ExecuteNonQuery(
                "UPDATE Orders SET Status = @status WHERE OrderId = @orderId",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@status", cmbChangeStatus->SelectedItem->ToString()),
                    gcnew SqlParameter("@orderId", orderId) });
            LoadAllOrders();
            lblStatus->Text = "Статус заказа изменен";
        }

        void OnGenerateReport(Object^ sender, EventArgs^ e) {
            DataTable^ dt = db->ExecuteQuery(
                "SELECT s.ServiceName AS [Услуга], COUNT(*) AS [Кол-во заказов], "
                "SUM(s.Price) AS [Сумма] "
                "FROM Orders o "
                "INNER JOIN OrderServices os ON o.OrderId = os.OrderId "
                "INNER JOIN Services s ON os.ServiceId = s.ServiceId "
                "WHERE o.CreatedDate BETWEEN @from AND @to "
                "GROUP BY s.ServiceName "
                "ORDER BY [Сумма] DESC",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@from", dtpFrom->Value),
                    gcnew SqlParameter("@to", dtpTo->Value) });
            dgvReport->DataSource = dt;

            // Считаем итог
            Object^ total = db->ExecuteScalar(
                "SELECT SUM(s.Price) FROM Orders o "
                "INNER JOIN OrderServices os ON o.OrderId = os.OrderId "
                "INNER JOIN Services s ON os.ServiceId = s.ServiceId "
                "WHERE o.CreatedDate BETWEEN @from AND @to",
                gcnew array<SqlParameter^>{
                    gcnew SqlParameter("@from", dtpFrom->Value),
                    gcnew SqlParameter("@to", dtpTo->Value) });
            if (total != nullptr && total != DBNull::Value)
                lblTotalIncome->Text = "Итого: " + Convert::ToDecimal(total).ToString("N2") + " руб.";
            else
                lblTotalIncome->Text = "Итого: 0 руб.";

            lblStatus->Text = "Отчёт сформирован";
        }

        // ---- Меню ----
        void OnLogout(Object^ sender, EventArgs^ e) {
            inactivityTimer->Stop();
            this->DialogResult = System::Windows::Forms::DialogResult::Retry;
            this->Close();
        }

        void OnExit(Object^ sender, EventArgs^ e) {
            inactivityTimer->Stop();
            this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
            this->Close();
        }
    };
}
