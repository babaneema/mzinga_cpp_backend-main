-- phpMyAdmin SQL Dump
-- version 5.1.1deb5ubuntu1
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Nov 15, 2024 at 06:13 AM
-- Server version: 8.0.39-0ubuntu0.22.04.1
-- PHP Version: 8.1.2-1ubuntu2.19

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `admin`
--

-- --------------------------------------------------------

--
-- Table structure for table `companies`
--

CREATE TABLE `companies` (
  `company_id` bigint UNSIGNED NOT NULL,
  `company` text NOT NULL,
  `database_username` text NOT NULL,
  `database_password` text NOT NULL,
  `database_name` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `companies`
--

INSERT INTO `companies` (`company_id`, `company`, `database_username`, `database_password`, `database_name`) VALUES
(1, 'mzingamaji', 'sql_mzingamaji_c', 'NEs578aPybpeTe6P', 'sql_mzingamaji_c'),
(2, 'loganimaji', 'logan_user', 'logan_password', 'logan_system');

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` bigint UNSIGNED NOT NULL,
  `company` text NOT NULL,
  `phone` text NOT NULL,
  `password` text NOT NULL,
  `reg_date` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`id`, `company`, `phone`, `password`, `reg_date`) VALUES
(2, 'mzingamaji', '255783300990', 'e744e838-d64f-40d8-9c00-2f2008ec361c:df824338dc85e23f', '2024-11-02'),
(3, 'mzingamaji', '255763096136', 'be0f7b67-e5c2-4884-84d7-e955e61fd289:b15ed9c81a706b74', '2024-11-05'),
(4, 'loganimaji', '2557171041141', 'e744e838-d64f-40d8-9c00-2f2008ec361c:df824338dc85e23f', '2024-11-14');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `companies`
--
ALTER TABLE `companies`
  ADD PRIMARY KEY (`company_id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `companies`
--
ALTER TABLE `companies`
  MODIFY `company_id` bigint UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;

--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `id` bigint UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
