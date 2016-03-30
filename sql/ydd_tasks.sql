-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Mar 31, 2016 at 01:18 AM
-- Server version: 5.5.48
-- PHP Version: 5.4.38

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

--
-- Database: `ydd_tasks`
--

-- --------------------------------------------------------

--
-- Table structure for table `tasks`
--

CREATE TABLE IF NOT EXISTS `tasks` (
	  `id` int(10) unsigned NOT NULL,
	    `userid` int(10) unsigned NOT NULL,
	      `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
	        `token` char(64) COLLATE utf8_unicode_ci NOT NULL
	) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `tasks`
--

INSERT INTO `tasks` (`id`, `userid`, `created`, `token`) VALUES
(1, 173025, 0x323031362d30332d33302031383a35383a3338, 'c9f13bf86c694e629440c6d56dd29b1e');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `tasks`
--
ALTER TABLE `tasks`
  ADD PRIMARY KEY (`id`);

  --
  -- AUTO_INCREMENT for dumped tables
  --

  --
  -- AUTO_INCREMENT for table `tasks`
  --
  ALTER TABLE `tasks`
    MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
