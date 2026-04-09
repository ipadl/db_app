-- =============================================
-- Скрипт создания базы данных AutoServiceDB
-- Лабораторная работа №3 - Вариант 2 (Автосервис)
-- =============================================

-- Создание БД
IF NOT EXISTS (SELECT name FROM sys.databases WHERE name = N'AutoServiceDB')
    CREATE DATABASE AutoServiceDB;
GO

USE AutoServiceDB;
GO

-- =============================================
-- Таблица ролей
-- =============================================
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'Roles')
CREATE TABLE Roles (
    RoleId INT IDENTITY(1,1) PRIMARY KEY,
    RoleName NVARCHAR(50) NOT NULL UNIQUE
);
GO

-- Вставляем роли
IF NOT EXISTS (SELECT * FROM Roles WHERE RoleName = 'Admin')
    INSERT INTO Roles (RoleName) VALUES ('Admin'), ('Client'), ('Mechanic'), ('Manager');
GO

-- =============================================
-- Таблица пользователей
-- =============================================
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'Users')
CREATE TABLE Users (
    UserId INT IDENTITY(1,1) PRIMARY KEY,
    Login NVARCHAR(100) NOT NULL UNIQUE,
    PasswordHash VARBINARY(256) NOT NULL,
    FullName NVARCHAR(200),
    RoleId INT NOT NULL FOREIGN KEY REFERENCES Roles(RoleId),
    CreatedDate DATETIME DEFAULT GETDATE()
);
GO

-- =============================================
-- Таблица автомобилей
-- =============================================
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'Cars')
CREATE TABLE Cars (
    CarId INT IDENTITY(1,1) PRIMARY KEY,
    OwnerId INT NOT NULL FOREIGN KEY REFERENCES Users(UserId),
    Brand NVARCHAR(100) NOT NULL,
    Model NVARCHAR(100) NOT NULL,
    Year INT,
    VIN NVARCHAR(17),
    Color NVARCHAR(50),
    EngineType NVARCHAR(50),
    Mileage INT
);
GO

-- =============================================
-- Таблица услуг
-- =============================================
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'Services')
CREATE TABLE Services (
    ServiceId INT IDENTITY(1,1) PRIMARY KEY,
    ServiceName NVARCHAR(200) NOT NULL,
    Price DECIMAL(10,2) NOT NULL
);
GO

-- =============================================
-- Таблица запчастей
-- =============================================
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'Parts')
CREATE TABLE Parts (
    PartId INT IDENTITY(1,1) PRIMARY KEY,
    PartName NVARCHAR(200) NOT NULL,
    Price DECIMAL(10,2) NOT NULL,
    Quantity INT NOT NULL DEFAULT 0
);
GO

-- =============================================
-- Таблица заказов
-- =============================================
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'Orders')
CREATE TABLE Orders (
    OrderId INT IDENTITY(1,1) PRIMARY KEY,
    CarId INT NOT NULL FOREIGN KEY REFERENCES Cars(CarId),
    ClientId INT NOT NULL FOREIGN KEY REFERENCES Users(UserId),
    MechanicId INT NULL FOREIGN KEY REFERENCES Users(UserId),
    Status NVARCHAR(50) NOT NULL DEFAULT N'Новый',
    CreatedDate DATETIME DEFAULT GETDATE(),
    DesiredDate DATETIME,
    Comment NVARCHAR(500)
);
GO

-- =============================================
-- Таблица связи заказов и услуг
-- =============================================
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'OrderServices')
CREATE TABLE OrderServices (
    OrderServiceId INT IDENTITY(1,1) PRIMARY KEY,
    OrderId INT NOT NULL FOREIGN KEY REFERENCES Orders(OrderId),
    ServiceId INT NOT NULL FOREIGN KEY REFERENCES Services(ServiceId)
);
GO

