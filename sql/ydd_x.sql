-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Mar 31, 2016 at 01:16 AM
-- Server version: 5.5.48
-- PHP Version: 5.4.38

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

--
-- Database: `ydd_173025`
--

DELIMITER $$
--
-- Procedures
--
CREATE DEFINER=`root`@`localhost` PROCEDURE `sp_log_html_record`(IN `p_taskid` INT UNSIGNED, IN `p_type` TINYINT UNSIGNED, IN `p_message` VARCHAR(4096), IN `p_data` MEDIUMTEXT, OUT `p_result` TINYINT UNSIGNED)
        NO SQL
	BEGIN
	DECLARE EXIT HANDLER FOR SQLEXCEPTION ROLLBACK;
	DECLARE EXIT HANDLER FOR SQLWARNING ROLLBACK;

	START TRANSACTION;
	SET p_result = 0;

	INSERT INTO `tasks_logs_records`(`taskid`, `message`, `type`)
    VALUES(p_taskid, p_message, p_type);

    SET @rec_id = LAST_INSERT_ID();

    INSERT INTO `tasks_logs_html`(`recordid`, `data`)
    VALUES(@rec_id, p_data);

    UPDATE `tasks_logs_records`
    SET `htmlid` = LAST_INSERT_ID()
    WHERE `id` = @rec_id;

    SET p_result = 1;
    COMMIT;
    END$$

    DELIMITER ;

    -- --------------------------------------------------------

    --
    -- Table structure for table `tasks_logs_html`
    --

    CREATE TABLE IF NOT EXISTS `tasks_logs_html` (
	      `recordid` int(10) unsigned NOT NULL,
	        `data` mediumtext COLLATE utf8_unicode_ci NOT NULL,
		  `id` int(10) unsigned NOT NULL
	    ) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

    --
    -- Dumping data for table `tasks_logs_html`
    --

    INSERT INTO `tasks_logs_html` (`recordid`, `data`, `id`) VALUES
    (6, 'GET /v4/json/ HTTP/1.1\r\nHosts: api-sandbox.direct.yandex.ru\r\nConnection: close\r\n\r\n{"method":"GetVersion"}\r\n', 6),
    (7, 'GET /v4/json/ HTTP/1.1\r\nHosts: api-sandbox.direct.yandex.ru\r\nConnection: close\r\n\r\n{"method":"GetVersion"}\r\n', 7),
    (8, 'GET /v4/json/ HTTP/1.1\r\nHosts: api-sandbox.direct.yandex.ru\r\nConnection: close\r\n\r\n{"method":"GetVersion"}\r\n', 8);

-- --------------------------------------------------------

--
-- Table structure for table `tasks_logs_records`
--

CREATE TABLE IF NOT EXISTS `tasks_logs_records` (
	  `taskid` int(10) unsigned NOT NULL,
	    `posted` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
	      `message` varchar(4096) COLLATE utf8_unicode_ci NOT NULL,
	        `id` int(10) unsigned NOT NULL,
		  `type` tinyint(3) unsigned NOT NULL COMMENT '0 - info, 1 - error, 2 - debug, 3 - warning',
		    `htmlid` int(10) unsigned DEFAULT NULL
	) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `tasks_logs_records`
--

INSERT INTO `tasks_logs_records` (`taskid`, `posted`, `message`, `id`, `type`, `htmlid`) VALUES
(1, 0x323031362d30332d33302032313a33323a3036, 'Created request', 6, 0, 6),
    (1, 0x323031362d30332d33302032313a33323a3139, 'Created request', 7, 0, 7),
    (1, 0x323031362d30332d33302032313a33323a3235, 'Created request', 8, 0, 8),
    (1, 0x323031362d30332d33302032313a34373a3234, 'Test', 10, 1, NULL);

-- --------------------------------------------------------

--
-- Table structure for table `tasks_phrases`
--

CREATE TABLE IF NOT EXISTS `tasks_phrases` (
	  `taskid` int(10) unsigned NOT NULL,
	    `phrase` varchar(1024) COLLATE utf8_unicode_ci NOT NULL,
	      `phraseid` int(10) unsigned NOT NULL
	) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `tasks_phrases`
--

INSERT INTO `tasks_phrases` (`taskid`, `phrase`, `phraseid`) VALUES
(1, 'гипсокартон', 1);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `tasks_logs_html`
--
ALTER TABLE `tasks_logs_html`
  ADD PRIMARY KEY (`id`),
    ADD KEY `taskid` (`recordid`);

--
-- Indexes for table `tasks_logs_records`
--
ALTER TABLE `tasks_logs_records`
  ADD PRIMARY KEY (`id`),
    ADD KEY `taskid` (`taskid`),
      ADD KEY `posted` (`posted`),
        ADD KEY `htmlid` (`htmlid`);

--
-- Indexes for table `tasks_phrases`
--
ALTER TABLE `tasks_phrases`
  ADD PRIMARY KEY (`phraseid`),
    ADD KEY `taskid` (`taskid`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `tasks_logs_html`
--
ALTER TABLE `tasks_logs_html`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=9;
  --
  -- AUTO_INCREMENT for table `tasks_logs_records`
  --
  ALTER TABLE `tasks_logs_records`
    MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=11;
    --
    -- AUTO_INCREMENT for table `tasks_phrases`
    --
    ALTER TABLE `tasks_phrases`
      MODIFY `phraseid` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
      --
      -- Constraints for dumped tables
      --

      --
      -- Constraints for table `tasks_logs_html`
      --
      ALTER TABLE `tasks_logs_html`
        ADD CONSTRAINT `tasks_logs_html_ibfk_1` FOREIGN KEY (`recordid`) REFERENCES `tasks_logs_records` (`id`);

	--
	-- Constraints for table `tasks_logs_records`
	--
	ALTER TABLE `tasks_logs_records`
	  ADD CONSTRAINT `tasks_logs_records_ibfk_1` FOREIGN KEY (`taskid`) REFERENCES `tasks_phrases` (`taskid`);

	  --
	  -- Constraints for table `tasks_phrases`
	  --
	  ALTER TABLE `tasks_phrases`
	    ADD CONSTRAINT `tasks_phrases_ibfk_1` FOREIGN KEY (`taskid`) REFERENCES `ydd_tasks`.`tasks` (`id`);

