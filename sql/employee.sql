-- phpMyAdmin SQL Dump
-- version 5.1.1deb5ubuntu1
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Nov 15, 2024 at 05:08 PM
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
-- Database: `sql_mzingamaji_c`
--

-- --------------------------------------------------------

--
-- Table structure for table `employee`
--

CREATE TABLE `employee` (
  `employee_id` bigint UNSIGNED NOT NULL,
  `employee_branch` int UNSIGNED NOT NULL,
  `employee_unique` text NOT NULL,
  `employee_name` text NOT NULL,
  `employee_gender` text NOT NULL,
  `employee_contact` text NOT NULL,
  `employee_address` text NOT NULL,
  `employee_administrative` text NOT NULL,
  `employee_reg_date` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

--
-- Dumping data for table `employee`
--

INSERT INTO `employee` (`employee_id`, `employee_branch`, `employee_unique`, `employee_name`, `employee_gender`, `employee_contact`, `employee_address`, `employee_administrative`, `employee_reg_date`) VALUES
(2, 1, '9e9a30b5-8cc6-4cff-a16a-90599610d8ba', 'Mzee Mitela', 'Male', '255783300990', 'Mzinga', 'Administrator', '2024-11-02'),
(3, 1, '5158b753-38f7-42c4-b7c4-cfc4d43abeab', 'Sunday Phery', 'Male', '255763096136', 'Mbagala', 'Worker', '2024-11-05');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `employee`
--
ALTER TABLE `employee`
  ADD PRIMARY KEY (`employee_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `employee`
--
ALTER TABLE `employee`
  MODIFY `employee_id` bigint UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=4;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