-- =============================================
-- Таблица связи заказов и запчастей
-- =============================================
IF NOT EXISTS (SELECT * FROM sys.tables WHERE name = 'OrderParts')
CREATE TABLE OrderParts (
    OrderPartId INT IDENTITY(1,1) PRIMARY KEY,
    OrderId INT NOT NULL FOREIGN KEY REFERENCES Orders(OrderId),
    PartId INT NOT NULL FOREIGN KEY REFERENCES Parts(PartId),
    Quantity INT NOT NULL DEFAULT 1
);
GO

-- =============================================
-- Тестовые данные
-- =============================================

-- Пользователи (пароли: admin, client, mechanic, manager)
IF NOT EXISTS (SELECT * FROM Users WHERE Login = 'admin')
BEGIN
    INSERT INTO Users (Login, PasswordHash, FullName, RoleId) VALUES
        ('admin', HASHBYTES('SHA2_256', N'admin'), N'Администратор Иванов', 1),
        ('client1', HASHBYTES('SHA2_256', N'client'), N'Петров Иван Сергеевич', 2),
        ('mechanic1', HASHBYTES('SHA2_256', N'mechanic'), N'Сидоров Алексей Петрович', 3),
        ('manager1', HASHBYTES('SHA2_256', N'manager'), N'Козлова Мария Ивановна', 4);
END
GO

-- Услуги
IF NOT EXISTS (SELECT * FROM Services)
BEGIN
    INSERT INTO Services (ServiceName, Price) VALUES
        (N'Замена масла', 1500.00),
        (N'Замена тормозных колодок', 3000.00),
        (N'Диагностика двигателя', 2500.00),
        (N'Шиномонтаж', 2000.00),
        (N'Замена фильтров', 1200.00),
        (N'Ремонт подвески', 8000.00),
        (N'Покраска кузова', 15000.00),
        (N'Замена аккумулятора', 500.00);
END
GO

-- Запчасти
IF NOT EXISTS (SELECT * FROM Parts)
BEGIN
    INSERT INTO Parts (PartName, Price, Quantity) VALUES
        (N'Масло моторное 5W-30 (4л)', 2500.00, 20),
        (N'Фильтр масляный', 450.00, 30),
        (N'Фильтр воздушный', 600.00, 25),
        (N'Колодки тормозные передние', 2200.00, 15),
        (N'Колодки тормозные задние', 1800.00, 15),
        (N'Аккумулятор 60Ah', 5500.00, 10),
        (N'Свечи зажигания (комплект)', 1200.00, 20),
        (N'Антифриз (5л)', 800.00, 30);
END
GO

-- Автомобили (для client1)
IF NOT EXISTS (SELECT * FROM Cars)
BEGIN
    INSERT INTO Cars (OwnerId, Brand, Model, Year, VIN, Color, EngineType, Mileage) VALUES
        (2, N'Toyota', N'Camry', 2020, '1HGBH41JXMN109186', N'Белый', N'Бензин 2.5', 45000),
        (2, N'Hyundai', N'Solaris', 2019, '2HGBH41JXMN109187', N'Серый', N'Бензин 1.6', 67000);
END
GO

-- Тестовый заказ
IF NOT EXISTS (SELECT * FROM Orders)
BEGIN
    INSERT INTO Orders (CarId, ClientId, MechanicId, Status, CreatedDate, DesiredDate, Comment) VALUES
        (1, 2, 3, N'В работе', '2026-04-01', '2026-04-10', N'Плановое ТО');

    INSERT INTO OrderServices (OrderId, ServiceId) VALUES (1, 1), (1, 5);
END
GO

PRINT N'База данных AutoServiceDB создана и заполнена тестовыми данными.';
PRINT N'';
PRINT N'Тестовые пользователи:';
PRINT N'  admin / admin       (Администратор)';
PRINT N'  client1 / client    (Клиент)';
PRINT N'  mechanic1 / mechanic (Механик)';
PRINT N'  manager1 / manager  (Менеджер)';
GO
